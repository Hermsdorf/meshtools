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

static char help[] = "Convecção-difusão-reaçao transiente\n\n";


void assemble_transport(TransientImplicitSystem* system)
{
    auto pmesh = system->get_mesh();
    int  ndim  =  pmesh.getDim();

    // Gerencia as numerações das equações do sistema
    auto equation_manager = system->get_equation_manager();
    int dof  = 0;

    int n_elements = pmesh.get_n_elements();
    bool flag = true;

    double *old_solution = system->get_old_solution_array();

    // loop sobre os elementos da malha por cores
    for (int iel = 0; iel < n_elements; iel++)
    {
        std::vector<Point> coords_iel;
        std::vector<unsigned int> conn_iel;

        pmesh.get_element_connectivity(iel, conn_iel);
        pmesh.get_element_coordinates(iel, coords_iel);
        int nnoel = conn_iel.size();

        std::vector<int>        global_indices;
        std::vector<int>        local_indices;
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
        equation_manager.local_indices(dof, conn_iel, local_indices);

        // calculando a função de forma e suas derivadas para elemento QUAD4 ou TRI3
        FEMGetQGauss(etype, qp, qw);

        Gradient velocity;
        velocity(0)  = sqrt(3.0) / 2.0;
        velocity(1)  = 1.0 / 2.0 ;
        double kd    = 1.0E-4;
        double sigma = 0.0;
        double theta = 0.5;
        double dt    = system->get_time_step();

        RealVector g;
        RealTensor G;

        // loop sobre os pontos de integração
        for (int q = 0; q < qp.size(); q++)
        {
            // calculando a função de forma e suas derivadas para o ponto de integração q
            FEMComputeFunctions(etype, qp[q], qw[q], coords_iel, qpoint, phi, dphi, JxW);
            FEMStab(etype, qp[q], coords_iel, g, G);

            // SUPG stabilization parameters
            double tau = (velocity) * (G.mult(velocity)) + (kd * kd) * (G.contract(G)) + 4.0/(dt*dt);
            double u_old = 0.0;
            Gradient grad_u_old;

            for (int i = 0; i < local_indices.size(); i++)
            {
                u_old += old_solution[local_indices[i]]*phi[i];
                grad_u_old(0) +=  old_solution[local_indices[i]]*dphi[i](0);
                grad_u_old(1) +=  old_solution[local_indices[i]]*dphi[i](1);
            }

            // calculando a matriz de rigidez e o vetor de forca local
            for (int i = 0; i < local_indices.size(); i++)
            {
                // Galerkin 
                Fe[i]   +=  JxW*(phi[i]*u_old -
                                     (1.0-theta)*dt*(phi[i]*(velocity * grad_u_old) +
                                                   kd*(dphi[i] * grad_u_old)  +
                                                   sigma*phi[i]*u_old)
                                   );

                // SUPG 
                Fe[i]  +=  JxW * tau * (velocity * dphi[i])*(
                                     u_old - (1.0-theta)*dt*(
                                                                phi[i]*(velocity * grad_u_old) +
                                                                sigma*phi[i]*u_old)
                                                            );

                for (int j = 0; j < local_indices.size(); j++)
                {
                    // Galerkin Formulation
                    Ke(i, j) += JxW * ( phi[i]*phi[j] +
                                        theta*dt*(phi[i] * (velocity * dphi[j]) + // w (a. grad u) - Termo convectivo
                                            kd * (dphi[i] * dphi[j])      +       // Grad w Grad u - Termo difusivo
                                            sigma*phi[i]*phi[j]                   // \sigma* w  u  -  Termo reação
                                        )           
                                       );

                    // SUPG Formulation
                    Ke(i, j) += JxW * tau * (velocity * dphi[i]) * (
                                     phi[j]             +     // Termo de massa SUPG
                                     velocity * dphi[j] +     // Termo SUPG convecção
                                     sigma*phi[j]);           // Termo SUPG reação
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
    TransientImplicitSystem *system = new TransientImplicitSystem(*pmesh, "transport");

    // Adiciona uma variável ao sistema
    int dof = system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc1(1, dof, "0.0", "x,y");
    InitialCondition  ic1(2, dof, "1.0", "x,y");

    system->add_dirichlet_boundary(bc1);
    system->add_initial_condition(ic1);
    system->attach_assemble(assemble_transport);


    // Inicializar o sistema 
    system->init();
    system->set_final_time(1.0);
    system->set_time_step(0.005);
    system->write_vtk("initial");
/*
    while(system->get_time() < system->get_final_time())
    {
        system->solve_time_step();
        MeshTools::Printf("Solved time : %.3f\n", system->get_time());
    }


    // Resolve o sistema de equações

    system->write_vtk("solution");
*/

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
