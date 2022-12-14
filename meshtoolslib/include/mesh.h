
#ifndef MESH_H__
#define MESH_H__

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "meshtools.h"
#include "numeric_vector.h"

using namespace std;


typedef enum {EDGE2=3, TRI3=5, QUAD4=9, TET4=10, HEX8=12} MeshElementType;

typedef enum {Int8=0, UInt8=1,Int32=2, UInt32=3, Float32=4,Float64=5} MeshDataType;
static string MeshDataTypeSTR[6] = {"Int8","UInt8","Int32", "UInt32", "Float32","Float64"}; 

typedef struct 
{
    string        name;
    MeshDataType  type;
    void          *data;

} MeshIODataInfo;


typedef MeshIODataInfo PointData;
typedef MeshIODataInfo CellData;

class MeshIODataAppended{
    
    public:
        MeshIODataAppended();
        void addPointDataInfo(const char* name, MeshDataType type, void *data_ptr);
        void addCellDataInfo(const char* name, MeshDataType type, void *data_ptr);
        void addTimeDataInfo(double time, int time_step);
        std::vector<PointData>& getPointDataInfo() { return list_point_data; };
        std::vector<CellData>& getCellDataInfo() { return list_cell_data;};
        double& getTime() { return time; };
        int& getTimeStep() { return time_step; };
    private:
        std::vector<PointData> list_point_data;
        std::vector<CellData>  list_cell_data;
        double time;
        int time_step;
};


typedef std::pair<int, std::string> physical_data_t;

typedef enum {METIS_ND=0, RCM, FF} reorder_t;
typedef enum {COLOR_DEFAULT=0, COLOR_DEFAULT_BLOCK} color_mode_t;
typedef enum {BINARY=0, ASCII} write_t;

class Mesh;
class Element
{
    friend class Mesh;
    public:
        Element(){};
        std::vector<unsigned int> & connectivity() {return _conn; };
        Point        &              node(int i) {return _coords[i]; } ;
        unsigned short&            type() {return _type; } ;
        unsigned int&              region()  {return _tag; } ;
        unsigned int               n_nodes() {return _conn.size(); };
    private:
        std::vector<unsigned int> _conn;
        std::vector<Point>        _coords;
        unsigned short            _type;
        unsigned int              _tag;
        
};


class Mesh {
    public:
        Mesh();
        /**
         * * OBJETIVO:
         *     Construtor da classe Mesh, responsável por inicializar as variáveis.
        */

        Mesh(const char* filename);
        /**
         * * OBJETIVO:
         *     Construtor da classe Mesh, responsável por inicializar as variáveis e preencher
         *     as informações da malha a partir do nome do arquivo chamando a função MeshGmshReader.
         * * PARAMETROS:
         * @param filename Nome do arquivo, formato msh, com os dados da malha.
        */

        Mesh(std::string filename);
        /**
         * * OBJETIVO:
         *     Construtor da classe Mesh, responsável por inicializar as variáveis e preencher
         *     as informações da malha a partir do nome do arquivo chamando a função MeshGmshReader.
         * * PARAMETROS:
         * @param filename Nome do arquivo, formato msh, com os dados da malha.
        */

        ~Mesh();
        /**
         * * OBJETIVO:
         *     Destrutor da classe Mesh, responsável por desalocar todas as arrays alocadas.
        */

        unsigned int get_n_face_elements();
        /**
         * * OBJETIVO:
         *     Obter o número de elementos de superfície da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned int.
        */

        unsigned int get_n_elements();
        /**
         * * OBJETIVO:
         *     Obter o número de elementos internos da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned int.
        */

        unsigned int get_n_nodes();
        /**
         * * OBJETIVO:
         *     Obter o número total de nós da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned int.
        */

        
        unsigned int get_n_colors();
        /**
         * * OBJETIVO:
         *     Obter o número total de cores dos elementos internas da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned int
        */

        std::vector<double>& getCoord();
        /**
         * * OBJETIVO:
         *     Obter a array coord a qual armazena as coordenadas nodais da malha.
         * 
         * * RETORNO:
         *     Referência a um vector do tipo double.
        */

        std::vector<unsigned int>& getConn();
        /**
         * * OBJETIVO:
         *     Obter a array conn a qual armazena as conectividades dos elementos da malha.
         * 
         * * RETORNO:
         *     Referência a um vector do tipo int.
        */

