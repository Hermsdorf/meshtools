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

void assemble_transport(TransientImplicitSystem* system)
{

    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    EquationManager &equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements = pmesh.get_n_elements();

    double *old_solution = system->get_old_solution_array();

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
        std::vector<double>   & phi = fem.get_phi();
        std::vector<Gradient> & dphi= fem.get_dphi();
        double                & JxW = fem.get_JxW();
        RealVector            & g   = fem.get_g();
        RealTensor            & G   = fem.get_G();
        Point                 & xyz = fem.get_xyz();
        
        equation_manager.global_indices(dof, elem.connectivity(), global_indices);
        equation_manager.local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento
        qrule.reset(elem);


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
            }

            // SUPG stabilization parameters
            const double tmp = (velocity) * (G.mult(velocity)) + (k * k) * (G.contract(G)) + dt_stab*4.0/(dt*dt);
            const double tau = 1.0/sqrt(tmp);

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
    system->set_final_time(8.0);
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
