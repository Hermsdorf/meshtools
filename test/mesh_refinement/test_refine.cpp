#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_refinement.h"
#include "gmsh_io.h"
#include <time.h>

int main(int argc, char *argv[])
{

    clock_t tInicio, tFim, tDecorrido;
  
    MeshTools::Init(argc, argv);

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/esfera_box.msh");

    GmshIO gmsh;
    mesh->write_vtk("esfera_box");

    MeshRefinement refiner(mesh);
    tInicio = clock();
    refiner.refine();
    tFim = clock();
    tDecorrido = ((tFim - tInicio) / (CLOCKS_PER_SEC / 1000));
    std::cout << "Tempo de execução: " << tDecorrido << " ms" << std::endl;


    mesh->write_vtk("esfera_box_refined");
    //gmsh.write("esfera_box_refined.msh", *mesh);

    MeshTools::Finalize();
    return 0;
}