        std::vector<unsigned int>& getOffset();
        /**
         * * OBJETIVO:
         *     Obter a array offset a qual armazena o offset dos elementos da malha.
         * 
         * * RETORNO:
         *     Referência a um vector do tipo int.
        */

        std::vector<unsigned short>& getType();
        /**
         * * OBJETIVO:
         *     Obter a array type a qual armazena os tipos dos elementos da malha.
         * 
         * * RETORNO:
         *     Referência a um vector do tipo unsigned short.
        */

        std::vector<int>& getPhysicalTag();
        /**
         * * OBJETIVO:
         *     Obter a array phyisical_tag a qual armazena as tags dos grupos físicos da malha.
         * 
         * * RETORNO:
         *     Referência a um vector do tipo int.
        */

        std::vector<unsigned int>& getColoring();
        /**
         * * OBJETIVO:
         *     Obter a a array mesh_coloring_internal que armazena as cores dos elementos internos da malha.
         * 
         * * RETORNO:
         *     Array do tipo int.
        */

        std::map<int, physical_data_t>& getPhysicalMap();
        /**
         * * OBJETIVO:
         *     Obter a variável phyisical_map a qual armazena o mapeamento dos grupos físicos da malha.
         * 
         * * RETORNO:
         *     Variável do tipo map<int, physical_data_t>.
        */

        int getDim();
        /**
         * * OBJETIVO:
         *     Obter a variável dim a qual armazena a dimensão da malha.
         * 
         * * RETORNO:
         *     Variável do tipo int.
        */

        std::string getFilename();
        /**
         * * OBJETIVO:
         *     Obter a variável filename a qual armazena o nome do arquivo msh.
         * 
         * * RETORNO:
         *     Variável do tipo string.
        */

        unsigned int* getElementConn(unsigned int element_num); 
        /**
         * * OBJETIVO:
         *     Obter a conectividade do elemento interno passado por argumento.
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_elements) que indica o elemento de superfície da malha.
         * 
         * * RETORNO:
         *     Array do tipo unsigned int o qual aponta para a posição do elemento
         *     interno desejado no array conn da malha.
        */

        unsigned int* getElementOffset(unsigned int element_num); 
        /**
         * * OBJETIVO:
         *     Obter o offset do elemento interno passado por argumento.
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_elements) que indica o elemento de superfície da malha.
         * 
         * * RETORNO:
         *     Array do tipo unsigned int o qual aponta para a posição do elemento
         *     interno desejado no array offset da malha.
        */

        unsigned int* getSurfaceElementConn(unsigned int element_num); 
        /**
         * * OBJETIVO:
         *     Obter a conectividade do elemento de superfície passado por argumento.
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_face_elements) que indica o elemento de superfície da malha.
         * 
         * * RETORNO:
         *     Array do tipo unsigned int o qual aponta para a posição do elemento
         *     de superfície desejado no array conn da malha.
        */

        unsigned int* getSurfaceElementOffset(unsigned int element_num);
        /**
         * * OBJETIVO:
         *     Obter o offset do elemento de superfície passado por argumento.
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_face_elements) que indica o elemento de superfície da malha.
         * 
         * * RETORNO:
         *     Array do tipo unsigned int o qual aponta para a posição do elemento
         *     de superfície desejado no array offset da malha.
        */

        unsigned int getElementConnSize(unsigned int element_num); 
        /**
         * * OBJETIVO:
         *     Obter o número de conectividades do elemento interno passado por argumento.
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_elements) que indica o elemento interno da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned int com o número de conectividades do elemento interno desejado.
        */

        unsigned int getSurfaceElementConnSize(unsigned int element_num); 
        /**
         * * OBJETIVO:
         *     Obter o número de conectividades do elemento de superfície passado por argumento.
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_face_elements) que indica o elemento interno da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned int com o número de conectividades do elemento de superfície desejado.
        */

        unsigned short getElementType(unsigned int element_num);
        /**
         * * OBJETIVO:
         *     Obter o tipo do elementos interno passado por argumento. 
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_elements) que indica o elemento interno da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned short com o tipo do elemento interno desejado.
        */

        unsigned short getSurfaceElementType(unsigned int element_num);
        /**
         * * OBJETIVO:
         *     Obter o tipo do elementos de superfície passado por argumento. 
         * 
         * * PARAMETROS:
         * @param element_num Número [0, n_face_elements) que indica o elemento de superfície da malha.
         * 
         * * RETORNO:
         *     Variável do tipo unsigned short com o tipo do elemento de superfície desejado.
        */

