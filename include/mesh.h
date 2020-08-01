#ifndef MESH_H__
#define MESH_H__

#include <vector>
#include <string>
#include <map>
using namespace std;

typedef std::pair<int, string> physical_data_t;

typedef enum {METIS_ND=0, RCM, FF} reorder_t;

class Mesh {
    public:
        Mesh();
        Mesh(const char* filename);
        ~Mesh();
        vector<double> getCoord();
        vector<int> getConn();
        vector<int> getOffset();
        vector<unsigned short> getType();
        vector<int> get_Physical_tag();
        int* get_Mesh_coloring_internal();
        int get_N_internal_colors();
        map<int, physical_data_t> getPhysical_map();
        int get_N_face_elements();
        int get_N_elements();
        int get_N_nodes();
        int getDim();
        string getFilename();
        int* getElementConn(int element_num); 
        int* getElementOffset(int element_num); 
        int* getSurfaceElementConn(int element_num); 
        int* getSurfaceElementOffset(int element_num);
        int getElementConnSize(int element_num); 
        int getSurfaceElementConnSize(int element_num); 
        void setCoord(vector<double> coord);
        void setConn(vector<int> conn);
        void setConnPosition(unsigned int value, int position);
        void setOffset(vector<int> offset);
        void setOffsetPosition(unsigned int value, int position);
        void setType(vector<unsigned short> type);
        void set_Physical_tag(vector<int> physical_tag);
        void set_Mesh_coloring_internal(int* mesh_coloring_internal);
        void set_N_internal_colors(int n_internal_colors);
        void set_Physical_map(map<int, physical_data_t> physical_map);
        void set_N_face_elements(int n_face_elements);
        void set_N_elements(int n_elements);
        void set_N_nodes(int n_nodes);
        void setDim(int dim);
        void setFilename(string filename);

        void MeshGmshReader(const char* filename);
        void MeshVTKWriter(int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure);
        void MeshVTKWriterInternal(int timeStep, int *nparts, int *epart, int* color, double* velocity, float* pressure);
        void MeshVTKWriterBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
        void MeshVTKWriterInternalBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
        void MeshReordering(reorder_t reorder);
        void MeshColoring();

    private:
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
        string filename;                    // Nome do arquvios de entrada de tipo msh
};

#endif // MESH_H