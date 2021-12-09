#include "petsc.h"
#include "meshtools.h"
static char help[] = "Empty Proglem\n\n";

int main(int argc, char* argv[])
{
    PetscErrorCode ierr;
    Vec x;
    ierr = PetscInitialize(&argc,&argv,nullptr,help); CHKERRQ(ierr);
    ierr = VecCreate(PETSC_COMM_WORLD,&x); CHKERRQ(ierr);



    ierr = VecDestroy(&x);
    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}