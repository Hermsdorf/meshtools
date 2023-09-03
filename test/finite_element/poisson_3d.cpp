
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
#include "xdmf_writer.h"

#include "test_config.h"


static char help[] = "2D Poisson Problem\n\n";

// Poisson Equation
// -----------------
// \nabla u = f  em um dominio \Omega = [0,1] x [0,1]
// \u = 0 na superficie de contorno \Lambda
// f(x,y) é tal que a solução exata é dada por
//   100.0 * x * (1.0 - x) * y * (1.0 - y) * z * (1.0 - z);
//
double exact_solution(double x, double y, double z, double t = 0.0)
{
    return 100.0 * x * (1.0 - x) * y * (1.0 - y) * z * (1.0 - z);
}

double derivative_exact_solution(unsigned int i, double x, double y, double z = 0.0, double t = 0.0)
{
    switch (i)
    {
    case 0:
        return 100.0 * (1.0 - 2.0 * x) * y * (1.0 - y) * z * (1.0 - z);
    case 1:
        return 100.0 * x * (1.0 - x) * (1.0 - 2.0 * y) * z * (1.0 - z);
    case 2:
        return 100.0 * x * (1.0 - x) * y * (1.0 - y) * (1.0 - 2.0 * z);
    default:
        return 0.0;
    }
}

// Calculado usando aproximação de diferencas finitas
double body_force(double x, double y, double z)
{
    const double eps = 1.0E-5;
    const double fxy = -(exact_solution(x, y - eps, z) +
                         exact_solution(x, y + eps, z) +
                         exact_solution(x - eps, y, z) +
                         exact_solution(x + eps, y, z) +
                         exact_solution(x, y, z - eps) +
                         exact_solution(x, y, z + eps) -
                         6. * exact_solution(x, y, z)) /
                       eps / eps;
    return fxy;
}

double compute_H1_error(ImplicitSystem &system, int dof)
{
    auto pmesh                  = system.get_mesh();
    auto eq_manager = system.get_equation_manager();

    // Compute the error estimator
    // ===========================
    //
    // The error estimator is computed as the difference between the exact solution and the
    // numerical solution.
    //   - e = \int_\Omega_e ||(grad.u - grad.u_h)|| dx

    // Get the solution vector
    double *solution = system.get_local_solution_array();

    int n_elements = pmesh.get_n_elements();
    int n_dofs = eq_manager.get_n_dofs();

    QGauss qrule;
    FEMFunction fem;

    std::vector<double>   & phi = fem.get_phi();
    std::vector<Gradient> & dphi= fem.get_dphi();
    double                & JxW    = fem.get_JxW();
    Point                 & qpoint = fem.get_xyz();

    double error_per_processor = 0.0;

    for (int iel = 0; iel < n_elements; ++iel)
    {

        Element elem;
        pmesh.getElement(iel,elem);

        double error_per_element = 0.0;
   
        std::vector<int> local_indices;


        eq_manager.local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento elem
        qrule.reset(elem);

        // loop sobre os pontos de integração
        for (int q = 0; q < qrule.n_points(); q++)
        {

            // calculando a função de forma e suas derivadas para o ponto de integração q
           fem.ComputeFunction(elem,qrule.get(q));

            Gradient gradu;
            Gradient gradu_h;
            for (int i = 0; i < local_indices.size(); i++)
            {
                gradu_h(0) += solution[local_indices[i]] * dphi[i](0);
                gradu_h(1) += solution[local_indices[i]] * dphi[i](1);
            }

            gradu(0) = derivative_exact_solution(0, qpoint(0), qpoint(1), qpoint(2));
            gradu(1) = derivative_exact_solution(1, qpoint(0), qpoint(1), qpoint(2));
            gradu(2) = derivative_exact_solution(2, qpoint(0), qpoint(1), qpoint(2));

            double val_error = (gradu - gradu_h).norm(); // The norm of the difference between derivative of exact solution and the derivative of numerical solution
            error_per_element += val_error * JxW;
        }
        error_per_processor += error_per_element;
    }

    double h1_error = error_per_processor;

    MPI_Allreduce(&error_per_processor, &h1_error, 1, MPI_DOUBLE, MPI_SUM, MeshTools::Comm());

    return h1_error;
}

