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

#include "metis.h"


typedef std::pair<int, string> physical_data_t;

typedef enum {METIS_ND=0, RCM, FF} reorder_t;

typedef struct {
    vector<double> coord;                    // Coordenadas nodais.
    vector<int>    conn;                     // Conectividade dos elementos.
    vector<int>    offset;                   // Mapeia a localização de cada elemento no vetor conn.
    vector<unsigned short>    type;          // Vetor indicando o tipo de cada elemento.
    vector<int>    physical_tag;             // Vetor indicando o physical tag de cada elemento.
    int* mesh_coloring_internal;             // Vetor indicando as cores dos elementos.
    int n_internal_colors;                   // Número total de cores dos elementos internos da malha.
    map<int, physical_data_t>  physical_map; // 
    int n_face_elements;                     // Numeros de elementos de superficie.
    int n_elements;                          // Numero de elementos internos.
    int n_nodes;                             // Numero de nós.
    int dim;                                 // Dimensão da malha.
} mesh_t;


typedef struct {
    int n_partitions;  // Número de partições.
    int *nodal_part;   // Informações nodais da partição.
    int *elem_part;    // Informações elementares da partição.
} mesh_partition_t;


mesh_t* MeshCreate(); 
/**
 * * OBJETIVO: 
 *    Leitura do arquivo no formato GMSH. Retorna um ponteiro para a estrutura mesh.
 * 
 * * RETORNO:
 *    Um ponteiro do tipo da estrutura mesh_t com as informações necessárias da malha.
*/

void MeshGmshReader(mesh_t* mesh, const char* filename);
/**
 * * OBJETIVO:
 *     Ler o arquivo no formato msh e preencher as informações da estrutura mesh_t.
 * 
 * * PARAMETROS:
 * @param mesh Malha a ser preenchida na função.
 * @param filename Nome do arquivo de entrada extensão msh.
*/

int* GetElementConn(mesh_t* mesh, int element_num); 
/** 
 * * OBJETIVO:
 *     Indicar a posição inicial das conectvidades do elemento interno com índice element_num
 *     no vetor de conectividades da malha.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para as operações.
 * @param element_num Índice do elemento interno calculado, varia de 0 a n_elements - 1.
 * 
 * * RETORNO:
 *     Um ponteiro do tipo inteiro indicando, no vetor de conectividades, o início das conectividades
 *     do elemento com índice element_num.
*/
int* GetElementOffset(mesh_t* mesh, int element_num); 
/** 
 * * OBJETIVO:
 *     Indicar a posição do offset do elemento interno com índice element_num no vetor
 *     de offset da malha.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para as operações.
 * @param element_num Índice do elemento interno calculado, varia de 0 a n_elements - 1.
 * 
 * * RETORNO:
 *     Um ponteiro do tipo inteiro indicando, no vetor offset, a posição do elemento com índice element_num.
*/

int* GetSurfaceElementConn(mesh_t* mesh, int element_num); 
/** 
 * * OBJETIVO:
 *     Indicar a posição inicial das conectvidades do elemento de superfície com índice
 *     element_num no vetor de conectividades da malha.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para as operações.
 * @param element_num Índice do elemento de superfície cálculado, varia de 0 a n_face_elements - 1.
 * 
 * * RETORNO:
 *     Um ponteiro do tipo inteiro indicando, no vetor de conectividades, o início das conectividades
 *     do elemento de superfície com índice element_num.
*/

int* GetSurfaceElementOffset(mesh_t* mesh, int element_num); 
/** 
 * * OBJETIVO:
 *     Indicar a posição do offset do elemento de superfície com índice element_num no vetor de offset da malha.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para as operações.
 * @param element_num Índice do elemento de superfície calculado, varia de 0 a n_face_elements - 1.
 * 
 * * RETORNO:
 *     Um ponteiro do tipo inteiro indicando, no vetor offset, a posição do elemento de superfície com índice element_num.
*/

int GetElementConnSize(mesh_t* mesh, int element_num); 
/**
 * * OBJETIVO:
 *     Indicar o número de conectividades presentes no elemento interno com índice element_num.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para as operações.
 * @param element_num Índice do elemento interno calculado, varia de 0 a n_elements - 1.
 * 
 * * RETORNO:
 *     Uma variável do tipo inteira com a quantidade de conectividades do elemento.
*/
int GetSurfaceElementConnSize(mesh_t* mesh, int element_num); 
/**
 * * OBJETIVO:
 *     Indicar o número de conectividades presentes no elemento de superfície com índice element_num.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para as operações.
 * @param element_num Índice do elemento interno calculado, varia de 0 a n_face_elements - 1.
 * 
 * * RETORNO:
 *     Uma variável do tipo inteira com a quantidade de conectividades do elemento.
*/

