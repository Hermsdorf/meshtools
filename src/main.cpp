#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

 /*   
    ofstream outData("matrix_data.txt");

    vector<int> xadjAntes;   // variaveis para armazenar as estruturas do grafo
    vector<int> adjncyAntes; // antes e após a reordenação nodal
    vector<int> xadjDepois;    // para testes de desempenho
    vector<int> adjncyDepois;
*/

    mesh_t* mesh            = MeshGmshReader(argv[1]);


 //   teste(mesh, (idx_t**)&xadjAntes, (idx_t**)&adjncyAntes);
    MeshReordering(mesh);
 //   teste(mesh, (idx_t**)&xadjDepois, (idx_t**)&adjncyDepois);

/* 
    for(int i = 0 ; i < xadjAntes.size() ; i++)
        cout << xadjAntes[i] << ",";
*/

    mesh_partition_t *parts = MeshPartitionerInternal(mesh, n_part);
 
    MeshVTKWriterInternal(mesh, out, parts->nodal_part, parts->elem_part);

    MeshPartitionDestroy(parts);
    
    delete mesh;
    return 0;
}


