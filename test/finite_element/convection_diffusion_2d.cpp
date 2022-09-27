
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

static char help[] = "2D Poisson Problem\n\n";


int convection_diffusion(int argc, char* argv[])
{
    PetscErrorCode ierr;
    MeshPartition *parts = new MeshPartition();
   
    Mesh          *mesh;  // serial mesh
    ParallelMesh  *pmesh; // parallel mesh
    int processor_id, n_processors;

    MeshTools::Init(argc,argv);
    processor_id = MeshTools::processor_id();
    n_processors = MeshTools::n_processors();

    if(processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha. 
        mesh = new Mesh(argv[1]);
        
        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if(n_processors > 1) {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);

    //pmesh->writePVTK("mesh");

    // Cria o sistema de equações implicito
    ImplicitSystem* implicit_system = new ImplicitSystem(*pmesh, "convection-diffusion");    

    // Adiciona uma variável ao sistema
    int dof = implicit_system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc1(1,dof,"0.0","x,y");
    DirichletBoundary bc2(2,dof,"1.0","x,y");
    implicit_system->add_dirichlet_boundary(bc1);
    implicit_system->add_dirichlet_boundary(bc2);

    
    // Inicializar o sistema
    // Necessário para calcular alocar o sistema
    implicit_system->init();

    int ndim                    = pmesh->getDim();

    // Gerencia as numerações das equações do sistema
    EquationManager& equation_manager = implicit_system->get_equation_manager();

    bool flag = true;
    // loop sobre os elementos da malha
    for(int iel =0; iel < pmesh->get_n_elements(); iel++)
    {

        std::vector<Point>            coords_iel;
        std::vector<unsigned int>     conn_iel;
        //const unsigned int *connectivity = pmesh->getElementConn(iel);
        pmesh->get_element_connectivity(iel, conn_iel);
        pmesh->get_element_coordinates(iel, coords_iel);
        int nnoel = conn_iel.size();


        std::vector<int> global_indices;
        std::vector<Point>    qp;  // coordenadas do ponto de integração
        std::vector<double>   qw;   // peso do ponto de integração
        DenseMatrix<double>   Ke(nnoel,nnoel); // matriz de rigidez do elemento
        std::vector<double>   Fe(nnoel);       // vetor de força do elemento
        std::vector<double>   phi(nnoel);
        std::vector<Gradient> dphi(nnoel);

        MeshElementType etype = (MeshElementType) pmesh->getElementType(iel);


        Point qpoint;    // coordenadas do ponto de integracao
        double JxW      = 0.0;

        equation_manager.global_indices(dof, conn_iel, global_indices);

        // calculando a função de forma e suas derivadas para elemento QUAD4 ou TRI3
        FEMGetQGauss(etype,qp,qw);

        Gradient velocity;
        velocity(0) = sqrt(2.0)/2.0;
        velocity(1) = velocity(0);
        double kd   = 1.0E-3;


        // loop sobre os pontos de integração
        for(int q = 0; q < qp.size(); q++)
        {

             // calculando a função de forma e suas derivadas para o ponto de integração q
            FEMComputeFunctions(etype, qp[q],qw[q],coords_iel,qpoint,phi,dphi,JxW);
           
            // calculando a matriz de rigidez e o vetor de forca local
            for(int i = 0; i < nnoel; i++)
            {
    
                for(int j = 0; j < nnoel; j++)
                    Ke(i,j) += JxW*(phi[i]*( velocity*dphi[j]) +       // w (a. grad u) - Termo convectivo
                                            kd*(dphi[i]*dphi[j])  );   // Grad w Grad u - Termo difusivo 
            }
        }

        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        implicit_system->add_matrix_entry(global_indices,global_indices, Ke.get_data() );
        implicit_system->add_rhs_entry(global_indices,Fe.data());
        
    }

    // Resolve o sistema de equações
    implicit_system->solve();

    implicit_system->write_vtk("solution");

    delete implicit_system;

    if(MeshTools::processor_id() == 0)  delete mesh;
    delete pmesh;
    delete parts;

    MeshTools::Finalize();
    return 0;
}

int main(int argc, char *argv[])
{

    return convection_diffusion(argc, argv);
}

