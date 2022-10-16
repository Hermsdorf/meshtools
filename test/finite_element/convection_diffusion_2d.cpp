
#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "implicit_system.h"
#include "dirichlet_boundary.h"
#include "fem_functions.h"
#include "dense_matrix.h"
#include "numeric_vector.h"
#include "tensor.h"

static char help[] = "Convecção-difusão-reaçao\n\n";

void assemble_convection_diffusion_reaction(ImplicitSystem* system)
{
    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    auto equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements = pmesh.get_n_elements();
    bool flag = true;

    // loop sobre os elementos da malha por cores
    for (int iel = 0; iel < n_elements; iel++)
    {
        std::vector<Point> coords_iel;
        std::vector<unsigned int> conn_iel;

        pmesh.get_element_connectivity(iel, conn_iel);
        pmesh.get_element_coordinates(iel, coords_iel);
        int nnoel = conn_iel.size();

        std::vector<int>        global_indices;
        std::vector<Point>      qp;                // coordenadas do ponto de integração
        std::vector<double>     qw;                // peso do ponto de integração
        DenseMatrix<double>     Ke(nnoel, nnoel);  // matriz de rigidez do elemento
        std::vector<double>     Fe(nnoel);         // vetor de força do elemento
        std::vector<double>    phi(nnoel);
        std::vector<Gradient> dphi(nnoel);

        MeshElementType etype = (MeshElementType)pmesh.getElementType(iel);

        Point qpoint; // coordenadas do ponto de integracao
        double JxW = 0.0;

        equation_manager.global_indices(dof, conn_iel, global_indices);

        // calculando a função de forma e suas derivadas para elemento QUAD4 ou TRI3
        FEMGetQGauss(etype, qp, qw);

        Gradient velocity;
        velocity(0)  = sqrt(3.0) / 2.0;
        velocity(1)  = 1.0 / 2.0 ;
        double kd    = 1.0E-4;
        double sigma = 0.0;

        RealVector g;
        RealTensor G;

        // loop sobre os pontos de integração
        for (int q = 0; q < qp.size(); q++)
        {
            // calculando a função de forma e suas derivadas para o ponto de integração q
            FEMComputeFunctions(etype, qp[q], qw[q], coords_iel, qpoint, phi, dphi, JxW);
            FEMStab(etype, qp[q], coords_iel, g, G);

            // SUPG stabilization parameters
            double tau = (velocity) * (G.mult(velocity)) + (kd * kd) * (G.contract(G)); // + dt_stab*4.0/(dt*dt);

            // calculando a matriz de rigidez e o vetor de forca local
            for (int i = 0; i < nnoel; i++)
            {
                for (int j = 0; j < nnoel; j++)
                {
                    // Galerkin Formulation
                    Ke(i, j) += JxW * (phi[i] * (velocity * dphi[j]) + // w (a. grad u) - Termo convectivo
                                       kd * (dphi[i] * dphi[j])      + // Grad w Grad u - Termo difusivo
                                       sigma*phi[i]*phi[j]             // \sigma* w  u  -  Termo reação
                                       );

                    // SUPG Formulation
                    Ke(i, j) += JxW * tau * (velocity * dphi[i]) * (velocity * dphi[j] +      // Termo SUPG convecção
                                              sigma*phi[j]);                                  // Termo SUPG reação
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

    if(argc < 2)
    {
        return 0;
    }

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
    ImplicitSystem *implicit_system = new ImplicitSystem(*pmesh, "convection-diffusion");

    // Adiciona uma variável ao sistema
    int dof = implicit_system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc1(1, dof, "0.0", "x,y");
    DirichletBoundary bc2(2, dof, "1.0", "x,y");
    implicit_system->add_dirichlet_boundary(bc1);
    implicit_system->add_dirichlet_boundary(bc2);
    implicit_system->attach_assemble(assemble_convection_diffusion_reaction);

    // Inicializar o sistema 
    implicit_system->init();

    // Resolve o sistema de equações
    implicit_system->solve();

    implicit_system->write_result("solution");

    delete implicit_system;

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
