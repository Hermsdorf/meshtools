
#include <cstdlib>
#include <memory>

#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_partition.h"
#include "mesh_reordering.h"
namespace MeshTools
{
    static int _processor_id;
    static int _n_processors;
    static FILE* _output;
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

#ifdef DEBUG_OUTPUT
    char _output_filename[255];
    snprintf(_output_filename, 255, "output_%d.txt", _processor_id);
    _output = fopen(_output_filename, "w");
#else
    _output = stdout;
#endif

    if(_processor_id == 0) {
        std::cout<<"\nMeshTools Initialization\n"
             <<"  Number of Processors: " << _n_processors << std::endl;
    }

}

void Finalize()
{
#ifdef DEBUG_OUTPUT
    fclose(_output); 
#endif
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

std::unique_ptr<ParallelMesh> read(const std::string filename)
{
    
    std::unique_ptr<MeshPartition> parts(new MeshPartition());
    std::unique_ptr<Mesh>    mesh;
    if (_processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha.
        mesh = std::make_unique<Mesh>();

        mesh->read(filename);

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (_n_processors > 1)
        {
            parts->apply_metis_partition(mesh, _n_processors);
        }
    }

    std::unique_ptr<ParallelMesh> pmesh = parts->distributed_mesh(mesh);
    return pmesh;
}


void Printf(const char format[],...)
{
    if (_processor_id == 0) {
        va_list Argp;
        va_start(Argp, format);
        fprintf(stdout, format, Argp);
        va_end(Argp);
     }
}

void PrintDebug(const char format[],...)
{
#ifdef NDEBUG
    va_list Argp;
    va_start(Argp, format);
    fprintf(stdout, "Processor %d: ", _processor_id);
    fprintf(stdout, format, Argp);
    va_end(Argp);
#endif
}

}
