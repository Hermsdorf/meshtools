#ifndef MESHTOOLS_H
#define MESHTOOLS_H


#include "GetPot.hpp"
#include "meshtools_config.h"
#include  "numeric_vector.h"

#ifdef MPI_ENABLE
#include "mpi.h"
#endif

#ifdef PETSC_ENABLE
#include "petsc.h"
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef HDF5_ENABLE
#include <hdf5.h>
#endif

#ifdef USE_CATALYST
#include "FEAdaptor.h"
#endif

#include <memory>

class ParallelMesh;
namespace MeshTools 
{

    void Init(int argc, char* argv[]);    
    void Finalize();
    void Printf(const char format[],...);
    void PrintDebug(const char format[],...);
    FILE* DebugOutput();
    std::unique_ptr<ParallelMesh> read(const std::string filename);

    int& processor_id();
    int& n_processors();

    MPI_Comm Comm();    

    void Exit();

}



#endif /* MESHTOOLS_H */
