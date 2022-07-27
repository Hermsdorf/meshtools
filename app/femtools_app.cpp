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

static char help[] = "Empty Problem\n\n";



// Poisson Equation
// -----------------
// \nabla u = f
// \u = 0 on boundary
// f(x,y)

double exact_solution(double x, double y)
{
    return 100.0 * x * (1.0 - x) * y * (1.0 - y);
}

// 
double body_force(double x, double y)
{
    const double eps = 1.0E-5;
    const double fxy = -(exact_solution(x, y-eps) +
                       exact_solution(x, y+eps) +
                        exact_solution(x-eps, y) +
                        exact_solution(x+eps, y) -
                        4.*exact_solution(x, y))/eps/eps;
    return fxy;
}

int main(int argc, char* argv[])
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

    if(MeshTools::n_processors() == 1)
        pmesh->WritePMesh("serial");
    else
        pmesh->WritePMesh("pmesh");

    // Cria o sistema de equações implicito
    ImplicitSystem* implicit_system = new ImplicitSystem(*pmesh, "poisson");    

    // Adiciona uma variável ao sistema
    int dof = implicit_system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc(1,dof,"0.0","");
    implicit_system->add_dirichlet_boundary(bc);

    

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
        const unsigned int *connectivity = pmesh->getElementConn(iel);
        //TODO: pmesh->get_element_connectivity(iel, connectivity);
        pmesh->get_element_coordinates(iel, coords_iel);
        int nnoel = coords_iel.size();


        int equation_indices[nnoel];
        std::vector<Point>    qp;  // coordenadas do ponto de integração
        std::vector<double>   qw;   // peso do ponto de integração
        DenseMatrix<double>   Ke(nnoel,nnoel); // matriz de rigidez do elemento
        std::vector<double>   Fe(nnoel);       // vetor de força do elemento
        std::vector<double>   phi(nnoel);
        std::vector<Gradient> dphi(nnoel);

        Point qpoint;    // coordenadas do ponto de integracao
        double JxW      = 0;

        equation_manager.equation_indices(dof, nnoel, connectivity, equation_indices);

        // calculando a função de forma e suas derivadas para elemento TRI3
        TRI3DefaultQGauss(qp,qw);

        // loop sobre os pontos de integração
        for(int q = 0; q < qp.size(); q++)
        {

            // calculando a função de forma e suas derivadas para o ponto de integração q
            TRI3ComputeFunctions(qp[q],qw[q],coords_iel,qpoint,phi,dphi,JxW);
    
            // calculando a matriz de rigidez e o vetor de forca local
            for(int i = 0; i < nnoel; i++)
            {
                // avaliando a função fonte
                double fxy = body_force(qpoint(0), qpoint(1));

                Fe[i] += JxW*fxy*phi[i];

                for(int j = 0; j < nnoel; j++)
                {
                    Ke(i,j) += JxW*(dphi[i]*dphi[j]);
                }
            }
        }

        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        implicit_system->add_matrix_entry(nnoel,equation_indices, nnoel, equation_indices, &Ke(0,0) );
        implicit_system->add_rhs_entry(nnoel,equation_indices,Fe.data());
        
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