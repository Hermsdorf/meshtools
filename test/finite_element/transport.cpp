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

static char help[] = "Convecção-difusão-reaçao transiente\n\n";

double exact_solution (const double x,
                       const double y,
                       const double t)
{
  const double xo = 0.2;
  const double yo = 0.2;
  const double u  = 1;
  const double v  = 1;

  const double num =
    pow(x - u*t - xo, 2.) +
    pow(y - v*t - yo, 2.);

  const double den =
    0.01*(4.*t + 1.);

  return exp(-num/den)/(4.*t + 1.);
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
        solution[i*ndof+dof]    =  exact_solution(x,y,0.0);
    }
    system->restore_local_solution_array(&solution);

}

void assemble_transport(TransientImplicitSystem* system)
{

    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    auto equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements = pmesh.get_n_elements();

    double *old_solution = system->get_old_solution_array();
    double *solution     = system->get_local_solution_array();


    QGauss qrule;
    FEMFunction fem;
    std::vector<double>   & phi = fem.get_phi();
    std::vector<Gradient> & dphi= fem.get_dphi();
    double                & JxW = fem.get_JxW();
    RealVector            & g   = fem.get_g();
    RealTensor            & G   = fem.get_G();

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

        Point qpoint; // coordenadas do ponto de integracao
        
        equation_manager.global_indices(dof, elem.connectivity(), global_indices);
        equation_manager.local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento elem
        qrule.reset(elem);

        Gradient velocity;
        velocity(0)         = 0.8;
        velocity(1)         = 0.8;
        double k            = 1.0E-4;
        double sigma        = 0.0;
        double theta        = 0.5;
        double dt           = system->get_deltat();
        double dt_stab      = 0.1;
        double source_term  = 0.0;
        double h_carach     = elem.calculate_h(JxW);


        // loop sobre os pontos de integração
        for (int q = 0; q < qrule.n_points(); q++)
        {
            // Calcula funções para elemento
            fem.ComputeFunction(elem,qrule.get(q));
        
            // SUPG stabilization parameters
            const double tmp = (velocity) * (G.mult(velocity)) + (k * k) * (G.contract(G)) + dt_stab*4.0/(dt*dt);
            const double tau = 1.0/sqrt(tmp);

            double u_old  = 0.0;
            Gradient grad_u_old;

            double    u = 0.0;
            Gradient  grad_u;

            for (int i = 0; i < local_indices.size(); i++)
            {
                u_old         +=  old_solution[local_indices[i]]*phi[i];
                grad_u_old(0) +=  old_solution[local_indices[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices[i]]*dphi[i](1);
                

                u             +=  solution[local_indices[i]]*phi[i];
                grad_u(0)     +=  solution[local_indices[i]]*dphi[i](0);
                grad_u(1)     +=  solution[local_indices[i]]*dphi[i](1);

                if(ndim == 3){
                    grad_u_old(2)  += old_solution[local_indices[i]]*dphi[i](2);
                    grad_u(2)      += solution[local_indices[i]]*dphi[i](2);
                }
            }

            const double adt1 = (1.0-theta)*dt;
            const double adt  = theta*dt;
            // CAU stabilization parameters
            const double ctau = fem.CAUStab(u, u_old, grad_u, source_term, velocity, sigma, k, dt, h_carach)*0.2;

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
                    // Ke(i,j) += JxW * ctau * adt * (dphi[i] * dphi[j]);

                }
            }
        }
        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        system->add_matrix_entry(global_indices, global_indices, Ke.get_data());
        system->add_rhs_entry(global_indices, Fe.data());
    }

    system->restore_old_solution_array(&old_solution);
}


int transport(int argc, char *argv[])
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
        mesh = new Mesh("/home/gfarache/git/meshtools/test/finite_element/msh/transport/test1.msh");

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);


    // Cria o sistema de equações implicito
    TransientImplicitSystem *system = new TransientImplicitSystem(*pmesh, "transport");
    system->add_variable("u");
    DirichletBoundary  bc(1,0,"0.0","x,y,z");
    system->add_dirichlet_boundary(bc);
    system->attach_init_function(init_transport);
    system->attach_assemble(assemble_transport);

    system->init();
    system->set_final_time(1.0);
    system->set_deltat(0.05);
    unsigned int write_interval = 2;


    char filename[100];
    sprintf(filename,"solution");
    system->write_result(filename);

    // XDMFWriter xdmf("transport");
    // xdmf.set_dir_path("output");
    // xdmf.write(system,system->get_time());

    // Time integratiom
    while(system->get_time() < system->get_final_time())
    {
        system->solve_time_step();

        if(system->get_time_step()%write_interval == 0 )
        {
            sprintf(filename,"solution");
            system->write_result(filename);
            //xdmf.write(system,system->get_time());
        }
    }

    sprintf(filename,"solution");
    system->write_result(filename);
    //xdmf.write(system,system->get_time());

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
    transport(argc, argv);
    MeshTools::Finalize();
}
