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

    idx_t* xadjAntes;   // variaveis para armazenar as estruturas do grafo
    idx_t* adjncyAntes; // antes e após a reordenação nodal
    idx_t* xadjDepois;    // para testes de desempenho
    idx_t* adjncyDepois;

    mesh_t* mesh            = MeshGmshReader(argv[1]);

    teste(mesh, &xadjAntes, &adjncyAntes);
    MeshReordering(mesh);
    teste(mesh, &xadjDepois, &adjncyDepois);

    mesh_partition_t *parts = MeshPartitionerInternal(mesh, n_part);
 
    MeshVTKWriterInternal(mesh, out, parts->nodal_part, parts->elem_part);

    MeshPartitionDestroy(parts);
    
    delete mesh;
    return 0;
}