        void setCoord(std::vector<double> &coord);
        /**
         * * OBJETIVO:
         *     Alterar o vector coord da malha.
         * 
         * * PARAMETROS:
         * @param coord Vector do tipo double com as novas informações de coordenadas dos nós da malha.
        */

        void setConn(std::vector<unsigned int> &conn);
        /**
         * * OBJETIVO:
         *     Alterar o vector conn da malha.
         * 
         * * PARAMETROS:
         * @param conn Vector do tipo unsigned int com as novas informações de conectividades dos elementos da malha.
        */

        void setConnPosition(unsigned int value, unsigned int position);
        /**
         * * OBJETIVO:
         *     Alterar o vector conn com o valor e a posição passados por argumento.
         * 
         * * PARAMETROS:
         * @param value Variável do tipo unsigned int indicando o valor a substituir.
         * @param position Variável do tipo unsigned int indicando a posição a substituir o valor da variável value.
        */

        void setOffset(std::vector<unsigned int> &offset);
        /**
         * * OBJETIVO:
         *     Alterar o vector offset da malha.
         * 
         * * PARAMETROS:
         * @param offset Vector do tipo unsigned int com as novas informações de offset dos elementos da malha.
        */

        void setOffsetPosition(unsigned int value, unsigned int position);
        /**
         * * OBJETIVO:
         *     Alterar o vector offset com o valor e a posição passados por argumento.
         * 
         * * PARAMETROS:
         * @param value Variável do tipo unsigned int indicando o valor a substituir.
         * @param position Variável do tipo unsigned int indicando a posição a substituir o valor da variável value.
        */

        void setType(std::vector<unsigned short> &type);
        /**
         * * OBJETIVO:
         *     Alterar o vector de tipos dos elementos da malha.
         * 
         * * PARAMETROS:
         * @param type Vector do tipo unsigned short com as novas informações de tipos dos elementos da malha.
        */

       void setTypePosition(unsigned short value, unsigned int position);
        /**
         * * OBJETIVO:
         *     Alterar o vector type com o valor e a posição passados por argumento.
         * 
         * * PARAMETROS:
         * @param value Variável do tipo unsigned int indicando o valor a substituir.
         * @param position Variável do tipo unsigned int indicando a posição a substituir o valor da variável value.
        */

        void set_physical_tag(std::vector<int> &physical_tag);
        /**
         * * OBJETIVO:
         *     Alterar o vector de phyisical tag da malha.
         * 
         * * PARAMETROS:
         * @param phyisical_tag Vector do tipo int com as novas informações de phyisical tag da malha.
        */

        void set_n_colors(unsigned int n_colors);
        /**
         * * OBJETIVO:
         *     Alterar o número total de cores nos elementos internos da malha.
         * 
         * * PARAMETROS:
         * @param n_internal_colors Variável do tipo unsigned int com o novo número a ser atualizado de cores internas da malha.
        */

        void set_physical_map(std::map<int, physical_data_t> &physical_map);
        /**
         * * OBJETIVO:
         *     Alterar o mapeamento dos grupos físicos da malha.
         * 
         * * PARAMETROS:
         * @param physical_map Variável do tipo map<int, physical_data_t> com o novo mapeamento do grupo físico da malha para ser atualizado.
        */

        void set_n_face_elements(unsigned int n_face_elements);
        /**
         * * OBJETIVO:
         *     Alterar o número de elementos de superfície presentes na malha
         * 
         * * PARAMETROS:
         * @param n_face_elements Variável do tipo unsigned int com o novo número de elementos de superfície para ser atualizado.
        */

        void set_n_elements(unsigned int n_elements);
        /**
         * * OBJETIVO:
         *     Alterar o número de elementos internos presentes na malha.
         * 
         * * PARAMETROS:
         * @param n_elements Variável do tipo unsigned int com o novo número de elemento internos para ser atualizado.
        */

        void set_n_nodes(unsigned int n_nodes);
        /**
         * * OBJETIVO:
         *     Alterar o número de nós presentes na malha.
         * 
         * * PARAMETROS:
         * @param n_nodes Variável do tipo unsigned int com o novo número de nós para ser atualizado.
        */

        void setDim(int dim);
        /**
         * * OBJETIVO:
         *     Alterar a variável dim, a qual armazena a dimensão da malha.
         * 
         * * PARAMETROS:
         * @param dim Variável do tipo int com a nova dimensão da malha, podendo ser 1, 2 ou 3.
        */

