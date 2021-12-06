#include "petsc.h"
#include "meshtools.h"
static char help[] = "Empty Proglem\n\n";

int main(int argc, char* argv[])
{
    PetscErrorCode ierr;
    ierr = PetscInitialize(&argc,&argv,nullptr,help); CHKERRQ(ierr);


    
    ierr = PetscFinalize();
    return 0;
}