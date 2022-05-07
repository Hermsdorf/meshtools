#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallel_mesh.h"
#include "dof_manager.h"
#include "dirichlet_boundary.h"

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
    Mesh*         mesh;
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

    if(n_processors > 1 ) 
    {
        // Malha gerada pelo processo mestre é distribuida
        // para os demais processos. 
        pmesh = parts->DistributedMesh(mesh);

        std::string str(argv[1]);
        str.resize(str.length()-4);
        pmesh->setFilename(str);
    }

    // Sistema de fluido: NS
    // 2D: 3 Dofs
    //  0 : vel_x
    //  1 : vel_y
    //  2 : pressao
    MeshIODataAppended info;
    pmesh->writePVTK("parallel", &info);
    pmesh->WritePMesh("parallel");
       

    DofManager* dm = new DofManager(*pmesh);
    dm->set_n_dofs(3);

    auto physical_data = pmesh->getPhysicalMap();
   for(int i = 0 ; i < physical_data.size() ; i++)
    {
        if(physical_data[i].first == 1)
        {
            
            DirichletBoundary* dirichlet1 = new DirichletBoundary(physical_data[i].first, 0, "1", "");
            DirichletBoundary* dirichlet2 = new DirichletBoundary(physical_data[i].first, 1, "0", "");
            DirichletBoundary* dirichlet3 = new DirichletBoundary(physical_data[i].first, 2, "0", "");
            dm->add_dirichlet_boundary(*dirichlet1);
            dm->add_dirichlet_boundary(*dirichlet2);
            dm->add_dirichlet_boundary(*dirichlet3);
            if (processor_id == 0)
                 std::cout << "Dirichlet boundary: " << dirichlet1->get_dof_id() << ", " << dirichlet1->get_boundary_id() << std::endl;
            }
    }
    
    unsigned int* onnz;
    unsigned int* dnnz;
    
    dm->prepare_to_use();
    //dm->calculate_onnz_dnnz(onnz, dnnz);

    // Criar o Sistema de Equações
    // std::vector<unsigned int> &gindices = pmesh->getLocal2Global();

    // Vec x;                                  // numero de nos totais
    // std::cout << "[" << processor_id << "] Calculating x vector...\n";
    // fillXVec(x, pmesh, gindices);
    // VecView(x, v_view);

    // Mat A;
    // std::cout << "[" << processor_id << "] Calculating A matrix...\n";
    // fillAMat(A, pmesh, processor_id);
    // MatView(A, m_view);

   
    // VecDestroy(&x);
    // MatDestroy(&A);
    //MeshTools::Finalize();
    //return 0;
}