#include <math.h>

#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
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

static char help[] = "Benchmark with Transient Rotation Pulse experiment\n\n";

double exact_solution (const double x,
                       const double y,
                       const double t)
{ 
    double r = (x - 5.0)*(x - 5.0) + (y - 7.5)*(y - 7.5);

    return exp(-0.5*r);
}

void init_transport(TransientImplicitSystem* system)
{
    auto& mesh    = system->get_mesh();
    auto& coords  = mesh->get_coordinate_vector();
    int n_nodes   = mesh->get_n_nodes();

    int ndof = system->get_equation_manager()->get_n_dofs();
    int dof  = system->get_variable_id("u");

    double* solution = system->get_local_solution_array();
    for(int i=0; i < n_nodes; i++)
    {
        const double x = coords[i*3 + 0];
        const double y = coords[i*3 + 1];
        solution[i*ndof+dof]    =  exact_solution(x, y, 0.0);
    }
    system->restore_local_solution_array(&solution);

}

void assemble_transport(TransientImplicitSystem* system)
{

    auto& mesh = system->get_mesh();
    int   ndim = mesh->get_mesh_dimension();

    // Gerencia as numerações das equações do sistema
    auto& equation_manager = system->get_equation_manager();
    int dof                = 0;

    int n_elements         = mesh->get_n_elements();

    double *old_solution   = system->get_old_solution_array();

    auto qrule = QGauss::New();
    auto fem   = FEMFunction::New();

    // loop sobre os elementos da malha por cores
    for (int iel = 0; iel < n_elements; iel++)
    {
        Element elem;
        mesh->get_element(iel,elem);

        int nnoel = elem.n_nodes();

        std::vector<int>        global_indices;
        std::vector<int>        local_indices;
        DenseMatrix<double>     Ke(nnoel, nnoel);  // matriz de rigidez do elemento
        std::vector<double>     Fe(nnoel);         // vetor de força do elemento
        std::vector<double>   & phi = fem->get_phi();
        std::vector<Gradient> & dphi= fem->get_dphi();
        Point                 & p   =  fem->get_xyz();
        double                & JxW = fem->get_JxW();
        RealVector            & g   = fem->get_g();
        RealTensor            & G   = fem->get_G();
        
        equation_manager->global_indices(dof, elem.connectivity(), global_indices);
        equation_manager->local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento
        qrule->reset(elem);

        
        double k            = 1.0E-08;
        double sigma        = 0.0;
        double theta        = 0.5;
        double dt           = system->get_deltat();
        double dt_stab       = 0.1;


        // loop sobre os pontos de integração
        for (int q = 0; q < qrule->n_points(); q++)
        {
            // Calcula funções para elemento
            fem->ComputeFunction(elem,qrule->get(q));


            double u_old  = 0.0;
            Gradient grad_u_old;
            Gradient velocity;

            for (int i = 0; i < local_indices.size(); i++)
            {
                velocity(0)   += -(elem.node(i)(1) - 5.0)*phi[i]; // V_x = -y - 5 
                velocity(1)   +=  (elem.node(i)(0) - 5.0)*phi[i]; // V_y =  x - 5
                u_old         +=  old_solution[local_indices[i]]*phi[i];
                grad_u_old(0) +=  old_solution[local_indices[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices[i]]*dphi[i](1);
            }

            // SUPG stabilization parameters
            const double tau = TAUStab(velocity, G, k, dt_stab, dt);


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

int rotation_pulse(int argc, char *argv[])
{

    //string test_mesh_dir = std::string(MESHTOOLS_SOURCE_DIR)+"/test/finite_element/msh/";
    string test_mesh_dir = std::string(MESHTOOLS_SOURCE_DIR)+"/test/finite_element/msh/";
    //test_mesh_dir.append("benchmark_rotation_pulse/benchmark_rotpulse_quad4_32.msh");
    // test_mesh_dir.append("benchmark_rotation_pulse/benchmark_rotpulse_quad_64.msh");
    test_mesh_dir.append("benchmark_rotation_pulse/benchmark_rotpulse_tri3_128.msh");

    auto mesh   = MeshTools::read(test_mesh_dir);

    // Cria o sistema de equações implicito
    auto system = TransientImplicitSystem::New(mesh, "benchmark_rotation_pulse");

    
    system->add_variable("u");
    DirichletBoundary  bc(1,0,"0.0","x,y,z");
    system->add_dirichlet_boundary(bc);
    system->attach_init_function(init_transport);
    system->attach_assemble(assemble_transport);

    system->init();

    // const double tfinal = 6.28;//2.0*M_PI;
    // const double nsteps = 800;
    // const double dt     = tfinal/nsteps;
    system->set_final_time(6.28);
    system->set_deltat(0.005);
    system->set_nonlinear_max_iter(1);
    unsigned int write_interval = 10;


    char filename[100];
    sprintf(filename,"rotation_pulse");
    system->write_result(filename);

    // Time integratiom
    double time = 0.0;
    while(system->get_time() < system->get_final_time())
    {
        system->solve_time_step();

        if(system->get_time_step()%write_interval == 0 )
        {
            system->write_result(filename);
        }
    }
    system->write_result(filename);

    return 0;
}

int main(int argc, char *argv[])
{
    MeshTools::Init(argc,argv);
    rotation_pulse(argc, argv);
    MeshTools::Finalize();
}

