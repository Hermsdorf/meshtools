
#include <iostream>

#include "mesh.h"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "ERROR: WRONG EXECUTION" << endl;
	cout << "./meshtools filename n_partitions" << endl;

	return 0;
    }

    int n_part = atoi(argv[2]);
    mesh_t* mesh            = MeshGMSHReader(argv[1]);
    mesh_partition_t *parts = MeshPartitioner(mesh, n_part);

    MeshVTKWriterInternal(mesh, "test1.vtu", parts->nodal_part, parts->elem_part);

    MeshPartitionDestroy(parts);
    delete mesh;
    return 0;
}