double compute_L2_error(ImplicitSystem &system, int dof)
{
    auto pmesh      = system.get_mesh();
    auto eq_manager = system.get_equation_manager();

    // Compute the error estimator
    // =========================
    //
    // The error estimator is computed as the difference between the exact solution and the
    // numerical solution.
    //  - e = \int_\Omega_e (u - u_h)^2 dx
    //  - E = \sqrt(e)

    // Get the solution vector
    double *solution = system.get_local_solution_array();

    int n_elements = pmesh.get_n_elements();
    int n_dofs = eq_manager.get_n_dofs();

    double error_per_processor = 0.0;

    QGauss qrule;
    FEMFunction fem;

    std::vector<double>   & phi = fem.get_phi();
    std::vector<Gradient> & dphi= fem.get_dphi();
    double                & JxW    = fem.get_JxW();
    Point                 & qpoint = fem.get_xyz();

    for (int iel = 0; iel < n_elements; ++iel)
    {

        Element elem;
        pmesh.getElement(iel,elem);

        double error_per_element = 0.0;
   
        std::vector<int> local_indices;


        eq_manager.local_indices(dof, elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento elem
        qrule.reset(elem);

        // loop sobre os pontos de integração
        for (int q = 0; q < qrule.n_points(); q++)
        {

            // calculando a função de forma e suas derivadas para o ponto de integração q
           fem.ComputeFunction(elem,qrule.get(q));

            double u_h = 0.0;
            for (int i = 0; i < local_indices.size(); i++)
                u_h += solution[local_indices[i]] * phi[i];

            double val_error = (u_h - exact_solution(qpoint(0), qpoint(1), qpoint(2)));
            error_per_element += (val_error * val_error) * JxW;
        }
        error_per_processor += error_per_element;
    }

    double l2_error = error_per_processor;

    MPI_Allreduce(&error_per_processor, &l2_error, 1, MPI_DOUBLE, MPI_SUM, MeshTools::Comm());

    return sqrt(l2_error);
}


void assemble_poisson(ImplicitSystem* system)
{
    auto pmesh = system->get_mesh();
    int ndim   = pmesh.getDim();
    int dof    = 0;
    // Gerencia as numerações das equações do sistema
    EquationManager &equation_manager = system->get_equation_manager();
    QGauss qrule;
    FEMFunction fem;
        
    std::vector<double>   & phi = fem.get_phi();
    std::vector<Gradient> & dphi= fem.get_dphi();
    double                & JxW    = fem.get_JxW();
    Point                 & qpoint = fem.get_xyz();

    // loop sobre os elementos da malha
    for (int iel = 0; iel < pmesh.get_n_elements(); iel++)
    {
        Element elem;
        pmesh.getElement(iel,elem);
        
        int nnoel = elem.n_nodes();

        std::vector<int>        global_indices;
        std::vector<int>        local_indices;
        DenseMatrix<double>     Ke(nnoel, nnoel);  // matriz de rigidez do elemento
        std::vector<double>     Fe(nnoel);          // vetor de força do elemento

        equation_manager.global_indices(dof, elem.connectivity(), global_indices);
        equation_manager.local_indices(dof,  elem.connectivity(), local_indices);

        // Obtem pontos de integração para elemento elem
        qrule.reset(elem);

        // loop sobre os pontos de integração
        for (int q = 0; q < qrule.n_points(); q++)
        {
            // calculando a função de forma e suas derivadas para o ponto de integração q
            fem.ComputeFunction(elem,qrule.get(q));

            // calculando a matriz de rigidez e o vetor de forca local
            for (int i = 0; i < local_indices.size(); i++)
            {
                // avaliando a função fonte
                double fxy = body_force(qpoint(0), qpoint(1), qpoint(2));
                Fe[i] += JxW * fxy * phi[i];

                for (int j = 0; j < local_indices.size(); j++)
                    Ke(i, j) += JxW * (dphi[i] * dphi[j]);
            }
        }

        // Inserindo a matriz de rigidez e o vetor de forca no sistema
        system->add_matrix_entry(global_indices, global_indices, Ke.get_data());
        system->add_rhs_entry(global_indices, Fe.data());
    }

}

/*
 *   p = std::log(std::fabs(erro[i - 1] / erro[i])) / std::log(2.0));
 */

int poisson(int argc, char *argv[], double& error_vec_l2, double& error_vec_h1)
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
        string test_mesh_dir = TEST_MESH_DIR;
        test_mesh_dir.append("poisson_3d/poisson3d.msh");
        mesh = new Mesh(test_mesh_dir);

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);


    // Cria o sistema de equações implicito
    ImplicitSystem *implicit_system = new ImplicitSystem(*pmesh, "poisson");

    // Adiciona uma variável ao sistema
    int dof = implicit_system->add_variable("u");

    // Adiciona uma condição de contorno ao sistema
    // Aplica a função g = 0 para a variável u no contorno identificado com 1.
    DirichletBoundary bc(1, dof, "100*x*y*z*(1-x)*(1-y)*(1-z)", "x,y");
    implicit_system->add_dirichlet_boundary(bc);
    implicit_system->attach_assemble(assemble_poisson);

    // Inicializar o sistema
    // Necessário para calcular alocar o sistema
    implicit_system->init();

    // Resolve o sistema de equações
    implicit_system->solve();

    implicit_system->write_result("poisson_3d");

    double l2_error = compute_L2_error(*implicit_system, 0);
    double h1_error = compute_H1_error(*implicit_system, 0);
    MeshTools::Printf( "Erro |u - u_exato| = %e\n", l2_error);
    MeshTools::Printf( "Erro |grad.u - grad.u_exato| = %e\n", h1_error);
    error_vec_l2 = l2_error;
    error_vec_h1 = h1_error;

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
    double error_L2, error_H1;
    poisson(argc, argv, error_L2, error_H1);
    MeshTools::Finalize();

    return 0;
}
