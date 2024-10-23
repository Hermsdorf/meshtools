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


static char help[] = "Benchmark with Disk Stretching experiment\n\n";

#define PROFILING

#define T 8.0
double function_g(const double t)
{
    return cos(M_PI * t/T);
}


double velocity_x(const double x,
                  const double y,
                  const double t)
{
    return function_g(t)*sin(2 * M_PI * y) * sin(M_PI * x) * sin(M_PI *x);
}

double velocity_y(const double x,
                  const double y,
                  const double t)
{
    return -function_g(t)*sin(2 * M_PI * x) * sin(M_PI * y) * sin(M_PI *y);
}

double initial_condition (const double x,
                          const double y,
                          const double t)
{ 
    
    double dist = (x - 0.5)*(x - 0.5) + (y - 0.75)*(y - 0.75);
    if(dist-0.015 < 1.0E-3) 
        return 1.0;
    return 0.0;
}

void init_transport(TransientImplicitSystem* system)
{

    auto & mesh   = system->get_mesh();
    auto & coords = mesh->get_coordinate_vector();
    int n_nodes   = mesh->get_n_nodes();

    int ndof = system->get_equation_manager()->get_n_dofs();
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

double f(Point p, double t)
{
    return 0.0;
}

void assemble_transport(TransientImplicitSystem* system)
{

    auto &mesh  =  system->get_mesh();
    int  ndim  =  mesh->get_mesh_dimension();

    // Gerencia as numerações das equações do sistema
    auto &equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements      = mesh->get_n_elements();

    double *old_solution = system->get_old_solution_array();
    double *solution     = system->get_local_solution_array();

    auto qrule = QGauss::New();
    auto fem   = FEMFunction::New();
    auto elem  = Element::New();
    FemStabilization fem_stab(fem);
    fem_stab.set_time_step_factor(0.1);
    fem_stab.set_yzb_phi_reference(1.0);
    fem_stab.set_yzb_beta(1.0);

    // loop sobre os elementos da malha por cores
    for (int iel = 0; iel < n_elements; iel++)
    {
        
        mesh->get_element(iel,*elem);

        int nnoel = elem->n_nodes();

        std::vector<int>        global_indices;
        std::vector<int>        local_indices;
        DenseMatrix<double>     Ke(nnoel, nnoel);  // matriz de rigidez do elemento
        std::vector<double>     Fe(nnoel);         // vetor de força do elemento
        std::vector<double>   & phi   = fem->get_phi();
        std::vector<Gradient> & dphi  = fem->get_dphi();
        double                & JxW   = fem->get_JxW();
        RealVector            & g     = fem->get_g();
        RealTensor            & G     = fem->get_G();
        RealVector            & dxi   = fem->get_dxi();
        RealVector            & deta  = fem->get_deta();
        RealVector            & dzeta = fem->get_dzeta();
        Point                 & xyz   = fem->get_xyz();
        
        equation_manager->global_indices(dof, elem->connectivity(), global_indices);
        equation_manager->local_indices(dof,  elem->connectivity(), local_indices);

        // Obtem pontos de integração para elemento
        qrule->reset(*elem);

        double           source_term;
        double k            = 1E-5;
        double sigma        = 0.0;
        double theta        = 0.5;
        double dt           = system->get_deltat();
        double t            = system->get_time();
        double dt_stab      = 1.0;
        double gt = function_g(t);

        // loop sobre os pontos de integração
        for (int q = 0; q < qrule->n_points(); q++)
        {
            // Calcula funções para elemento
            fem->ComputeFunction(*elem,qrule->get(q));
            // double h_carach = elem->calculate_h();

            // assert(h_carach >= 0.0);


            RealVector velocity;
            double u_old  = 0.0;
            Gradient grad_u_old;

        
            double    u = 0.0;
            Gradient  grad_u;

            velocity(0)   = velocity_x(xyz(0), xyz(1),t);  //function_g(t)*sin(2 * M_PI * xyz(1)) * sin(M_PI * xyz(0)) * sin(M_PI * xyz(0)); 
            velocity(1)   = velocity_y(xyz(0), xyz(1),t);  //-function_g(t)*sin(2 * M_PI * xyz(0)) * sin(M_PI * xyz(1)) * sin(M_PI * xyz(1));
           
            for (int i = 0; i < local_indices.size(); i++)
            {
                u_old         +=  old_solution[local_indices[i]]*phi[i];
                grad_u_old(0) +=  old_solution[local_indices[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices[i]]*dphi[i](1);

                u             +=  solution[local_indices[i]]*phi[i];
                grad_u(0)     +=  solution[local_indices[i]]*dphi[i](0);
                grad_u(1)     +=  solution[local_indices[i]]*dphi[i](1);
    
            }

            source_term = f(xyz,t);



            // SUPG stabilization parameters
            //const double tau = TAUStab(velocity, G, k, dt_stab, dt);
            const double tau = fem_stab.supg_stabilization(velocity, k, dt);

            // CAU stabilization parameters
            // const double ctau = CAUStab(u, u_old, grad_u, source_term, velocity, sigma, k, dt, h_carach);
            
            // YZB Stabilization 
            // Reference:
            // Bazilevs, Y., Calo, V.M., Tezduyar, T.E. and Hughes, T.J.R. (2007), 
            // YZβ discontinuity capturing for advection-dominated processes with 
            // application to arterial drug delivery. Int. J. Numer. Meth. Fluids, 54: 593-608. 
            // https://doi.org/10.1002/fld.1484
            // double beta    = 1.0;
            // double phi_ref = 1.0;
            // double fopc    = 0.0;
            // double inv_phi_ref = 1.0 / phi_ref;

            double dudt        = (u - u_old) / dt;
            double adv         = (velocity * grad_u);
            double reac        = sigma*u;

            double residuo = dudt + adv - reac - source_term;
            // double A       = abs(residuo) / phi_ref;
            // double tmp1    = pow(grad_u(0)*inv_phi_ref,2.0) + pow(grad_u(1)*inv_phi_ref,2.0);
            // double B       = tmp1 > 0.0 ? pow(tmp1, (beta / 2.0 - 1.0)): 0.0;
            // double C = pow(h_carach / 2.0, beta);
            // const double ctau = A*B*C*fopc;

            // YZB stabilization parameters
            const double ctau = fem_stab.yzb_stabilization(residuo, grad_u);

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

                    // YZB contribution
                    Ke(i,j) += JxW * ctau * adt * (dphi[i] * dphi[j] );

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
 
    string test_mesh_dir = MESHTOOLS_SOURCE_DIR;
    test_mesh_dir.append("/test/finite_element/msh/benchmark_disc_stretching/disk_quad4.msh");

    auto mesh   = MeshTools::read(test_mesh_dir);

    auto system = TransientImplicitSystem::New(mesh,"benchmark_disk_stretching");

    system->add_variable("u");
    DirichletBoundary  bc(1,0,"0.0","x,y,z");
    InitialCondition   ic(3,0,"1.0","x,y,z");
    system->add_initial_condition(ic);
    system->add_dirichlet_boundary(bc);
    //system->attach_init_function(init_transport);
    system->attach_assemble(assemble_transport);

    system->init();
    system->set_final_time(T);
    system->set_deltat(0.0025);
    system->set_nonlinear_max_iter(7);
    system->set_nonlinear_tolerance(1.0E-4);
    system->set_linear_tolerance(1.0E-3);


    unsigned int write_interval = 20;


    char filename[100];
    snprintf(filename,100,"disk_stretching");
    system->write_result(filename);

    // Time integratiom
#ifdef PROFILING
   PetscLogStage  stagenum0;
   PetscLogStageRegister("Time Integration", &stagenum0); 
   PetscLogStagePush(stagenum0);   
#endif
    while(system->get_time() < system->get_final_time())
    {
        system->solve_time_step();

        if(system->get_time_step()%write_interval == 0 )
        {
            system->write_result(filename);
        }
    }
#ifdef PROFILING
   PetscLogStagePop();   
#endif

    system->write_result(filename);


    return 0;
}

int main(int argc, char *argv[])
{
    MeshTools::Init(argc,argv);
    disk_stretching(argc, argv);
    MeshTools::Finalize();
}
