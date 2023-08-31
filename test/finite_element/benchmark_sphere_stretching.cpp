#include <math.h>
#include <omp.h>

#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "implicit_system.h"
#include "transient_implicit_system.h"
#include "dirichlet_boundary.h"
#include "fem_functions.h"
#include "dense_matrix.h"
#include "numeric_vector.h"
#include "tensor.h"
#include "xdmf_writer.h"
#include "fem_stabilizations.h"

#include "test_config.h"

static char help[] = "Benchmark with Sphere Stretching experiment\n\nReference: DOI 10.1002/fld.2614";

double function_g(const double t)
{
    const int T = 3;
    return cos(M_PI * t/T);
}

/*
    Here we use CFL condition to calculate the time step
    for the simulation. The CFL condition is given by:
    cfl = u*dt/dx
    where u is the velocity, dt is the time step and dx
    is the characteristic length (h) of the element. The
    characteristic length of the element is calculated
    as the diameter of the sphere that circumscribes
    the element. The velocity is calculated as the
    maximum velocity of the analytical solution.

    So dt for a given time step is calculated with the minimum
    h characteristic and the maximum velocity for all elements.
*/
double calculate_stable_dt(ParallelMesh* pmesh, double cfl, double t)
{
    unsigned int nelem = pmesh->get_n_elements();
    QGauss qrule;
    FEMFunction fem;
    double min_h = PETSC_MAX_REAL;
    double max_velocity = 0.0;

    double velocity = 0.0;
    for(unsigned int iel = 0; iel < nelem; iel++)
    {
        Element elem;
        pmesh->getElement(iel, elem);

        qrule.reset(elem);
        // As TET4 has only one integration point, it is used
        // this integration point to calculate the characteristic
        // length of the element
        fem.ComputeFunction(elem,qrule.get(0));
        double h = elem.calculate_h();

        std::vector<unsigned int> conn = elem.connectivity();
        int connsize = conn.size();
        for(int conn_i = 0; conn_i < connsize; conn_i++)
        {
            Point node = elem.node(conn_i);
            double x = node(0);
            double y = node(1);
            double z = node(2);
            double vel_x_gt = 2*sin(M_PI*x)*sin(M_PI*x)*sin(2*M_PI*y)*sin(2*M_PI*z);
            double vel_y_gt = -sin(2*M_PI*x)*sin(M_PI*y)*sin(M_PI*y)*sin(2*M_PI*z);
            double vel_z_gt = -sin(2*M_PI*x)*sin(2*M_PI*y)*sin(M_PI*z)*sin(M_PI*z);
 
            double gt = function_g(t);
            double vel_x = vel_x_gt*gt;
            double vel_y = vel_y_gt*gt;
            double vel_z = vel_z_gt*gt;

            velocity = sqrt(vel_x*vel_x + vel_y*vel_y + vel_z*vel_z);

            min_h = std::min(min_h, h);
            max_velocity = std::max(max_velocity, velocity);            
        }
    }

    double dt = cfl*min_h/max_velocity;
    return dt;
}

double initial_condition (const double x,
                          const double y,
                          const double z,
                          const double t)
{ 
    
    double dist = sqrt((x - 0.35)*(x - 0.35) + (y - 0.35)*(y - 0.35) + (z - 0.35)*(z - 0.35));
    if(fabs(dist-0.15) < 5e-5)
        return 1.0;
    return 0.0;
}

void init_transport(TransientImplicitSystem* system)
{

    auto mesh   = system->get_mesh();
    auto coords = mesh.getCoord();
    int n_nodes = mesh.get_n_nodes();

    int ndof = system->get_equation_manager().get_n_dofs();
    int dof  = system->get_variable_id("u");

    double* solution = system->get_local_solution_array();
    for(int i=0; i < n_nodes; i++)
    {
        const double x = coords[i*3 + 0];
        const double y = coords[i*3 + 1];
        const double z = coords[i*3 + 2];
        solution[i*ndof+dof]    =  initial_condition(x, y, z, 0.0);
    }
    system->restore_local_solution_array(&solution);

}

