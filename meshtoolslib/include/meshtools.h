#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#include "meshtools_config.h"

#if USE_MPI
#include "mpi.h"
#endif

#ifdef PETSC_ENABLE
#include "petsc.h"
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef USE_CATALYST
    #include "FEAdaptor.h"
#endif

namespace MeshTools 
{

    void Init(int argc, char* argv[]);    
    void Finalize();

    int& processor_id();
    int& n_processors();

    MPI_Comm Comm();    

    void Exit();

}



#endif /* MESHTOOLS_H */
