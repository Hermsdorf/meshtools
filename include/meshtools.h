#ifndef MESHTOOLS_H
#define MESHTOOLS_H

#define DEBUG_


#include "GetPot.hpp"
#include "meshtools_config.h"

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

class ParallelMesh;
namespace MeshTools 
{

    void Init(int argc, char* argv[]);    
    void Finalize();
    void Printf(const char format[],...);
    ParallelMesh* ReadMesh(const std::string& filename);

    int& processor_id();
    int& n_processors();

    MPI_Comm Comm();    

    void Exit();

}



#endif /* MESHTOOLS_H */
