
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



// Poisson Equation
// -----------------
// \nabla u = f  em um dominio \Omega = [0,1] x [0,1]
// \u = 0 na superficie de contorno \Lambda
// f(x,y) é tal que a solução exata é dada por
//   100.0 * x * (1.0 - x) * y * (1.0 - y);
//
double exact_solution(double x, double y, double z=0.0, double t=0.0)
{
    return 100.0 * x * (1.0 - x) * y * (1.0 - y);
}


double derivative_exact_solution(unsigned int i, double x, double y, double z=0.0, double t=0.0)
{
    switch (i)
    {
        case 0: return 100.0 * (1.0 - 2.0 * x) * y * (1.0 - y);
        case 1: return 100.0 * x * (1.0 - x) * (1.0 - 2.0 * y);
        default: return 0.0;
    }
}



// Calculado usando aproximação de diferencas finitas
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


// double compute_H1_error(ImplicitSystem& system, int dof)
/*
    // The error estimator is computed as the difference between the exact solution and the
    // numerical solution.
    //  - e = \int_\Omega_e ||(grad.u - grad.u_h)|| dx
    //  -

*/

// 
double compute_L2_error(ImplicitSystem& system, int dof)
{

    ParallelMesh&         pmesh = system.get_mesh();
    EquationManager& eq_manager = system.get_equation_manager();

    // Compute the error estimator
    // =========================
    //
    // The error estimator is computed as the difference between the exact solution and the
    // numerical solution.
    //  - e = \int_\Omega_e (u - u_h)^2 dx
    //  - E = \sqrt(e)

    // Get the solution vector
    double* solution = system.get_local_solution_array();

    int n_elements = pmesh.get_n_elements();
    int n_dofs     = eq_manager.get_n_dofs();
     
    double error_per_processor = 0.0;

    for(int iel = 0; iel < n_elements; ++iel)
    {

        double error_per_element = 0.0;
        std::vector<Point>            coords_iel;
        std::vector<unsigned int>     connectivity;
        pmesh.get_element_connectivity(iel, connectivity);
        pmesh.get_element_coordinates(iel, coords_iel);
        int nnoel = connectivity.size();
        std::vector<int> local_indices;

        std::vector<Point>    qp;  // coordenadas do ponto de integração
        std::vector<double>   qw;   // peso do ponto de integração
        std::vector<double>   phi(nnoel);
        std::vector<Gradient> dphi(nnoel);

        MeshElementType etype = (MeshElementType) pmesh.getElementType(iel);
        Point qpoint;    // coordenadas do ponto de integracao
        double JxW      = 0;

        eq_manager.local_indices(dof, connectivity, local_indices);

        // calculando a função de forma e suas derivadas para elemento QUAD4 ou TRI3
        FEMGetQGauss(etype,qp,qw);

     
        // loop sobre os pontos de integração
        for(int q = 0; q < qp.size(); q++)
        {

             // calculando a função de forma e suas derivadas para o ponto de integração q
            FEMComputeFunctions(etype, qp[q],qw[q],coords_iel,qpoint,phi,dphi,JxW);
           
            double u_h = 0.0;
            Gradient gradu;
            Gradient gradu_h;
            for(int i = 0; i < nnoel; i++){
                u_h        += solution[local_indices[i]] * phi[i];
                gradu_h(0) += solution[local_indices[i]] * dphi[i](0);
                gradu_h(1) += solution[local_indices[i]] * dphi[i](1);
            }

            gradu(0) = derivative_exact_solution(0, qpoint(0), qpoint(1));
            gradu(1) = derivative_exact_solution(1, qpoint(0), qpoint(1));

            //double h1_error  = (gradu - gradu_h).norm()
            double val_error = (u_h - exact_solution(qpoint(0), qpoint(1)));
            error_per_element += (val_error * val_error) * JxW;

        }
        error_per_processor += error_per_element;
        //printf("Error estimator: %e\n", error_per_processor);
    }

    double error = error_per_processor;
    
    MPI_Allreduce(&error_per_processor, &error, 1, MPI_DOUBLE, MPI_SUM,MeshTools::Comm());
 
    //printf("Error: %e\n", sqrt(error));
    return sqrt(error);
}

/*
 *   p = std::log(std::fabs(erro[i - 1] / erro[i])) / std::log(2.0));
 */

int poisson(int argc, char* argv[])
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


    // Cria o sistema de equações implicito
    ImplicitSystem* implicit_system = new ImplicitSystem(*pmesh, "poisson");    

    // Adiciona uma variável ao sistema
    int dof = implicit_system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc(1,dof,"100*x*y*(1-x)*(1-y)","x,y");
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
        double JxW      = 0;

        equation_manager.global_indices(dof, conn_iel, global_indices);

        // calculando a função de forma e suas derivadas para elemento QUAD4 ou TRI3
        FEMGetQGauss(etype,qp,qw);

        // loop sobre os pontos de integração
        for(int q = 0; q < qp.size(); q++)
        {

             // calculando a função de forma e suas derivadas para o ponto de integração q
            FEMComputeFunctions(etype, qp[q],qw[q],coords_iel,qpoint,phi,dphi,JxW);
           
            // calculando a matriz de rigidez e o vetor de forca local
            for(int i = 0; i < nnoel; i++)
            {
                // avaliando a função fonte
                double fxy = body_force(qpoint(0), qpoint(1));
                Fe[i] += JxW*fxy*phi[i];
                for(int j = 0; j < nnoel; j++)
                    Ke(i,j) += JxW*(dphi[i]*dphi[j]);
            }
        }

        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        implicit_system->add_matrix_entry(global_indices,global_indices, Ke.get_data() );
        implicit_system->add_rhs_entry(global_indices,Fe.data());
        
    }

    // Resolve o sistema de equações
    implicit_system->solve();

    double erro = compute_L2_estimator(*implicit_system, 0);
    PetscPrintf(MeshTools::Comm(), "Erro |u - uxato| = : %e\n", erro);
    //implicit_system->write_vtk("solution");

    delete implicit_system;

    if(MeshTools::processor_id() == 0)  delete mesh;
    delete pmesh;
    delete parts;

    MeshTools::Finalize();
    return 0;
}

int main(int argc, char *argv[])
{

    return poisson(argc, argv);
}

