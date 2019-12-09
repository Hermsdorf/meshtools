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
    ofstream times("times.txt", fstream::app);
    clock_t t;
    str.resize(str.length()-4);
  
    const char* out;
    str = str.append(".vtu");
    out = str.c_str();
    int n_part = atoi(argv[2]);
    times << out << "      n_partitions = " << n_part << endl;

    t = clock();
    mesh_t* mesh            = MeshGMSHReader(argv[1]);
    t = clock() - t;
    double time_GMSHReader = ((double)t)/CLOCKS_PER_SEC;

    mesh_partition_t *parts = MeshPartitioner(mesh, n_part);
 
    t = clock();
    MeshVTKWriterInternal(mesh, out, parts->nodal_part, parts->elem_part);
    t = clock() - t;
    double time_VTKWriter = ((double)t)/CLOCKS_PER_SEC;

    MeshPartitionDestroy(parts);
    
    times << "MeshGMSHReader = " << time_GMSHReader << endl;
    times << "MeshVTKWriterInternal = " << time_VTKWriter << "\n\n\n";

    delete mesh;
    return 0;
}


