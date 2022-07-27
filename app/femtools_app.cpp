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
   
    Mesh          *mesh;
    ParallelMesh  *pmesh;
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

    ImplicitSystem* implicit_system = new ImplicitSystem(*pmesh, "poisson");    
    int dof = implicit_system->add_variable("u");
    DirichletBoundary bc(1,dof,"0.0","");
    implicit_system->add_dirichlet_boundary(bc);

    // Indicar as condições de contorno

    // Inicializar o sistema
    implicit_system->init();

    std::vector<double>& coords = pmesh->getCoord();
    int ndim                    = pmesh->getDim();

    // loop sobre os elementos da malha
    EquationManager& em = implicit_system->get_equation_manager();

    bool flag = true;
    for(int iel =0; iel < pmesh->get_n_elements(); iel++)
    {
        int nnoel          = pmesh->getElementConnSize(iel);
        unsigned int* conn = pmesh->getElementConn(iel);
        
        int equation_indices[nnoel];

        std::vector<Point>  qp;  // coordenadas do ponto de integração
        std::vector<double> qw;   // peso do ponto de integração
        
        //Point centroid;
        // Obtendo as cordenadas do nós do elemento
        std::vector<Point> coords_iel(nnoel);
        const double one3 = 1.0/3.0;
        for(int i = 0; i < nnoel; i++)
        {
            int index    = conn[i];
            coords_iel[i](0)   = coords[index*3];
            coords_iel[i](1)   = coords[index*3+1];
            coords_iel[i](2)   = coords[index*3+2];

            // centroid(0) += coords[index*3]*one3;
            // centroid(1) += coords[index*3+1]*one3;
            // centroid(2) += coords[index*3+2]*one3;
        }

        DenseMatrix<double>   Ke(nnoel,nnoel); // matriz de rigidez do elemento
        std::vector<double>   Fe(nnoel);       // vetor de força do elemento
        std::vector<double>   phi(nnoel);
        std::vector<Gradient> dphi(nnoel);

        Point xy_gauss;    // coordenadas do ponto de integracao
        double JxW      = 0;

        em.equation_indices(dof, conn, nnoel, equation_indices);

        // calculando a função de forma e suas derivadas para elemento TRI3
        TRI3DefaultQGauss(qp,qw);
        for(int q = 0; q < qp.size(); q++)
        {
            TRI3ComputeFunctions(qp[q],qw[q],coords_iel,xy_gauss,phi,dphi,JxW);
    
            // calculando a matriz de rigidez e o vetor de forca local
            for(int i = 0; i < nnoel; i++)
            {
                double fxy = body_force(xy_gauss(0), xy_gauss(1));

                Fe[i] += JxW*fxy*phi[i];

                for(int j = 0; j < nnoel; j++)
                {
                    Ke(i,j) += JxW*(dphi[i]*dphi[j]);
                }
            }
        }

        // if(flag)
        // {
        //     std:: cout << "\nElement = " << iel << " Centroid = (" << centroid(0) <<" , " << centroid(1) <<")" << std::endl;
        //     Ke.print();
        //     //flag = false;
        // }

        implicit_system->add_matrix_entry(nnoel,equation_indices, nnoel, equation_indices, &Ke(0,0) );
        implicit_system->add_rhs_entry(nnoel,equation_indices,Fe.data());
        
    }

    
    implicit_system->solve();
    
    //implicit_system->print_matrix();
    //implicit_system->print_rhs();
    
    double *solution_ptr = implicit_system->get_local_solution_array();
    MeshIODataAppended info;
    info.addPointDataInfo("u", Float64, solution_ptr);
    pmesh->writePVTK("parallel", &info);
    implicit_system->restore_local_solution_array(&solution_ptr);
    
    delete implicit_system;

    if(MeshTools::processor_id() == 0)  delete mesh;
    delete pmesh;
    delete parts;

    MeshTools::Finalize();
    return 0;
}