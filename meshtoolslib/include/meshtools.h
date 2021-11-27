#ifndef C1D6AFE9_DE06_4F81_9D1F_8A4C41B5D464
#define C1D6AFE9_DE06_4F81_9D1F_8A4C41B5D464

#include "meshtools_config.h"

#if USE_MPI
#include "mpi.h"
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef USE_CATALYST
    #include "FEAdaptor.h"
#endif

namespace MeshTools 
{
    static int processor_id;
    static int n_processors;

    void Init(int argc, char* argv[]);

    void Finalize();

    void Exit();
}



#endif /* C1D6AFE9_DE06_4F81_9D1F_8A4C41B5D464 */
