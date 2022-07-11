#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallel_mesh.h"
#include "implicit_system.h"
#include "dirichlet_boundary.h"


#include <assert.h>

static char help[] = "Empty Problem\n\n";

void fillXVec(Vec &x, ParallelMesh* pmesh, std::vector<unsigned int> &gindices)
{
    VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, pmesh->get_n_global_nodes(), &x);
    for(int i = 0; i < pmesh->get_n_elements(); i++)
    {
        unsigned int  csize = pmesh->getElementConnSize(i);
        unsigned int* conn = pmesh->getElementConn(i);
        for(int j = 0; j < csize; j++)
            VecSetValue(x,gindices[conn[j]], 1, ADD_VALUES);
    }

    VecAssemblyBegin(x);
    VecAssemblyEnd(x);
}

void fillAMat(Mat& A, ParallelMesh* pmesh, int processor_id)
{
    PetscViewer m_view;

    PetscInt rstart, rend;
    MatGetOwnershipRange(A, &rstart, &rend);

    // CSR Matriz Esparsa
    MatCreateAIJ(PETSC_COMM_WORLD, PETSC_DECIDE,PETSC_DECIDE,pmesh->get_n_global_nodes(),pmesh->get_n_global_nodes(),10,NULL, 10, NULL, &A);
    MatGetOwnershipRange(A, &rstart, &rend);

    std::cout << "MAT: processor ID " << processor_id << " Interval [" << rstart <<","<<rend <<"]\n" << std::flush;

    MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);
    MatSetOption(A, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE); // sugestão do petsc


    // Prencher a matriz:
    for(int i = 0; i < pmesh->get_n_global_nodes(); i++)
    {
        for(int j = 0; j < pmesh->get_n_global_nodes(); j++)
        {
            MatSetValue(A, i, j, i+j, INSERT_VALUES);
        }
    }

    MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);
}

void fillBMat(Mat& B, ParallelMesh* pmesh, std::vector<unsigned int>& gindices)
{
    PetscViewer m_view;
    PetscInt rstart, rend;
    MatGetOwnershipRange(B, &rstart, &rend);
    for(int i = 0 ; i < pmesh->get_n_elements() ; i++)
    {
        int connsize = pmesh->getElementConnSize(i);
        unsigned int* conn = pmesh->getElementConn(i);
        MatCreateAIJ(PETSC_COMM_WORLD,PETSC_DECIDE,PETSC_DECIDE,connsize,connsize,connsize,NULL, connsize, NULL, &B);
        MatGetOwnershipRange(B, &rstart, &rend);

        MatAssemblyBegin(B, MAT_FINAL_ASSEMBLY);
        MatAssemblyEnd(B, MAT_FINAL_ASSEMBLY);
        MatSetOption(B, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE); // sugestão do petsc

        // Prencher a matriz:
        for(int j = 0; j < connsize; j++)
        {
            for(int k = 0; k < connsize; k++)
            {
                MatSetValue(B, gindices[conn[i]], gindices[conn[j]], 1, INSERT_VALUES); // tentando montar matriz local
            }
        }

        MatAssemblyBegin(B, MAT_FINAL_ASSEMBLY);
        MatAssemblyEnd(B, MAT_FINAL_ASSEMBLY);
    }
}

int main(int argc, char* argv[])
{
    PetscErrorCode ierr;
    PetscViewer v_view, m_view;
    Vec v;
    IS is; // index set
    
    MeshPartition *parts = new MeshPartition();
    int processor_id, n_processors;
    Mesh        * mesh;
    ParallelMesh* pmesh;

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

    cout << "local nodes on processor " << MeshTools::processor_id() << ": "<< pmesh->get_n_local_nodes() << std::endl;



    ImplicitSystem* implicit_system = new ImplicitSystem(*pmesh, "poisson");    
    implicit_system->add_variable("u");
    implicit_system->init();

    EquationManager& em = implicit_system->get_equation_manager();

    for(int iel =0; iel < pmesh->get_n_elements(); iel++)
    {
        int connsize       = pmesh->getElementConnSize(iel);
        unsigned int* conn = pmesh->getElementConn(iel);
        int gindex[connsize];

        double Ke[3][3] = {0};
        double Fe[3] = {0};
        em.equation_indices(0, conn, connsize, gindex);

        for(int i = 0; i < connsize; i++)
        {
            assert(gindex[i] >= 0);
            Fe[i] += 1.0;

            for(int j = 0; j < connsize; j++)
            {
                Ke[i][j] += 1.0;
            }
        }

        if(processor_id == 0)
        {
            std::cout << "Elemento " << iel << ": ";
            std::cout  << "indices: " << gindex[0] << " " << gindex[1] << " " << gindex[2] << std::endl;
        }
        
        implicit_system->add_matrix_entry(connsize,gindex, connsize, gindex, &Ke[0][0]);
        implicit_system->add_rhs_entry(connsize,gindex,Fe);
        
    }
    implicit_system->print_matrix();
    //implicit_system->print_rhs();
    
    //implicit_system->solve();
    

    /*
    implicit_system->close();
    double *solution_ptr = implicit_system->get_local_solution_array();
    for(int i = 0 ; i < pmesh->get_n_global_nodes() ; i++)
    {
        std::cout << "Solution[" << i << "] = " << solution_ptr[i] << std::endl;
    }

    MeshIODataAppended info;
    info.addPointDataInfo("u", Float64, solution_ptr);
    pmesh->writePVTK("parallel", &info);
    pmesh->WritePMesh("parallel");

    implicit_system->restore_local_solution_array(&solution_ptr); // ??
    

    delete implicit_system;
    */

    if(MeshTools::processor_id() == 0)  delete mesh;
    delete pmesh;
    delete parts;
    MeshTools::Finalize();
    return 0;
}