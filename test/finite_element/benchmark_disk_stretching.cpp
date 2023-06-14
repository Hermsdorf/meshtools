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
#include <math.h>

static char help[] = "Benchmark with Disk Stretching experiment\n\n";

double function_g(const double t)
{
    const int T = 8;
    return cos(2.0 * M_PI * t/T);
}

double initial_condition (const double x,
                          const double y,
                          const double t)
{ 
    
    double dist = (x - 0.5)*(x - 0.5) + (y - 0.75)*(y - 0.75);
    if(dist-0.01 < 0.0001) 
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
        solution[i*ndof+dof]    =  initial_condition(x, y, 0.0);
    }
    system->restore_local_solution_array(&solution);

}


double cau_stab(double u, double u_old, Gradient grad_u, double f,
                RealVector velocity, double sigma, double K, double dt,
                RealVector dxi, RealVector deta, RealVector dzeta)
{
    double res = velocity*grad_u - sigma*u - f;

    double velocity_norm = velocity.norm();
    double dphi_norm     = std::max(1.0E-10, grad_u.norm());
    
    RealVector v;

    
    if(dphi_norm == 0.0)
        v = velocity;
    else
        v = velocity - (grad_u*res)/(dphi_norm*dphi_norm);

    RealVector be;
    for(int i = 0 ; i < 3 ; i++)
        be(i) = velocity(i)*(dxi(i) + deta(i) + dzeta(i));
    
    double be_norm = be.norm();
    double he = 2.0*velocity_norm/be_norm;
    double Pe = he*velocity_norm/(2*K);
    double tau_e = std::max(0.0, 1.0 - (1.0/Pe));


    RealVector be_c;
    for(int i = 0 ; i < 3 ; i++){
        double velocity_diff = velocity(i) - v(i);
        be_c(i) = velocity_diff*(dxi(i) + deta(i) + dzeta(i));
    }

    double be_c_norm = be_c.norm();
    RealVector diff_v = velocity - v;
    double     diff_v_norm = diff_v.norm();

    be_c_norm = std::max(1.0E-10,be_c_norm);
    double he_c = 2*diff_v_norm/be_c_norm;
    double Pe_c = (he_c*diff_v_norm)/(2.0*std::abs(K));
    double tau_c = std::max(0.0, 1.0 - (1.0/Pe_c));

    double res_vel_dphi = std::abs(res)/(velocity_norm*dphi_norm);
    double tauc_hc_tau_h = (tau_c*he_c)/(tau_e*he);
    
    if(res_vel_dphi >= tauc_hc_tau_h)
        return 0.0;
    else
        return (tau_e*he*0.5)*(tauc_hc_tau_h - res_vel_dphi)*(std::abs(res)/dphi_norm);

/*
    double  res_mass = (u - u_old) / dt;
    double  res_adv  = (velocity * grad_u);
    double residuo   = res_mass + res_adv - sigma*u - f;
    double gcnorm = grad_u.norm();
    gcnorm = std::max(1.0E-10, gcnorm);
    double ogcnorm = 1.0 / gcnorm;
    double aux3 = res_adv / (ogcnorm * ogcnorm);
    RealVector b(grad_u(0) * aux3, grad_u(1) * aux3);
    double bnorm = b.norm();
    bnorm = std::max(bnorm, 1.0E-10);
    double bdb = k * bnorm*bnorm;
    bdb = std::max(bdb, 1.0E-10);
    double Pe_p = h_caract * (bnorm * bnorm * bnorm) / bdb;
    Real alpha_c = std::min(0.25 * Pe_p, 0.70);
    Real delta_sco = 0.5 * h_caract * alpha_c * residuo * ogcnorm * fopc;

*/
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


            RealVector velocity;
            double u_old  = 0.0;
            Gradient grad_u_old;

        
            double    u = 0.0;
            Gradient  grad_u;

            velocity(0)   = 0.0;  //function_g(t)*sin(2 * M_PI * xyz(1)) * sin(M_PI * xyz(0)) * sin(M_PI * xyz(0)); 
            velocity(1)   = 0.0; //-function_g(t)*sin(2 * M_PI * xyz(0)) * sin(M_PI * xyz(1)) * sin(M_PI * xyz(1));
           
            for (int i = 0; i < local_indices.size(); i++)
            {
                double x      = elem.node(i)(0);
                double y      = elem.node(i)(1);
                double velx_x = gt*sin(2 * M_PI * y) * sin(M_PI * x) * sin(M_PI *x);
                double velx_y = -gt*sin(2 * M_PI * x) * sin(M_PI * y) * sin(M_PI *y);

                velocity(0) += velx_x * phi[i];
                velocity(1) += velx_y * phi[i];
                
                u_old         +=  old_solution[local_indices[i]]*phi[i];
                grad_u_old(0) +=  old_solution[local_indices[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices[i]]*dphi[i](1);

                u             +=  solution[local_indices[i]]*phi[i];
                grad_u(0)     +=  solution[local_indices[i]]*dphi[i](0);
                grad_u(1)     +=  solution[local_indices[i]]*dphi[i](1);
    
            }

            source_term = 0.0; f(xyz,0.0);


            // SUPG stabilization parameters
            const double tmp = (velocity) * (G.mult(velocity)) + (k * k) * (G.contract(G)) + dt_stab*4.0/(dt*dt);
            const double tau = 1.0/sqrt(tmp);

            // CAU stabilization parameters
            const double ctau = cau_stab(u, u_old, grad_u, source_term, velocity, sigma, k, dt, dxi, deta, dzeta);

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
                    // Ke(i,j) += JxW * ctau * adt * (dphi[i] * dphi[j] );

                }
            }
        }
        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        system->add_matrix_entry(global_indices, global_indices, Ke.get_data());
        system->add_rhs_entry(global_indices, Fe.data());
    }

    system->restore_old_solution_array(&old_solution);

}


int disk_stretching(int argc, char *argv[])
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
        mesh = new Mesh(argv[1]);

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);


    // Cria o sistema de equações implicito
    TransientImplicitSystem *system = new TransientImplicitSystem(*pmesh, "benchmark_disk_stretching");
    system->add_variable("u");
    DirichletBoundary  bc(1,0,"0.0","x,y,z");
    system->add_dirichlet_boundary(bc);
    system->attach_init_function(init_transport);
    system->attach_assemble(assemble_transport);

    system->init();
    system->set_final_time(1.0);
    system->set_deltat(0.0025);
    unsigned int write_interval = 20;


    char filename[100];
    sprintf(filename,"solution");
    system->write_result(filename);

    // Time integratiom
    while(system->get_time() < system->get_final_time())
    {
        system->solve_time_step();

        if(system->get_time_step()%write_interval == 0 )
        {
            sprintf(filename,"solution");
            system->write_result(filename);
        }
    }

    sprintf(filename,"solution");
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
    disk_stretching(argc, argv);
    MeshTools::Finalize();
}
