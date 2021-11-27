
#include <cstdlib>
#include "meshtools.h"

namespace MeshTools
{
void Init(int argc, char* argv[])
{
    processor_id = 0;
    n_processors = 1;
#ifdef USE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &processor_id);
#endif

}

void Finalize()
{
#ifdef USE_MPI
    MPI_Finalize();
#endif  
}

void Exit()
{
#ifdef USE_MPI
    MPI_Finalize() ;
#endif  
    std::exit(-1);
}

}
