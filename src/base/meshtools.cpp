
#include <cstdlib>
#include <memory>

#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_partition.h"
namespace MeshTools
{
    static int _processor_id;
    static int _n_processors;
    MPI_Comm  _mpi_comm;

void Init(int argc, char* argv[])
{
    _processor_id = 0;
    _n_processors = 1;
#if defined(PETSC_ENABLE)
    PetscInitialize(&argc,&argv,0,0);
    _mpi_comm = PETSC_COMM_WORLD;
    MPI_Comm_size(_mpi_comm, &_n_processors);
    MPI_Comm_rank(_mpi_comm, &_processor_id);
#elif defined(USE_MPI)
    MPI_Init(&argc, &argv);
    _mpi_comm = MPI_COMM_WORLD;
    MPI_Comm_size(_mpi_comm, &_n_processors);
    MPI_Comm_rank(_mpi_comm, &_processor_id);
#endif
    if(_processor_id == 0) {
        std::cout<<"\nMeshTools Initialization\n"
             <<"  Number of Processors: " << _n_processors << std::endl;
    }

}

void Finalize()
{
#if defined(PETSC_ENABLE)
    PetscFinalize();
#elif defined(USE_MPI)
    MPI_Finalize();
#endif  
}

void Exit()
{
#if defined(PETSC_ENABLE)
    PetscFinalize();
#elif defined(USE_MPI)
    MPI_Finalize();
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

// ParallelMesh* ReadMesh(const std::string& filename)
// {
//     Mesh* mesh = nullptr;
//     MeshPartition* parts = new MeshPartition();
//     if (_processor_id == 0)
//     {
//         // Rodando serial ou em paralelo o processo mestre
//         // irá ler a malha.
//         mesh = new Mesh(filename);

//         // Se houver mais um processo, o processo mestre irá
//         // particionar a malha
//         if (_n_processors > 1)
//         {
//             parts->ApplyPartitioner(mesh, _n_processors);
//         }
//     }

//     ParallelMesh* pmesh = parts->DistributedMesh(mesh);
//     if(mesh) delete mesh;
//     if(parts) delete parts;
//     return pmesh;

// }

void Printf(const char format[],...)
{
    if (_processor_id == 0) {
        va_list Argp;
        va_start(Argp, format);
        fprintf(stdout, format, Argp);
        va_end(Argp);
     }
}

}
