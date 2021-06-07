#include <iostream>
#include <cmath>
#include <unistd.h>
#include <cstring>


#include "meshtools_config.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallelmesh.h"


#include "finite_element_kernels.h"

#if USE_MPI
#include "mpi.h"
#endif

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifdef USE_CATALYST
#include "FEAdaptor.h"
#endif

using namespace std;

static void usage(const char *arg0)
{
    cerr << "Usage: " << arg0 << " <options>" << endl;
    cerr << "\t -h                   : show help" << endl;
    cerr << "\t -m <filename>        : where <filename> is the gmsh file name (gmsh ascii v.2.2)> " << endl;
    cerr << "\t -c [color algorithm] : where [color algotihm] is the coloring algorithm. The options are: " << endl;
    cerr <<"\t\t  greedy  : greedy serial version (default)" << endl;
    cerr <<"\t\t  blocked : blocked serial version" << endl;
    cerr <<"\t\t  rokos   : openmp greedy version " << endl;
    cerr << "\t -b <block size> : where <block size> is block size used in the the blocked version coloring algorithm." << endl;
    cerr << "\t -r <reordering algorithm> : where [reordering algotihm] is the nodal renumering algorithm. The options are: " << endl;
    cerr <<"\t\t  rcm       : apply rcm (default) " << endl;
    cerr <<"\t\t  nd        : apply nested disection algorithm " << endl;
    cerr <<"\t\t  first-fit : first touch algorithm" << endl;
    exit(-1);
}


int main(int argc, char* argv[])
{         

    int   opt;
    char* gmsh_filename    = 0; 
    bool  flg_catalyst     = false;
    bool  flg_gmsh         = false;
    char* catalyst_script  = 0;
    char* rorder_alg_name  = 0;
    char* color_alg_name   = 0;
    char* block_size_str;

    reorder_t    reordering = RCM;
    color_mode_t color_alg  = COLOR_DEFAULT_BLOCK;
    int block_size          = 4096;

#ifdef USE_MPI
    MPI_Init(argc, argv);
#endif

    // Obrigatorio ter ao menos 3 argumentos:
    // ./meshtools -m <filename>
    if(argc < 3)
    {
        usage(argv[0]);
    }

    // Trata os argumentos que são passados por linha de comando
    while( (opt = getopt(argc, argv, "hm:c:r:c:b:")) !=  -1 ) {
        switch ( opt ) {
            case 'h': /* help */
                usage(argv[0]) ;
                break ;
            case 'm': /* flag -m */
                gmsh_filename = optarg ;
                flg_gmsh      = true;
                break ;
            case 'v':
                catalyst_script = optarg;
                flg_catalyst    = true;
                break;
            case 'r':
                rorder_alg_name = optarg;
                if(strcmp(rorder_alg_name,"nd")==0)
                    reordering = METIS_ND;
                if(strcmp(rorder_alg_name,"first-fit")==0)
                    reordering = FF;
                break;
            case 'c':
                color_alg_name = optarg;
                if(strcmp(color_alg_name,"greedy")==0)
                    color_alg = COLOR_DEFAULT;
                if(strcmp(color_alg_name,"blocked")==0)
                    color_alg = COLOR_DEFAULT_BLOCK;
                 if(strcmp(color_alg_name,"rokos")==0)
                     color_alg = COLOR_ROKOS;
                break;
            case 'b':
                block_size_str = optarg;
                block_size = atoi(block_size_str);
                break;
            default:
                fprintf(stderr, "Opcao invalida ou faltando argumento: `%c'\n", optopt) ;
                usage(argv[0]);
                return -1 ;
        }
    }

    if(!flg_gmsh)
    {
        usage(argv[0]);
    }

    int n_processors = 1;
    int processor_id = 0;

#ifdef USE_MPI
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &processor_id);
#endif

#ifdef USE_CATALYST
    CatalystInitialize(1, catalyst_script);
#endif

    Mesh             *mesh  = nullptr;
    ParallelMesh     *pmesh = nullptr;
    Mesh_partition_t *parts = new Mesh_partition_t();

    if(processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha. 
        mesh = new Mesh(gmsh_filename);

        // Aplica a reordenação nodal considerando o algoritmo
        // escolhido pelo usuário
        mesh->MeshReordering(reordering);
    
        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if(n_processors > 1 ) {
            parts->MeshPartitionerInternal(mesh, n_processors);
        }
    }

    if(n_processors > 1 ) 
    {
        // Malha gerada pelo processo mestre é distribuida
        // para os demais processos. 
        pmesh = parts->DistributedMeshInternal(mesh, processor_id, n_processors);

        std::string str(gmsh_filename);
        str.resize(str.length()-4);
        pmesh->setFilename(str);

        // Aplica em cada partição a coloração
        pmesh->MeshColoring(color_alg, block_size);

        FiniteElementKernels::run(*pmesh);

        //Escreve partição na arquivo 
        pmesh->writeParallelMesh();
    }
    else
    {
        // Em caso de execução em serial, aplica a coloração 
        // em toda a malha
        mesh->MeshColoring(color_alg, block_size);

        // Escreve a malha em arquivo.
        mesh->MeshVTKWriterInternalBinAppended(0);

        FiniteElementKernels::run(*mesh);

    }

    // Desaloca as estruturas criadas.
    if(mesh)  delete mesh;
    if(parts) delete parts;
    if(pmesh) delete pmesh;

#ifdef USE_CATALYST
    CatalystFinalize();
#endif

#ifdef USE_MPI
    MPI_Finalize();
#endif

    return 0;

}




