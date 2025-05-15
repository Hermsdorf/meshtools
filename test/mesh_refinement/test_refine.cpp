#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_refinement.h"
#include "gmsh_io.h"
#include <time.h>

int main(int argc, char *argv[])
{

    string meshFiliename = argv[1];
    string n_times = argv[2];
    clock_t tInicio, tFim, tDecorrido;
    
    MeshTools::Init(argc, argv);
    //std::cout<<"Numero de refinamentos: "<<stoi(n_times)<<std::endl;

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/" + meshFiliename);

    GmshIO gmsh;
    mesh->write_vtk(meshFiliename);

    MeshRefinement refiner(mesh);
    tInicio = clock();
    for(int i = 0; i < stoi(n_times); i++)
    {
        std::cout << "Processador " << MeshTools::processor_id() << ". " << std::endl;
        cout << "Refinamento "<< i+1 <<": " << std::endl;
        refiner.refine();
    }
    tFim = clock();
    tDecorrido = ((tFim - tInicio) / (CLOCKS_PER_SEC / 1000));
    std::cout << std::endl;
    std::cout << "Processador " << MeshTools::processor_id() << ". " << std::endl;
    std::cout << "Tempo de execução: " << tDecorrido << " ms" << std::endl;


    meshFiliename += "_refined";
    mesh->write_vtk(meshFiliename );
    //gmsh.write("esfera_box_refined.msh", *mesh);

    MeshTools::Finalize();
    return 0;
}
