#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "numeric_vector.h"

int normal(int argc, char *argv[])
{
    MeshPartition *parts = new MeshPartition();

    Mesh *mesh;          // serial mesh
    ParallelMesh *pmesh; // parallel mesh
    int processor_id, n_processors;

    MeshTools::Init(argc, argv);
    processor_id = MeshTools::processor_id();
    n_processors = MeshTools::n_processors();

    if (processor_id == 0)
    {
        // Rodando serial ou em paralelo o processo mestre
        // irá ler a malha.
        mesh = new Mesh("../msh/dez.msh");

        // Se houver mais um processo, o processo mestre irá
        // particionar a malha
        if (n_processors > 1)
        {
            parts->ApplyPartitioner(mesh, n_processors);
        }
    }

    pmesh = parts->DistributedMesh(mesh);

    for (int iel = 0; iel < pmesh->get_n_face_elements(); iel++)
    {
        SurfaceElement elem;
        pmesh->getSurfaceElement(iel,elem);

        RealVector normal = elem.calculate_normal();
        cout << "Surface element " << iel << " normal: " << normal << endl;
    }

    if (MeshTools::processor_id() == 0)
        delete mesh;
    delete pmesh;
    delete parts;

    return 0;
}

int main(int argc, char *argv[])
{
    MeshTools::Init(argc, argv);
    normal(argc, argv);
    MeshTools::Finalize();

    return 0;
}