double f(Point p, double t)
{
    return 0.0;
}

void assemble_transport(TransientImplicitSystem* system)
{
    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    EquationManager &equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements = pmesh.get_n_elements();

    double *old_solution = system->get_old_solution_array();
    double *solution     = system->get_local_solution_array();

    QGauss qrule;
    FEMFunction fem;

    // loop sobre os elementos da malha por cores
    for (int iel = 0; iel < n_elements; iel++)
    {
        Element elem;
        pmesh.getElement(iel,elem);

        int nnoel = elem.n_nodes();

        std::vector<int>        global_indices;
        std::vector<int>        local_indices;
        DenseMatrix<double>     Ke(nnoel, nnoel);  // matriz de rigidez do elemento
        std::vector<double>     Fe(nnoel);         // vetor de força do elemento
        std::vector<double>   & phi   = fem.get_phi();
        std::vector<Gradient> & dphi  = fem.get_dphi();
        double                & JxW   = fem.get_JxW();
        RealVector            & g     = fem.get_g();
        RealTensor            & G     = fem.get_G();
        RealVector            & dxi   = fem.get_dxi();
        RealVector            & deta  = fem.get_deta();
        RealVector            & dzeta = fem.get_dzeta();
        Point                 & xyz   = fem.get_xyz();
        
        equation_manager.global_indices(dof, elem.connectivity(), global_indices);
        equation_manager.local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento
        qrule.reset(elem);

        double           source_term;
        double k            = 1E-5;
        double sigma        = 0.0;
        double theta        = 0.5;
        double dt           = system->get_deltat();
        double t            = system->get_time();
        double dt_stab      = 1.0;
        double gt = function_g(t);

        // loop sobre os pontos de integração
        for (int q = 0; q < qrule.n_points(); q++)
        {
            // Calcula funções para elemento
            fem.ComputeFunction(elem,qrule.get(q));
            double h_carach = elem.calculate_h();

            RealVector velocity;
            double u_old  = 0.0;
            Gradient grad_u_old;
        
            double    u = 0.0;
            Gradient  grad_u;

            velocity(0) = 0.0; 
            velocity(1) = 0.0;
            velocity(2) = 0.0;
           
            for (int i = 0; i < local_indices.size(); i++)
            {
                double x      = elem.node(i)(0);
                double y      = elem.node(i)(1);
                double z      = elem.node(i)(2);
                double vel_x = 2*sin(M_PI*x)*sin(M_PI*x)*sin(2*M_PI*y)*sin(2*M_PI*z)*gt;
                double vel_y =  -sin(2*M_PI*x)*sin(M_PI*y)*sin(M_PI*y)*sin(2*M_PI*z)*gt;
                double vel_z =  -sin(2*M_PI*x)*sin(2*M_PI*y)*sin(M_PI*z)*sin(M_PI*z)*gt;

                velocity(0) += vel_x * phi[i];
                velocity(1) += vel_y * phi[i];
                velocity(2) += vel_z * phi[i];
                
                u_old         +=  old_solution[local_indices[i]]*phi[i];
                grad_u_old(0) +=  old_solution[local_indices[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices[i]]*dphi[i](1);
                grad_u_old(2) +=  old_solution[local_indices[i]]*dphi[i](2);

                u             +=  solution[local_indices[i]]*phi[i];
                grad_u(0)     +=  solution[local_indices[i]]*dphi[i](0);
                grad_u(1)     +=  solution[local_indices[i]]*dphi[i](1);
                grad_u(2)     +=  solution[local_indices[i]]*dphi[i](2);
            }

            source_term = f(xyz,t);

            // SUPG stabilization parameters
            const double tau = TAUStab(velocity, G, k, dt_stab, dt);

            // CAU stabilization parameters
            const double ctau = CAUStab(u, u_old, grad_u, source_term, velocity, sigma, k, dt, h_carach);

            const double adt1 = (1.0-theta)*dt;
            const double adt  = theta*dt;
            // calculando a matriz de rigidez e o vetor de forca local
            for (int i = 0; i < local_indices.size(); i++)
            {
                // Galerkin 
                Fe[i]   +=  JxW*(phi[i]*u_old - adt1*phi[i]*(velocity * grad_u_old) 
                                              - adt1*k*(dphi[i] * grad_u_old)  
                                              - adt1*sigma*phi[i]*u_old
                                );

                //  SUPG contribution
                Fe[i] += JxW * tau * (
                                         u_old * (velocity * dphi[i])+
                                         -adt1 * (grad_u_old * velocity)*(velocity * dphi[i])
                                         -adt1 * (sigma*u_old)*(velocity * dphi[i])
                                     );

                for (int j = 0; j < local_indices.size(); j++)
                {
                    // Galerkin Formulation
                    Ke(i, j) += JxW * ( phi[i]*phi[j]                              // termo de massa
                                           + adt*(phi[i] * (velocity * dphi[j]))   // Na (vel. grad Nb) - Termo convectivo
                                           + adt*k*(dphi[i] * dphi[j])             // Grad Na Grad Nb - Termo difusivo
                                           + adt*sigma*phi[i]*phi[j]              // \sigma* Na  Nb  - Termo reação      
                                       );

                    // SUPG contribution
                    Ke(i, j) += JxW * tau * (
                                    phi[j]*(velocity * dphi[i]) +
                                     adt * (velocity * dphi[j])*(velocity * dphi[i]) +
                                     adt * (sigma * phi[j] )*(velocity * dphi[i])
                            );

                    // CAU contribution
                    Ke(i,j) += JxW * ctau * adt * (dphi[i] * dphi[j]);

                }
            }
        }
        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        system->add_matrix_entry(global_indices, global_indices, Ke.get_data());
        system->add_rhs_entry(global_indices, Fe.data());
    }

    system->restore_old_solution_array(&old_solution);

}


int sphere_stretching(int argc, char *argv[])
{
    PetscErrorCode ierr;
    MeshPartition *parts = new MeshPartition();

    Mesh *mesh;          // serial mesh
    ParallelMesh *pmesh; // parallel mesh
    int processor_id, n_processors;

    processor_id = MeshTools::processor_id();
    n_processors = MeshTools::n_processors();

    if (processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha.
        string test_mesh_dir = TEST_MESH_DIR;
        test_mesh_dir.append("benchmark_sphere_stretching/sphere.msh");
        mesh = new Mesh(test_mesh_dir);

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);


    // Cria o sistema de equações implicito
    TransientImplicitSystem *system = new TransientImplicitSystem(*pmesh, "benchmark_sphere_stretching");
    system->add_variable("u");
    DirichletBoundary bc(1,0,"0.0","x,y,z");
    system->add_dirichlet_boundary(bc);
    system->attach_init_function(init_transport);
    system->attach_assemble(assemble_transport);

    system->init();

    double tf = 3.0;
    double dt;
    double local_dt = calculate_stable_dt(pmesh, 1, 0.0);
    MPI_Allreduce(&local_dt, &dt, 1, MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD);

    if (processor_id == 0)
        printf("Respecting CFL condition, dt = %f\n", dt);

    system->set_final_time(tf);
    system->set_deltat(dt);
    unsigned int write_interval = 25;


    char filename[100];
    sprintf(filename,"sphere_stretching");
    system->write_result(filename);

    // Time integratiom
    while(system->get_time() < system->get_final_time())
    {
        system->solve_time_step();

        if(system->get_time_step()%write_interval == 0 )
            system->write_result(filename);
        
        // Dynamic time step
        local_dt = calculate_stable_dt(pmesh, 1, system->get_time());
        MPI_Allreduce(&local_dt, &dt, 1, MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD);
        if(dt < 0.0001)
            dt = 0.0001;
        if(dt > 0.1)
            dt = 0.1;

        system->set_deltat(dt);
    }

    system->write_result(filename);

    delete system;

    if (MeshTools::processor_id() == 0)
        delete mesh;
    delete pmesh;
    delete parts;

    return 0;
}

int main(int argc, char *argv[])
{
    MeshTools::Init(argc,argv);
    sphere_stretching(argc, argv);
    MeshTools::Finalize();
}
