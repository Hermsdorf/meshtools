// Arquivo: mesh.h
// Implememta rotinas para manipulação de malhas de elementos finitos
// Autor: Jose J. Camata
// Criado em: 

#ifndef MESH_H__
#define MESH_H__

#include <vector>
#include <string>
#include <map>
using namespace std;


typedef std::pair<int, string> physical_data_t;

typedef struct {
    vector<double> coord;                    // Coordenadas nodais
    vector<int>    conn;                     // conectividade dos elementos
    vector<int>    offset;                   // 
    vector<int>    type;                     // mapeia a localização de cada elemento no vetor conn
    vector<int>    physical_tag;             //  
    map<int, physical_data_t>  physical_map; // 
    int n_face_elements;                     // Numeros de elementos na superficies
    int n_elements;                          // Numero de elementos internos
    int n_nodes;                             // Numero de nós.

} mesh_t;


typedef struct {
    int n_partitions;
    int *nodal_part;
    int *elem_part;
} mesh_partition_t;


// Leitura do arquivo no formato GMSH. Retorna um ponteiro 
// para a estrutura mesh.
mesh_t* MeshReadGMSH(const char* filename);

void    MeshVTKWriter(mesh_t* mesh, const char* filename);

void    MeshVTKWriterInternal(mesh_t* mesh, const char* filename, int *npart, int *epart);

mesh_t* MeshGMSHReader(const char* filename);

mesh_partition_t* MeshPartitioner(mesh_t* mesh, int nparts);

void MeshPartitionDestroy(mesh_partition_t* mp);

//void  MeshWrite(mesh* m, const char* outfile);

#endif


