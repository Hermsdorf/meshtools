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

/*
    cout << endl << "OFFSET: ";
    for(int i = 0 ; i < mesh->offset.size() ; i++)
        cout << mesh->offset[i] << " ";
    cout << endl << "CONN ANTES: ";
    for(int i = 0 ; i < mesh->conn.size() ; i++)
        cout << mesh->conn[i] << " ";
    vector<double> newCoord;
*/
    MeshReordering(mesh, RCM);
    
/*
    cout << endl << "CONN DEPOIS: ";
    for(int i = 0 ; i < mesh->conn.size() ; i++)
        cout << mesh->conn[i] << " ";

*/

    mesh_partition_t *parts = MeshPartitionerInternal(mesh, n_part);
 
    MeshVTKWriterInternal(mesh, out, parts->nodal_part, parts->elem_part);

    MeshPartitionDestroy(parts);
    
    delete mesh;
    return 0;
}


