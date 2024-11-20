#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_refinement.h"
#include "gmsh_io.h"
#include <time.h>

int main(int argc, char *argv[])
{

    clock_t tInicio, tFim, tDecorrido;
  
    MeshTools::Init(argc, argv);

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/tetraedro_simples.msh");

    GmshIO gmsh;
    mesh->write_vtk("tetraedro_simples");

    MeshRefinement refiner(mesh);
    tInicio = clock();
    refiner.refine();
    tFim = clock();
    tDecorrido = ((tFim - tInicio) / (CLOCKS_PER_SEC / 1000));
    std::cout << "Tempo de execução: " << tDecorrido << " ms" << std::endl;


    mesh->write_vtk("tetraedro_simples_refined_corrigido");
    gmsh.write("tetraedro_simples_refined_corrigido.msh", *mesh);

    MeshTools::Finalize();
    return 0;
}
