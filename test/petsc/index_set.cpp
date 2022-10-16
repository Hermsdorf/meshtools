static char help[] = "Creating a general index set.\n\n";

/*
    Include petscis.h so we can use PETSc IS objects. Note that this automatically
  includes petscsys.h.
*/
#include <petscis.h>
#include <petscviewer.h>

int main(int argc,char **argv)
{
  PetscInt       *indices,n;
  const PetscInt *nindices;
  PetscMPIInt    rank;
  IS             is;

  PetscInitialize(&argc,&argv,(char*)0,help);
  MPI_Comm_rank(PETSC_COMM_WORLD,&rank);
  /*
     Create an index set with 5 entries. Each processor creates
   its own index set with its own list of integers.
  */
  PetscMalloc1(5,&indices);
  indices[0] = rank + 1;
  indices[1] = rank + 2;
  indices[2] = rank + 3;
  indices[3] = rank + 4;
  indices[4] = rank + 5;
  ISCreateGeneral(PETSC_COMM_SELF,5,indices,PETSC_COPY_VALUES,&is);
  /*
     Note that ISCreateGeneral() has made a copy of the indices
     so we may (and generally should) free indices[]
  */
  PetscFree(indices);

  /*
     Print the index set to stdout
  */
  ISView(is,PETSC_VIEWER_STDOUT_SELF);

  /*
     Get the number of indices in the set
  */
  ISGetLocalSize(is,&n);

  /*
     Get the indices in the index set
  */
  ISGetIndices(is,&nindices);
  /*
     Now any code that needs access to the list of integers
   has access to it here through indices[].
   */
  PetscPrintf(PETSC_COMM_SELF,"[%d] First index %" PetscInt_FMT "\n",rank,nindices[0]);

  /*
     Once we no longer need access to the indices they should
     returned to the system
  */
  ISRestoreIndices(is,&nindices);

  /*
     One should destroy any PETSc object once one is completely
    done with it.
  */
  ISDestroy(&is);
  PetscFinalize();
  return 0;
}

/*TEST

   test:

TEST*/
