#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

#include "mesh.h"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "ERROR: WRONG EXECUTION" << endl;
	    cout << "./meshtools filename n_partitions" << endl;

	    return 0;
    }
    string str = argv[1];

    str.resize(str.length()-4);
  
    const char* out;
    str = str.append(".vtu");
    out = str.c_str();
    int n_part = atoi(argv[2]);

    mesh_t* mesh            = MeshGmshReader(argv[1]);
      
    MeshReordering(mesh);

    mesh_partition_t *parts = MeshPartitioner(mesh, n_part);
 
    MeshVTKWriterInternal(mesh, out, parts->nodal_part, parts->elem_part);

    MeshPartitionDestroy(parts);
    
    delete mesh;
    return 0;
}