        void setFilename(std::string filename);
        /**
         * * OBJETIVO:
         *     Alterar a variável filename, a qual armazena o nome do arquivo formato msh.
         * 
         * * PARAMETROS:
         * @param filename Variável do tipo string com o novo nome do arquivo de entrada extensão msh.
        */

        void MeshGmshReader(const char* filename);
        /**
         * * OBJETIVO:
         *     Ler o arquivo no formato msh e preencher as informações da estrutura mesh_t.
         * 
         * * PARAMETROS:
         * @param filename Variável do tipo const char* com o nome do arquivo de entrada extensão msh.
        */

       std::vector<unsigned int>& getNodeIndexes()
       {
            return this->node_index;
       }
       
        void MeshReordering(reorder_t reorder);
        /**
         * * OBJETIVO:
         *     Reordenar os elementos de forma que seu armazenamento seja mais otimizado
         * 
         * * PARAMETROS:
         * @param reorder Algoritmo utilizado para a reordenação podendo ser FF, METIS_ND ou RCM.
         *                Recomendamos o algoritmo RCM.          
        */

        void MeshColoring(color_mode_t cmode=COLOR_DEFAULT_BLOCK, int block_size=4096);
        /**
         * * OBJETIVO:
         *     Coloração dos elementos internos para utilização em paralelismo com memória compartilada (OpenMP).
         *    
         * * ALGORITMO:
         *     First-Fit Coloring.
        */

        // TODO: removing this function
        void MeshColoring_test();
        /**
         * * OBJETIVO:
         *     Testar se a coloração calculada no algoritmo está correta.
        */

       void WriteVTK(const char* filename, MeshIODataAppended* info = nullptr);

       void WriteMTS(const char* filename);


       void extract_boundary_nodes(std::vector<int>& tag);


       void get_element_coordinates(int element_id, std::vector<Point> &coordinates);

       void get_element_connectivity(int element_id, std::vector<unsigned int> &connectivity);

       void getElement(unsigned int element_id, Element& elem);

        inline int getElementTag(int iel) { return physical_tag[iel+n_face_elements]; };

        int getSurfaceElementTag(int iel) { return physical_tag[iel]; };

        unsigned int get_mesh_element_type(){ return element_type; }

        unsigned int get_boundary_mesh_element_type() {return boundary_element_type; }

        unsigned int getElementConnectivitySize();

        unsigned int getBoundaryElementConnectivitySize();

        unsigned int *getElementConnectivityData();

        unsigned int *getBoundaryElementsConnectivityData() ;

        double       *getCoordinatesData(); 

        std::vector<int> getFaceToElement() { return face_to_element; };
        void             setFaceToElement(std::vector<int> face_to_element) { this->face_to_element = face_to_element; };
        int              getVTKElemContourNNodes(int vtk_type);
        int              getVTKElemContourNFaces(int vtk_type);
        void*            getVTKElemConnSequence(int vtk_type);
        int              getGmshElemNNodes(int type);
        int              getGmshElemTypeDim(int type);
        void             process_face_to_element();
    


    protected:
        unsigned int                n_face_elements;            // Numero de elementos de superficie.
        unsigned int                n_elements;                 // Numero de elementos internos.
        unsigned int                n_nodes;                    // Numero de nós.
        std::vector<double>         coord;                      // Coordenadas nodais.
        std::vector<unsigned int>   conn;                       // Conectividade dos elementos.
        std::vector<unsigned int>   offset;                     // Mapeia a localização de cada elemento no array conn.
        std::vector<unsigned short> type;                       // Array indicando o tipo de cada elemento.
        std::vector<int>            physical_tag;               // Array indicando o physical tag de cada elemento.
        std::vector<int>            boundary_nodes;
        std::vector<unsigned int>   node_index;                 // Array indicando o índice de cada nó.
        unsigned short              element_type;
        unsigned short              boundary_element_type;
        std::vector<int>            face_to_element;             // Array indicando  qual elemento interno pertence o elemento de superfície.

        std::map<int, physical_data_t>  physical_map;
        unsigned int dim;                                       // Dimensão da malha.
        
        // TODO: remover 
        std::string filename;                    // Nome do arquvios de entrada de tipo msh
        std::vector<unsigned int> coloring;
        unsigned int              n_colors;       // Número total de cores dos elementos internos da malha.
};

#endif // MESH_H