
#include <cstdlib>
#include "meshtools.h"

namespace MeshTools
{
    static int _processor_id;
    static int _n_processors;
    MPI_Comm  _mpi_comm;

void Init(int argc, char* argv[])
{
    _processor_id = 0;
    _n_processors = 1;
#ifdef USE_MPI
    MPI_Init(&argc, &argv);
    
    _mpi_comm = MPI_COMM_WORLD;
    MPI_Comm_size(_mpi_comm, &_n_processors);
    MPI_Comm_rank(_mpi_comm, &_processor_id);
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

int& processor_id()
{
    return _processor_id;
}
    
int& n_processors()
{
    return _n_processors;
}

MPI_Comm Comm()
{
    return _mpi_comm;
}

}
