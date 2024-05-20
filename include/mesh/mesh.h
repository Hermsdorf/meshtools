#ifndef MESH_H__
#define MESH_H__

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "meshtools.h"
#include "mesh_coloring.h" 

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

class Element;
class SurfaceElement;
class MeshPartition;

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


typedef std::pair<int, std::string> PhysicalData;

typedef enum {METIS_ND=0, RCM, FF} reorder_t;
typedef enum {BINARY=0, ASCII} write_t;

class Mesh {
    public:

        friend class MeshColoring;
        friend class MeshPartition;
        
        enum class ColoringMode {NONE=0, DEFAULT=1, BLOCKED=2};
        
        /**
         * @brief Construct a new Mesh object
         * 
         */
        Mesh();


        /**
         * @brief Destroy the Mesh object
         * 
         */
        ~Mesh();


        /**
         * @brief Get the number of surface elements 
         * 
         * @return unsigned int 
         */
        unsigned int get_n_surface_elements();


        /**
         * @brief Get the number of elements 
         * 
         * @return unsigned int 
         */
        unsigned int get_n_elements();
    

        /**
         * @brief Get the n nodes object
         * 
         * @return unsigned int 
         */
        unsigned int get_n_nodes();


        /**
         *  @brief Get the number of colors
         * 
         *  @return unsigned int 
         */
        unsigned int get_n_colors();


        /**
         * @brief Get the color size 
         * 
         * @param color 
         * @return unsigned int 
         */
        unsigned int get_color_size(unsigned int color);


        /**
         * @brief Get the mesh dimension 
         * 
         * @return int 
         */
        unsigned int get_mesh_dimension();


        /**
         * @brief Get the element vertices coordinates 
         * 
         * @param iel 
         * @param points 
         */
        void get_element_vertices(const std::vector<unsigned int> &elem_conn, std::vector<Point> &points);


        /**
         * @brief Get the node id 
         * 
         * @param node 
         * @return unsigned int 
         */
        unsigned int get_node_id(unsigned int node);


        /**
         * @brief Get the element connectivity 
         * 
         * @param element_num 
         * @param conn 
         */
        void get_element_connectivity(unsigned int element_num, std::vector<unsigned int> &conn); 


        /**
         * @brief Get the surface element connectivity object
         * 
         * @param element_num 
         * @param conn 
         */
        void get_surface_element_connectivity(unsigned int element_num, std::vector<unsigned int> &conn); 
  

        /**
         *  brief Get the element type 
         * 
         * @param element_num 
         * @return unsigned short 
         */
        unsigned short get_element_type(unsigned int element_num);

        /**
         * @brief Get the surface element type 
         * 
         * @param element_num 
         * @return unsigned short 
         */
        unsigned short get_surface_element_type(unsigned int element_num);
      


       /**
        * @brief Get the element physical tag
        * 
        * @param el 
        * @return unsigned int 
        */
       unsigned int get_element_physical_tag(unsigned int el);

        /**
         * @brief Get the surface element physical tag 
         * 
         * @param el 
         * @return unsigned int 
         */
       unsigned int get_surface_element_physical_tag(unsigned int el);

       /**
        * @brief Read a mesh file
        * 
        * @param filename 
        */
        void read(std::string filename);
 

        /**
         * @brief Coloring the mesh
         * 
         * @param mode 
         * @param block_size 
         */
        void apply_colors(ColoringMode mode = ColoringMode::NONE, int block_size = 4096);


        /**
         * @brief Write mesh 
         * 
         * 
         * @param filename 
         * @param info 
         */
        void write_glvis(string filename, MeshIODataAppended* info = nullptr);

 
        /**
         * @brief Get the mesh element type object
         * 
         * @return unsigned short 
         */
        unsigned short get_mesh_element_type();


        /**
         * @brief Get the surface mesh element type object
         * 
         * @return unsigned short 
         */
        unsigned short get_surface_mesh_element_type();

        /**
         * @brief Get the element with surface element object
         * 
         * @param face_elem_id 
         * @return unsigned int 
         */
        unsigned int get_element_with_surface_element(unsigned int face_elem_id);

        /**
         * @brief Get the physical map object
         * 
         * @return const std::map<int, PhysicalData>& 
         */
        std::map<int, PhysicalData> & get_physical_map();

        /**
         * @brief Set the n elements object
         * 
         * @param _n_elements 
         */
        void   set_n_elements(unsigned int _n_elements);

        /**
         * @brief Set the n surface elements object
         * 
         * @param _n_surface_elements 
         */
        void   set_n_surface_elements(unsigned int _n_surface_elements);

        /**
         * @brief Set the n nodes object
         * 
         * @param _n_nodes 
         */
        void   set_n_nodes(unsigned int _n_nodes);

        void   set_n_colors(unsigned int _n_colors);

        void   set_coordinate_vector(std::vector<double>& _coords);

        void   set_connectivity_vector(std::vector<unsigned int> &_conn);

        void   set_offset_vector(std::vector<unsigned int>&   _offset);

        void   set_element_physical_tag_vector(std::vector<int>& _physical_tag);

        void   set_element_type_vector(std::vector<unsigned short>& _type);

        void   set_node_index_vector(std::vector<unsigned int> &_node_index);

        void   set_mesh_element_type(unsigned int _new_type);

        void   set_surface_mesh_element_type(unsigned int _new_type);

        void   set_mesh_dimension(unsigned int _dim);

        void   set_colors_vector( std::vector<unsigned int> & _colors);

        void   set_physical_map(const std::map<int, PhysicalData> & _new_map);

        void   set_face_to_element_vector(std::vector<unsigned int>& _new_face_to_elem);

        std::vector<double>&         get_coordinate_vector();

        std::vector<unsigned int>&   get_connectivity_vector();

        std::vector<unsigned int>&   get_offset_vector();

        std::vector<unsigned short>& get_element_type_vector(); 

        std::vector<int>&            get_element_physical_tag_vector();

        std::vector<unsigned int>&   get_face_to_element_vector();

        std::vector<unsigned int>&   get_colors_vector();

        std::vector<unsigned int>&   get_node_index_vector();  

        unsigned int get_element_connectivity_size();

        unsigned int get_surface_element_connectivity_size();

        const unsigned int* get_element_connectivity_pointer();

        const unsigned int* get_surface_element_connectivity_pointer();

        const unsigned short* get_element_type_pointer();

        const unsigned short* get_surface_element_type_pointer();   

        void get_only_element_offset_vector(std::vector<unsigned int>& offset);

        void   write_vtk(std::string basename);

    protected:

        // void   extract_boundary_nodes(std::vector<unsigned int>& nodes);

        void   gmsh_reader(const char* filename); 



        void   process_face_to_element();




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
        unsigned short              surface_element_type;
        std::vector<unsigned int>    face_to_element;             // Array indicando  qual elemento interno pertence o elemento de superfície.

        std::map<int, PhysicalData>  physical_map;
        unsigned int dim;                                       // Dimensão da malha.
        

        std::vector<unsigned int> coloring;       // store the number elements for each color
        unsigned int              n_colors;       // Número total de cores dos elementos internos da malha.
};

#endif // MESH_H