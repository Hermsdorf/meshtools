#include <iostream>
#include <map>
#include <vector>

#ifndef MESH_H__
#define MESH_H__

typedef std::pair<int, std::string> physical_data_t;

typedef enum {METIS_ND=0, RCM, FF} reorder_t;

class Mesh {
    public:
        Mesh();
        Mesh(const char* filename);
        ~Mesh();
        unsigned int get_n_face_elements();
        unsigned int get_n_elements();
        unsigned int get_n_nodes();
        unsigned int get_n_internal_colors();
        std::vector<double>& getCoord();
        std::vector<unsigned int>& getConn();
        std::vector<unsigned int>& getOffset();
        std::vector<unsigned short>& getType();
        std::vector<int>& get_physical_tag();
        int* get_mesh_coloring_internal();
        std::map<int, physical_data_t> get_physical_map();
        int getDim();
        std::string getFilename();
        unsigned int* getElementConn(unsigned int element_num); 
        unsigned int* getElementOffset(unsigned int element_num); 
        unsigned int* getSurfaceElementConn(unsigned int element_num); 
        unsigned int* getSurfaceElementOffset(unsigned int element_num);
        unsigned int getElementConnSize(unsigned int element_num); 
        unsigned int getSurfaceElementConnSize(unsigned int element_num); 
        unsigned short getElementType(unsigned int element_num);
        unsigned short getSurfaceElementType(unsigned int element_num);

        void setCoord(std::vector<double> coord);
        void setConn(std::vector<unsigned int> conn);
        void setConnPosition(unsigned int value, unsigned int position);
        void setOffset(std::vector<unsigned int> offset);
        void setOffsetPosition(unsigned int value, unsigned int position);
        void setType(std::vector<unsigned short> type);
        void set_physical_tag(std::vector<int> physical_tag);
        void set_mesh_coloring_internal(int* mesh_coloring_internal);
        void set_n_internal_colors(unsigned int n_internal_colors);
        void set_physical_map(std::map<int, physical_data_t> physical_map);
        void set_n_face_elements(unsigned int n_face_elements);
        void set_n_elements(unsigned int n_elements);
        void set_n_nodes(unsigned int n_nodes);
        void setDim(int dim);
        void setFilename(std::string filename);

        void MeshGmshReader(const char* filename);
        void MeshVTKWriter(int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure);
        void MeshVTKWriterInternal(int timeStep, int *nparts, int *epart, int* color, double* velocity, float* pressure);
        void MeshVTKWriterBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
        void MeshVTKWriterInternalBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
        void MeshReordering(reorder_t reorder);
        void MeshColoring();

    private:
        unsigned int n_face_elements;            // Numeros de elementos de superficie.
        unsigned int n_elements;                 // Numero de elementos internos.
        unsigned int n_nodes;                    // Numero de nós.
        std::vector<double> coord;               // Coordenadas nodais.
        std::vector<unsigned int>    conn;       // Conectividade dos elementos.
        std::vector<unsigned int>    offset;     // Mapeia a localização de cada elemento no vetor conn.
        std::vector<unsigned short>    type;     // Vetor indicando o tipo de cada elemento.
        std::vector<int>    physical_tag;        // Vetor indicando o physical tag de cada elemento.
        int* mesh_coloring_internal;             // Vetor indicando as cores dos elementos.
        unsigned int n_internal_colors;          // Número total de cores dos elementos internos da malha.
        std::map<int, physical_data_t>  physical_map;
        unsigned int dim;                        // Dimensão da malha.
        std::string filename;                    // Nome do arquvios de entrada de tipo msh
};

#endif // MESH_H