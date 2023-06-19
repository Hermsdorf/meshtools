
#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "nonlinear_implicit_system.h"
#include "dirichlet_boundary.h"
#include "fem_functions.h"
#include "dense_matrix.h"
#include "numeric_vector.h"
#include "tensor.h"

static char help[] = "Convecção-difusão-reaçao\n\n";

void assemble_convection_diffusion_reaction(NonLinearImplicitSystem* system)
{
    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    auto equation_manager = system->get_equation_manager();
    int dof  = 0;

    double *solution     = system->get_local_solution_array();

    int n_elements = pmesh.get_n_elements();

    QGauss qrule;
    FEMFunction fem;
    std::vector<double>   & phi   = fem.get_phi();
    std::vector<Gradient> & dphi  = fem.get_dphi();
    double                & JxW   = fem.get_JxW();
    RealTensor            & G     = fem.get_G();

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

        equation_manager.global_indices(dof, elem.connectivity(), global_indices);
        equation_manager.local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento elem
        qrule.reset(elem);

        Gradient velocity;
        velocity(0)        = sqrt(3.0) / 2.0;
        velocity(1)        = 1.0 / 2.0 ;
        double kd          = 0.2E-3;
        double sigma       = 0.0;
        double source_term = 0.0;

        // loop sobre os pontos de integração
        for (int q = 0; q < qrule.n_points() ; q++)
        {
            // calculando a função de forma e suas derivadas para o ponto de integração q
            fem.ComputeFunction(elem, qrule.get(q));
            double h_carach    = elem.calculate_h(JxW);

            // SUPG stabilization parameters
            double tau = (velocity) * (G.mult(velocity)) + (kd * kd) * (G.contract(G)); // + dt_stab*4.0/(dt*dt);

            double    u = 0.0;
            Gradient  grad_u;

            for (int i = 0; i < local_indices.size(); i++)
            {
                u             +=  solution[local_indices[i]]*phi[i];
                grad_u(0)     +=  solution[local_indices[i]]*dphi[i](0);
                grad_u(1)     +=  solution[local_indices[i]]*dphi[i](1);

                if(ndim == 3){
                    grad_u(2)      += solution[local_indices[i]]*dphi[i](2);
                }
            }

            // CAU stabilization parameters
            const double ctau = fem.CAUStab(u, u, grad_u, source_term, velocity, sigma, kd, 1, h_carach);

            // calculando a matriz de rigidez e o vetor de forca local
            for (int i = 0; i < nnoel; i++)
            {
                for (int j = 0; j < nnoel; j++)
                {
                    // Galerkin Formulation
                    Ke(i, j) += JxW * (phi[i] * (velocity * dphi[j]) + // Na (a. grad Nb) - Termo convectivo
                                       kd * (dphi[i] * dphi[j])      + // GradNa Grad Nb - Termo difusivo
                                       sigma*phi[i]*phi[j]             // \sigma* w  u  -  Termo reação
                                       );

                    // SUPG Formulation
                    Ke(i, j) += JxW * tau * (velocity * dphi[i]) * (velocity * dphi[j] +      // Termo SUPG convecção
                                              sigma*phi[j]);                                  // Termo SUPG reação

                    // CAU contribution
                    Ke(i,j) += JxW * ctau * (dphi[i] * dphi[j]);
                }
            }
        }
        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        system->add_matrix_entry(global_indices, global_indices, Ke.get_data());
        system->add_rhs_entry(global_indices, Fe.data());
    }

}

int run_convection_diffusion_reaction(int argc, char *argv[])
{
    PetscErrorCode ierr;
    MeshPartition *parts = new MeshPartition();

    // if(argc < 2)
    // {
    //     return 0;
    // }

    Mesh *mesh;          // serial mesh
    ParallelMesh *pmesh; // parallel mesh
    int processor_id, n_processors;

    processor_id = MeshTools::processor_id();
    n_processors = MeshTools::n_processors();

    if (processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha.
    
        mesh = new Mesh("/home/gfarache/git/meshtools/test/finite_element/msh/convection_difussion_2d/conv2dtri3.msh");

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);


    // Cria o sistema de equações implicito
    NonLinearImplicitSystem *nonlinear_implicit_system = new NonLinearImplicitSystem(*pmesh, "convection-diffusion");

    // Adiciona uma variável ao sistema
    int dof = nonlinear_implicit_system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g(x, y) = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc1(1, dof, "0.0", "x,y");
    // Aplica a função g(x, y) = 1 para a variável u no contorno identificado com 2.
    DirichletBoundary bc2(2, dof, "1.0", "x,y");
    nonlinear_implicit_system->add_dirichlet_boundary(bc1);
    nonlinear_implicit_system->add_dirichlet_boundary(bc2);
    nonlinear_implicit_system->attach_assemble(assemble_convection_diffusion_reaction);

    // Inicializar o sistema 
    nonlinear_implicit_system->init();

    // Resolve o sistema de equações
    nonlinear_implicit_system->solve();

    nonlinear_implicit_system->write_result("solution");

    delete nonlinear_implicit_system;

    if (MeshTools::processor_id() == 0)
        delete mesh;
    delete pmesh;
    delete parts;

    
    return 0;
}

int main(int argc, char *argv[])
{
    MeshTools::Init(argc, argv);
    run_convection_diffusion_reaction(argc, argv);
    MeshTools::Finalize();
}
