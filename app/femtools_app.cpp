#include "petsc.h"
#include "meshtools.h"
#include <iostream>

using namespace std;

static char help[] = "Empty Problem\n\n";

int main(int argc, char* argv[])
{
    PetscErrorCode ierr;
    Vec v;
    PetscViewer v_view;
    IS is; // index set
    ierr = PetscInitialize(&argc,&argv,nullptr,help); CHKERRQ(ierr);

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

    // after it is necessary to apply 
    ISLocalToGlobalMappingApply(mapping, m, input, output);
    PetscIntView(m, output, PETSC_VIEWER_STDOUT_WORLD);
    
    ierr = VecDestroy(&v);
    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}