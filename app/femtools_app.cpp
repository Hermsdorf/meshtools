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


    VecView(v, v_view);
    
    ISCreateGeneral(PETSC_COMM_WORLD, vector_size, v, PETSC_COPY_VALUES, &is);

    /*
          Print the index set to stdout
    */
    ISView(is,PETSC_VIEWER_STDOUT_SELF);


    ierr = VecDestroy(&v);
    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}