void MeshVTKWriter(mesh_t* mesh, const char* filename, int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure);
/**
 * * OBJETIVO:
 *     Escrita da malha completa, com elementos internos e de superfície, no formato VTK.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a escrita.
 * @param filename Nome do arquivo de entrada no formato msh.
 * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView.
 *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
 * @param npart Vetor do tipo inteiro com as informações nodais de partição.
 * @param epart Vetor do tipo inteiro com as informações elementares de partição.
 * @param color Vetor do tipo inteiro com as informações de coloração dos elementos internos.
 * @param velocity Vetor do tipo double com informações das velocidades da malha.
 * @param pressure Vetor do tipo float com informações das pressões da malha.
*/

void MeshVTKWriterInternal(mesh_t* mesh, const char* filename, int timeStep, int *nparts, int *epart, int* color, double* velocity, float* pressure);
/**
 * * OBJETIVO:
 *     Escrita da malha somente com elementos internos no formato VTK.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a escrita.
 * @param filename Nome do arquivo de entrada no formato msh.
 * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView.
 *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
 * @param npart Vetor do tipo inteiro com as informações nodais de partição.
 * @param epart Vetor do tipo inteiro com as informações elementares de partição.
 * @param color Vetor do tipo inteiro com as informações de coloração dos elementos internos.
 * @param velocity Vetor do tipo double com informações das velocidades da malha.
 * @param pressure Vetor do tipo float com informações das pressões da malha.
*/

void MeshVTKWriterBinAppended(mesh_t* mesh, const char* filename, int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
/**
 * * OBJETIVO:
 *     Escrita da malha completa em binário, com elementos internos e de superfície, no formato VTK.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a escrita.
 * @param filename Nome do arquivo de entrada no formato msh.
 * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView. 
 *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
 * @param npart Vetor do tipo inteiro com as informações nodais de partição.
 * @param epart Vetor do tipo inteiro com as informações elementares de partição.
 * @param color Vetor do tipo inteiro com as informações de coloração dos elementos internos.
 * @param velocity Vetor do tipo double com informações das velocidades da malha.
 * @param pressure Vetor do tipo float com informações das pressões da malha.
*/

void MeshVTKWriterInternalBinAppended(mesh_t* mesh, const char* filename, int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
/**
 * * OBJETIVO:
 *     Escrita da malha em binário somente com elementos internos no formato VTK.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a escrita.
 * @param filename Nome do arquivo de entrada no formato msh.
 * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView.
 *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
 * @param npart Vetor do tipo inteiro com as informações nodais de partição.
 * @param epart Vetor do tipo inteiro com as informações elementares de partição.
 * @param color Vetor do tipo inteiro com as informações de coloração dos elementos internos.
 * @param velocity Vetor do tipo double com informações das velocidades da malha.
 * @param pressure Vetor do tipo float com informações das pressões da malha.
*/

void MeshReordering(mesh_t *mesh, reorder_t reorder);
/**
 * * OBJETIVO:
 *     Reordenar os elementos de forma que seu armazenamento seja mais otimizado
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a reordenação.
 * @param reorder Algoritmo utilizado para a reordenação podendo ser FF, METIS_ND ou RCM,
 *                sendo RCM a recomendada.
*/

void MeshColoring(mesh_t* mesh);
/**
 * * OBJETIVO:
 *     Coloração dos elementos internos para utilização em paralelismo.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a reordenação.
*/

mesh_partition_t* MeshPartitioner(mesh_t* mesh, int nparts);
/**
 * * OBJETIVO:
 *     Particionar toda a malha para utilização em paralelismo.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a reordenação.
 * @param nparts Número de partições.
 * 
 * * RETORNO:
 *     Ponteiro para estrutura do tipo mesh_partition_t com informações nodais e elementares de particionamento.
*/

mesh_partition_t* MeshPartitionerInternal(mesh_t* mesh, int nparts);
/**
 * * OBJETIVO:
 *     Particionar os elementos internos da malha para utilização em paralelismo.
 * 
 * * PARAMETROS:
 * @param mesh Malha utilizada para a reordenação.
 * @param nparts Número de partições.
 * 
 * * RETORNO:
 *     Ponteiro para estrutura do tipo mesh_partition_t com informações nodais e elementares de particionamento.
*/

void MeshPartitionDestroy(mesh_partition_t* mp);
/**
 * * OBJETIVO:
 *     Deletar os vetores com informações de particionamento
 * 
 * * PARAMETROS:
 * @param mp Estrutura de particionamento a ser deletada.
*/

void MeshDestroy(mesh_t **mesh);
/**
 * * OBJETIVO:
 *     Deletar toda malha e suas informações.
 * 
 * * PARAMETROS:
 * @param mesh Malha a ser deletada.
*/

#endif


