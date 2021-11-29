#include <iostream>
#include <cmath>
#include <unistd.h>
#include <cstring>

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallel_mesh.h"

//#include "petsc.h"

using namespace std;

static void usage(const char *arg0)
{
    cerr << "Usage: " << arg0 << " <options>" << endl;
    cerr << "  -h                   : show help" << endl;
    cerr << "  -m <filename>        : where <filename> is the gmsh file name (gmsh ascii v.2.2)> " << endl;
    cerr << "  -c [color algorithm] : where [color algotihm] is the coloring algorithm. The options are: " << endl;
    cerr <<"      greedy  : greedy serial version (default)" << endl;
    cerr <<"      blocked : blocked serial version" << endl;
    cerr <<"      rokos   : openmp greedy version " << endl;
    cerr << "  -b <block size> : where <block size> is block size used in the the blocked version coloring algorithm." << endl;
    cerr << "  -r <reordering algorithm> : where [reordering algotihm] is the nodal renumering algorithm. The options are: " << endl;
    cerr <<"      rcm       : apply rcm (default) " << endl;
    cerr <<"      nd        : apply nested disection algorithm " << endl;
    cerr <<"      natural   : first touch algorithm" << endl;
    cerr <<"      none      : keep gmsh ordering " << endl;    
    cerr <<"  -w <vtk write_mode> : where [vtk type] is the way to write the mesh in vtk file. The options are: " << endl;
    cerr <<"      ascii             : write ascii files  " << endl;
    cerr <<"      binary            : write binary files " << endl;
    
}


int main(int argc, char* argv[])
{         

    int   opt;
    char* gmsh_filename    = 0; 
    bool  flg_catalyst     = false;
    bool  flg_gmsh         = false;
    bool  flg_reorder      = false;
    bool  flg_write        = false;
    char* catalyst_script  = 0;
    char* rorder_alg_name  = 0;
    char* color_alg_name   = 0;
    char* write_alg_name   = 0;
    char* block_size_str;

    reorder_t    reordering = RCM;
    color_mode_t color_alg  = COLOR_DEFAULT_BLOCK;
    write_t      writing    = BINARY;
    int block_size          = 4096;
    int n_processors = 1;
    int processor_id = 0;

    MeshTools::Init(argc,argv);

    // Obrigatorio ter ao menos 3 argumentos:
    // ./meshtools -m <filename>
    if(argc < 3)
    {
        if(MeshTools::processor_id==0) 
            usage(argv[0]);
        MeshTools::Finalize();
        return 0;
    }

    // Trata os argumentos que são passados por linha de comando
    while( (opt = getopt(argc, argv, "hm:c:r:v:b:w:")) !=  -1 ) {
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
                flg_reorder = true;
                if(strcmp(rorder_alg_name,"nd")==0){
                    reordering = METIS_ND;
                    cout << "Metis Nested-Dissection reordering algorithm selected.\n";
                }
                if(strcmp(rorder_alg_name,"natural")==0){
                    reordering = FF;
                    cout << "First Touch reordering algorithm selected.\n";
                }
                if(strcmp(rorder_alg_name,"none")==0){
                    flg_reorder = false;
                    cout << "Original Gmsh ordering selected.\n";
                }
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
            case 'w':
                flg_write = true;
                write_alg_name = optarg;
                if(strcmp(write_alg_name,"binary")==0)
                    writing = BINARY;
                if(strcmp(write_alg_name,"ascii")==0)
                    writing = ASCII;
                break;
            default:
                fprintf(stderr, "Opcao invalida ou faltando argumento: `%c'\n", optopt) ;
                usage(argv[0]);
                return -1 ;
        }
    }

    if(!flg_gmsh)
    {
        if(MeshTools::processor_id==0) 
            usage(argv[0]);
        MeshTools::Finalize();
        return 0;
    }

    Mesh             *mesh  = nullptr;
    ParallelMesh     *pmesh = nullptr;
    Mesh_partition_t *parts = new Mesh_partition_t();

    if(MeshTools::processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha. 
        mesh = new Mesh(gmsh_filename);

        // Aplica a reordenação nodal considerando o algoritmo
        // escolhido pelo usuário
        if(flg_reorder)
            mesh->MeshReordering(reordering);
    
        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if(MeshTools::n_processors > 1 ) {
            parts->MeshPartitionerInternal(mesh, MeshTools::n_processors);
        }
    }

    if(MeshTools::n_processors > 1 ) 
    {
        // Malha gerada pelo processo mestre é distribuida
        // para os demais processos. 
        pmesh = parts->DistributedMeshInternal(mesh, processor_id, n_processors);
        std::string str(gmsh_filename);
        str.resize(str.length()-4);
        pmesh->setFilename(str);

        // Aplica em cada partição a coloração
        pmesh->MeshColoring(color_alg, block_size);

        //FiniteElementKernels::run(*pmesh);

        //Escreve partição na arquivo 
        pmesh->writeParallelMesh();
    }
    else
    {
        // Em caso de execução em serial, aplica a coloração 
        // em toda a malha
        mesh->MeshColoring(color_alg, block_size);

        // Escreve a malha em arquivo.
        if(flg_write)
            mesh->MeshVTKWriting(writing);

        //FiniteElementKernels::run(*mesh);

    }

    // Desaloca as estruturas criadas.
    if(mesh)  delete mesh;
    if(parts) delete parts;
    if(pmesh) delete pmesh;

    MeshTools::Finalize();

    return 0;

}



