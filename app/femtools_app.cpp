#include "petsc.h"

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallel_mesh.h"

static char help[] = "Empty Problem\n\n";

int main(int argc, char* argv[])
{
    PetscErrorCode ierr;
    Vec v;
    PetscViewer v_view;
    IS is; // index set
    
    Mesh_partition_t *parts = new Mesh_partition_t();
    int processor_id, n_processors;
    Mesh* mesh;
    ParallelMesh* pmesh;

    ierr = PetscInitialize(&argc,&argv,nullptr,help); CHKERRQ(ierr);
    MPI_Comm_rank(PETSC_COMM_WORLD,&processor_id);
    MPI_Comm_size(PETSC_COMM_WORLD,&n_processors);

    if(processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha. 
        mesh = new Mesh(argv[1]);
        
        std::cout << "MESH\n";
        std::cout << "  nodes: \n    ";
        std::vector<double> coord = mesh->getCoord();
        for(int i = 0 ; i < coord.size() ; i++)
        {
            std::cout << coord[i] << " ";
            if((i+1)%3 == 0)
                std::cout << "\n    ";
        }        

        std::cout << "\n  elems: \n";
        unsigned int nelem = mesh->get_n_elements();
        unsigned int* conn;
        unsigned int connsize;
        for(int i = 0 ; i < nelem ; i++)
        {
            conn = mesh->getElementConn(i);
            connsize = mesh->getElementConnSize(i);
            std::cout << "   elem " << i+1 << ": ";
            for(int j = 0 ; j < connsize ; j++)
                std::cout << conn[j] << " ";
            std::cout << "\n";
        }

        // Aplica a reordenação nodal considerando o algoritmo
        // escolhido pelo usuário
    
        mesh->MeshReordering(RCM);
    
        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if(n_processors > 1) {
            parts->MeshPartitionerInternal(mesh,n_processors);
        }
    }

    if(n_processors > 1 ) 
    {
        // Malha gerada pelo processo mestre é distribuida
        // para os demais processos. 
        pmesh = parts->DistributedMeshInternal(mesh, processor_id, n_processors);

        std::string str(argv[1]);
        str.resize(str.length()-4);
        pmesh->setFilename(str);

        //Escreve partição na arquivo 
        pmesh->writeParallelMesh();
    }

    std::cout << "\nPMESH " << processor_id << "\n";
    std::cout << "  nodes: \n    ";
    std::vector<double> coord = pmesh->getCoord();
    for(int i = 0 ; i < coord.size() ; i++)
    {
        std::cout << coord[i] << " ";
        if((i+1)%3 == 0)
            std::cout << "\n    ";
    }
        
    std::cout << "\n  local to global: ";
    std::vector<unsigned int> ltg = pmesh->get_local_to_global();
    for(int i = 0 ; i < ltg.size() ; i++)
        std::cout << ltg[i] << " ";
    

    std::cout << "\n  elems: \n";
    unsigned int nelem = pmesh->get_n_elements();
    unsigned int* conn;
    unsigned int connsize;
    for(int i = 0 ; i < nelem ; i++)
    {
        conn = pmesh->getElementConn(i);
        connsize = pmesh->getElementConnSize(i);
        std::cout << "   elem " << i+1 << ": ";
        for(int j = 0 ; j < connsize ; j++)
            std::cout << conn[j] << " ";
        std::cout << "\n";
    }

    // get_n_nodes() retorna o numero de nos no pmesh
    //std::cout << "rank " << processor_id << " nnodes " << pmesh->get_n_nodes() << "  nelements " << pmesh->get_n_elements() << '\n';
    // Criar o Sistema de Equações
    std::vector<unsigned int> &gindices = pmesh->get_local_to_global();

    // for(int i = 0 ; i < gindices.size() ; i++)
    // {
    //     std::cout << "rank " << processor_id << "   " << i <<  " ->> " << gindices[i] << "\n";
    // }

    // Objetivo criar ym sistema de equações:
    // MATRIZ A
    // Vetores b,x

    Vec x;                                  // numero de nos totais
    VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, pmesh->get_n_nodes(), &x);
    
    // Intervalo dos indices globais em cada processo
    PetscInt rstart, rend;
    VecGetOwnershipRange(x, &rstart, &rend);

    std::cout << "processor ID " << processor_id << " Interval [" << rstart <<","<<rend <<"]\n" << std::flush;

    int total_nodes;
    if(processor_id == 0)
    {
        int nnodes = pmesh->get_n_nodes();
        int nnodes_otherrank;
        MPI_Send(&nnodes, 1, MPI_INT, 0, 0, PETSC_COMM_WORLD);
        MPI_Recv(&nnodes_otherrank, 1, MPI_INT, 0, 0, PETSC_COMM_WORLD, MPI_STATUS_IGNORE);

        total_nodes = nnodes + nnodes_otherrank;
    }
    else
    {
        int nnodes = pmesh->get_n_nodes();
        int nnodes_otherrank;
        MPI_Send(&nnodes, 1, MPI_INT, 1, 0, PETSC_COMM_WORLD);
        MPI_Recv(&nnodes_otherrank, 1, MPI_INT, 1, 0, PETSC_COMM_WORLD, MPI_STATUS_IGNORE);

        total_nodes = nnodes + nnodes_otherrank;
    }


    VecView(x, v_view);

    // Prencher o vetor:
    for(int i = 0; i < pmesh->get_n_elements(); i++)
    {
        unsigned int  csize = pmesh->getElementConnSize(i);
        unsigned int* conn = pmesh->getElementConn(i);
        for(int j = 0; j < csize; j++)
            VecSetValue(x,gindices[conn[j]], 1, ADD_VALUES);
    }

    VecAssemblyBegin(x);
    VecAssemblyEnd(x);

    VecView(x, v_view);


    //Mat A;
    // CSR Matriz Esparsa
    //MatCreateAIJ(PETSC_COMM_WORLD, PETSC_DECIDE,PETSC_DECIDE,pmesh->get_n_nodes(),pmesh->get_n_nodes(),10,NULL, 10, NULL, &A);

    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}