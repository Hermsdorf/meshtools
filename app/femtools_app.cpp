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
    Vec x;
    ierr = PetscInitialize(&argc,&argv,nullptr,help); CHKERRQ(ierr);

    Mesh             *mesh  = nullptr;
    ParallelMesh     *pmesh = nullptr;
    Mesh_partition_t *parts = new Mesh_partition_t();
    
    int processor_id = 0;
    int n_processors = 0;

    MPI_Comm_rank(PETSC_COMM_WORLD,&processor_id );
    MPI_Comm_size(PETSC_COMM_WORLD,&n_processors );

    if(processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha. 
        mesh = new Mesh("test2d.msh");

        // Aplica a reordenação nodal considerando o algoritmo
        // escolhido pelo usuário
    
        mesh->MeshReordering(RCM);
    
        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if(n_processors > 1 ) {
            parts->MeshPartitionerInternal(mesh,n_processors);
        }
    }

    if(MeshTools::n_processors > 1 ) 
    {
        // Malha gerada pelo processo mestre é distribuida
        // para os demais processos. 
        pmesh = parts->DistributedMeshInternal(mesh, processor_id, n_processors);


    }
    // Criar o Sistema de Equações
    //
    auto &gindices = pmesh->get_local_to_global();

    // Objetivo criar ym sistema de equações:
    // MATRIZ A
    // Vetores b,x

    // Vec x;                                  // numero de nos totais
    VecCreateMPI(PETSC_COMM_WORLD,PETSC_DECIDE,pmesh->get_n_nodes(), &x);
    
    // Intervalo dos indices globais em cada processo
    int rstart, rend;
    VecGetOwnershipRange(x,&rstart,&rend);

    std::cout << "processor ID " << processor_id << "Interval [" << rstart <<","<<rend <<"]\n" << std::flush;

    // Prencher o vetor:
    for(int i = 0; i < pmesh->get_n_elements(); i++)
    {
        unsigned int  csize = pmesh->getElementConnSize(i);
        unsigned int* conn = pmesh->getElementConn(i);
        for(int j = 0; j < csize; j++)
            VecSetValue(x,gindices[conn[j]], 1.0, ADD_VALUES);
    }

    VecAssemblyBegin(x);
    VecAssemblyEnd(x);

    //VecView(x);


    Mat A;
    // CSR Matriz Esparsa
    //MatCreateAIJ(PETSC_COMM_WORLD, PETSC_DECIDE,PETSC_DECIDE,pmesh->get_n_nodes(),pmesh->get_n_nodes(),10,NULL, 10, NULL, &A);



    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}