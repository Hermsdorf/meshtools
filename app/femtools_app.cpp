#include "petsc.h"

#include "meshtools.h"
<<<<<<< HEAD
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallel_mesh.h"
=======
#include <iostream>

using namespace std;
>>>>>>> 55040cdb09f98f5e548219cba67afb505e6952e2

static char help[] = "Empty Problem\n\n";

int main(int argc, char* argv[])
{
    PetscErrorCode ierr;
    Vec v;
    PetscViewer v_view;
    IS is; // index set
    ierr = PetscInitialize(&argc,&argv,nullptr,help); CHKERRQ(ierr);
<<<<<<< HEAD

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
=======
>>>>>>> 55040cdb09f98f5e548219cba67afb505e6952e2

    int size, rank;
    PetscInt i = 3;
    PetscInt vector_size = 8;
    PetscReal value = 3.14;

    MPI_Comm_size(PETSC_COMM_WORLD,&size);
    MPI_Comm_rank(PETSC_COMM_WORLD,&rank);

    // testing some vec methods
    if(rank == 0)
    {
        ierr = VecCreateMPI(PETSC_COMM_WORLD, 3, vector_size, &v); CHKERRQ(ierr); // creating a vector with local size 3 and global size 8
    }
    else{
        if(rank == 1){
            ierr = VecCreateMPI(PETSC_COMM_WORLD, 3, vector_size, &v); CHKERRQ(ierr); // creating a vector with local size 3 and global size 8
        }
        else{
            ierr = VecCreateMPI(PETSC_COMM_WORLD, 2, vector_size, &v); CHKERRQ(ierr); // creating a vector with local size 3 and global size 8
        }
    }
    VecSetValue(v, 5, value, INSERT_VALUES);
    
    VecAssemblyBegin(v); // necessario para atualizar o array entre os processos (?)
    VecAssemblyEnd(v);


    //VecView(v, v_view);
    
    // testing index set
    // indices has each position of the array to every processor
    // indices = [0, 3, 9, 12] -> processor 1 has elements on input 0th to 2th element,
    //                            processor 2 has elements 3th to 8th element,
    //                            processor 3 has elements 9th to 12th element

    // to use IS it is necessary pass the input and the indices to mapping all elements
    PetscInt indices[] = {0, 3, 9, 12}, n = 5;
    PetscInt input[] = {10, 20}, *output, m = 5;
    
    ISCreateGeneral(PETSC_COMM_SELF,n,indices,PETSC_COPY_VALUES,&is);

    ISView(is,PETSC_VIEWER_STDOUT_SELF);

    ISLocalToGlobalMapping mapping;

    // at the beginning it is necessary to create and set options of the mapping
    ISLocalToGlobalMappingCreate(PETSC_COMM_WORLD,1,n,indices,PETSC_COPY_VALUES,&mapping);
    ISLocalToGlobalMappingSetFromOptions(mapping);

<<<<<<< HEAD
=======
    // after it is necessary to apply 
    ISLocalToGlobalMappingApply(mapping, m, input, output);
    PetscIntView(m, output, PETSC_VIEWER_STDOUT_WORLD);
    
    ierr = VecDestroy(&v);
>>>>>>> 55040cdb09f98f5e548219cba67afb505e6952e2
    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}