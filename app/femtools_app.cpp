#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "implicit_system.h"
#include "dirichlet_boundary.h"
#include "fem_functions.h"

static char help[] = "Empty Problem\n\n";

// Poisson Equation
// -----------------
// \nabla u = f
// \u = 0 on boundary
// f(x,y)

double body_force(double x, double y)
{
    return 1.0;
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
    // Indicar as condições de contorno


    // Inicializar o sistema
    implicit_system->init();

    double k  = 1.0E-3; // difusão

    std::vector<double>& coords = pmesh->getCoord();
    int ndim = pmesh->getDim();

    // loop sobre os elementos da malha
    EquationManager& em = implicit_system->get_equation_manager();
    for(int iel =0; iel < pmesh->get_n_elements(); iel++)
    {
        int connsize       = pmesh->getElementConnSize(iel);
        unsigned int* conn = pmesh->getElementConn(iel);
        
        int gindex[connsize];
        int nqp = 1       ;  // usando um ponto de integração para TRI3
        double qp[nqp][2] ;  // coordenadas do ponto de integração
        double qw[nqp]    ;  // peso do ponto de integração
        
        // Obtendo as cordenadas do nós do elemento
        double xyz[connsize*3];
        for(int i = 0; i < connsize; i++)
        {
            int index    = conn[i];
            xyz[i*3]   = coords[index*3];
            xyz[i*3+1] = coords[index*3+1];
            xyz[i*3+2] = coords[index*3+2];
        }

        double Ke[connsize][connsize] = {0}; // matriz de rigidez local
        double Fe[connsize]           = {0}; // vetor de forca local
        double phi[connsize]          = {0}; // vetor de solucao local
        double dphi[connsize][2]   = {0};  // derivada da solucao local
        double xyqp[2] = {0};              // coordenadas do ponto de integracao
        double JxW = 0;

        // calculando a função de forma e suas derivadas para elemento TRI3
        QGaussTri3(nqp,qp,qw);
        ComputeTRI3Functions(qp[0],qw[0],xyz,xyqp,phi,dphi,&JxW);
        em.equation_indices(dof, conn, connsize, gindex);

        // calculando a matriz de rigidez e o vetor de forca local

        for(int i = 0; i < connsize; i++)
        {
            double fxy = body_force(xyqp[0], xyqp[1]);

            Fe[i] += JxW*fxy*phi[i];

            for(int j = 0; j < connsize; j++)
            {
                Ke[i][j] += JxW*k*(dphi[i][0]*dphi[j][0] + dphi[i][1]*dphi[j][1]);
            }
        }

        implicit_system->add_matrix_entry(connsize,gindex, connsize, gindex, &Ke[0][0]);
        implicit_system->add_rhs_entry(connsize,gindex,Fe);
        
    }
    implicit_system->print_matrix();
    implicit_system->print_rhs();
    
    implicit_system->solve();
    

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