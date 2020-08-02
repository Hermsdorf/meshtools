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
        vector<unsigned int> getConn();
        vector<unsigned int> getOffset();
        vector<unsigned short> getType();
        vector<int> get_physical_tag();
        int* get_mesh_coloring_internal();
        unsigned int get_n_internal_colors();
        map<int, physical_data_t> get_physical_map();
        unsigned int get_n_face_elements();
        unsigned int get_n_elements();
        unsigned int get_n_nodes();
        int getDim();
        string getFilename();
        unsigned int* getElementConn(unsigned int element_num); 
        unsigned int* getElementOffset(unsigned int element_num); 
        unsigned int* getSurfaceElementConn(unsigned int element_num); 
        unsigned int* getSurfaceElementOffset(unsigned int element_num);
        unsigned int getElementConnSize(unsigned int element_num); 
        unsigned int getSurfaceElementConnSize(unsigned int element_num); 
        unsigned short getElementType(unsigned int element_num);
        unsigned short getSurfaceElementType(unsigned int element_num);

        void setCoord(vector<double> coord);
        void setConn(vector<unsigned int> conn);
        void setConnPosition(unsigned int value, unsigned int position);
        void setOffset(vector<unsigned int> offset);
        void setOffsetPosition(unsigned int value, unsigned int position);
        void setType(vector<unsigned short> type);
        void set_physical_tag(vector<int> physical_tag);
        void set_mesh_coloring_internal(int* mesh_coloring_internal);
        void set_n_internal_colors(unsigned int n_internal_colors);
        void set_physical_map(map<int, physical_data_t> physical_map);
        void set_n_face_elements(unsigned int n_face_elements);
        void set_n_elements(unsigned int n_elements);
        void set_n_nodes(unsigned int n_nodes);
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
        vector<unsigned int>    conn;            // Conectividade dos elementos.
        vector<unsigned int>    offset;          // Mapeia a localização de cada elemento no vetor conn.
        vector<unsigned short>    type;          // Vetor indicando o tipo de cada elemento.
        vector<int>    physical_tag;             // Vetor indicando o physical tag de cada elemento.
        int* mesh_coloring_internal;             // Vetor indicando as cores dos elementos.
        unsigned int n_internal_colors;          // Número total de cores dos elementos internos da malha.
        map<int, physical_data_t>  physical_map; // 
        unsigned int n_face_elements;            // Numeros de elementos de superficie.
        unsigned int n_elements;                 // Numero de elementos internos.
        unsigned int n_nodes;                    // Numero de nós.
        unsigned int dim;                        // Dimensão da malha.
        string filename;                         // Nome do arquvios de entrada de tipo msh
};

#endif // MESH_H