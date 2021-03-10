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

        
        unsigned int get_n_internal_colors();
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

        std::vector<int>& get_physical_tag();
        /**
         * * OBJETIVO:
         *     Obter a array phyisical_tag a qual armazena as tags dos grupos físicos da malha.
         * 
         * * RETORNO:
         *     Referência a um vector do tipo int.
        */

        int* get_mesh_coloring_internal();
        /**
         * * OBJETIVO:
         *     Obter a a array mesh_coloring_internal que armazena as cores dos elementos internos da malha.
         * 
         * * RETORNO:
         *     Array do tipo int.
        */

        std::map<int, physical_data_t> get_physical_map();
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

        void setCoord(std::vector<double> coord);
        /**
         * * OBJETIVO:
         *     Alterar o vector coord da malha.
         * 
         * * PARAMETROS:
         * @param coord Vector do tipo double com as novas informações de coordenadas dos nós da malha.
        */

        void setConn(std::vector<unsigned int> conn);
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

        void setOffset(std::vector<unsigned int> offset);
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

        void setType(std::vector<unsigned short> type);
        /**
         * * OBJETIVO:
         *     Alterar o vector de tipos dos elementos da malha.
         * 
         * * PARAMETROS:
         * @param type Vector do tipo unsigned short com as novas informações de tipos dos elementos da malha.
        */

        void set_physical_tag(std::vector<int> physical_tag);
        /**
         * * OBJETIVO:
         *     Alterar o vector de phyisical tag da malha.
         * 
         * * PARAMETROS:
         * @param phyisical_tag Vector do tipo int com as novas informações de phyisical tag da malha.
        */

        void set_mesh_coloring_internal(int* mesh_coloring_internal);
        /**
         * * OBJETIVO:
         *     Alterar o array de cores da malha.
         * 
         * * PARAMETROS:
         * @param mesh_coloring_internal Array do tipo int* com a nova informação de coloração dos elementos internos da malha.
        */

        void set_n_internal_colors(unsigned int n_internal_colors);
        /**
         * * OBJETIVO:
         *     Alterar o número total de cores nos elementos internos da malha.
         * 
         * * PARAMETROS:
         * @param n_internal_colors Variável do tipo unsigned int com o novo número a ser atualizado de cores internas da malha.
        */

        void set_physical_map(std::map<int, physical_data_t> physical_map);
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

        void MeshVTKWriter(int timeStep, int *npart, int* epart, int* color, double* velocity, float* pressure);
        /**
         * * OBJETIVO:
         *     Escrita da malha completa, com elementos internos e de superfície, no formato VTK.
         * 
         * * PARAMETROS:
         * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView.
         *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
         * @param npart Array do tipo inteiro com as informações nodais de partição.
         * @param epart Array do tipo inteiro com as informações elementares de partição.
         * @param color Array do tipo inteiro com as informações de coloração dos elementos internos.
         * @param velocity Array do tipo double com informações das velocidades da malha.
         * @param pressure Array do tipo float com informações das pressões da malha.
        */
        void MeshVTKWriterInternal(int timeStep, int *nparts, int *epart, int* color, double* velocity, float* pressure);
        /**
         * * OBJETIVO:
         *     Escrita da malha somente com elementos internos no formato VTK.
         * 
         * * PARAMETROS:
         * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView.
         *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
         * @param npart Array do tipo inteiro com as informações nodais de partição.
         * @param epart Array do tipo inteiro com as informações elementares de partição.
         * @param color Array do tipo inteiro com as informações de coloração dos elementos internos.
         * @param velocity Array do tipo double com informações das velocidades da malha.
         * @param pressure Array do tipo float com informações das pressões da malha.
        */

        void MeshVTKWriterBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
        /**
         * * OBJETIVO:
         *     Escrita da malha completa em binário, com elementos internos e de superfície, no formato VTK.
         * 
         * * PARAMETROS:
         * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView. 
         *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
         * @param npart Array do tipo inteiro com as informações nodais de partição.
         * @param epart Array do tipo inteiro com as informações elementares de partição.
         * @param color Array do tipo inteiro com as informações de coloração dos elementos internos.
         * @param velocity Array do tipo double com informações das velocidades da malha.
         * @param pressure Array do tipo float com informações das pressões da malha.
        */
        void MeshVTKWriterInternalBinAppended(int timeStep, int* npart, int* epart, int* color, double* velocity, float* pressure);
        /**
         * * OBJETIVO:
         *     Escrita da malha em binário somente com elementos internos no formato VTK.
         * 
         * * PARAMETROS:
         * @param timeStep Variável para criar uma sequência de arquivos a serem abertos no ParaView.
         *                 (Para gerar somente um arquivo da malha, inserir 0 no valor do timeStep)
         * @param npart Array do tipo inteiro com as informações nodais de partição.
         * @param epart Array do tipo inteiro com as informações elementares de partição.
         * @param color Array do tipo inteiro com as informações de coloração dos elementos internos.
         * @param velocity Array do tipo double com informações das velocidades da malha.
         * @param pressure Array do tipo float com informações das pressões da malha.
        */

        void MeshReordering(reorder_t reorder);
        /**
         * * OBJETIVO:
         *     Reordenar os elementos de forma que seu armazenamento seja mais otimizado
         * 
         * * PARAMETROS:
         * @param reorder Algoritmo utilizado para a reordenação podendo ser FF, METIS_ND ou RCM.
         *                Recomendamos o algoritmo RCM.          
        */

        void MeshColoring();
        /**
         * * OBJETIVO:
         *     Coloração dos elementos internos para utilização em paralelismo com memória compartilada (OpenMP).
         *    
         * * ALGORITMO:
         *     First-Fit Coloring.
        */

    protected:
        unsigned int n_face_elements;            // Numero de elementos de superficie.
        unsigned int n_elements;                 // Numero de elementos internos.
        unsigned int n_nodes;                    // Numero de nós.
        std::vector<double> coord;               // Coordenadas nodais.
        std::vector<unsigned int> conn;          // Conectividade dos elementos.
        std::vector<unsigned int> offset;        // Mapeia a localização de cada elemento no array conn.
        std::vector<unsigned short> type;        // Array indicando o tipo de cada elemento.
        std::vector<int> physical_tag;           // Array indicando o physical tag de cada elemento.
        int* mesh_coloring_internal;             // Array indicando as cores dos elementos.
        unsigned int n_internal_colors;          // Número total de cores dos elementos internos da malha.
        std::map<int, physical_data_t>  physical_map;
        unsigned int dim;                        // Dimensão da malha.
        std::string filename;                    // Nome do arquvios de entrada de tipo msh
};

#endif // MESH_H