#include <iostream>
#include <cassert>
#include <set>
#include <unordered_map>
#include "mesh.h" 
#include "numeric_vector.h"

#include "vtk_writer.h"
#include "glvis_writer.h"

#include "hash.h"

#include "mesh_helper.h"

using namespace std;

// //unsigned int edge2_faces[2][1] = {{0},{1}};

// unsigned int edge2_faces[2] = {0,1};

// //unsigned int quad4_faces[4][2] = {{0,1},{1,2},{2,3},{3,0}};
// unsigned int quad4_faces[8] = {0,1,1,2,2,3,3,0}; // 2D

// //unsigned int tri3_faces[3][2]  = {{0,1},{1,2},{2,0}};
// unsigned int tri3_faces[6]  = {0,1,1,2,2,0}; // 2D

// //unsigned int tet4_faces[4][3]  = {{0,2,1},{0,3,2},{0,1,3},{1,2,3}};
// unsigned int tet4_faces[12]  = {0,2,1,0,3,2,0,1,3,1,2,3}; // 3D

// //unsigned int hex8_faces[6][4]  = {{0,1,2,3},{4,5,6,7},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7}};
// unsigned int hex8_faces[24]  = {0,1,2,3,4,5,6,7,0,1,5,4,1,2,6,5,2,3,7,6,3,0,4,7}; // 3D

Mesh::Mesh()
{
    this->n_face_elements = 0;
    this->n_elements      = 0;
    this->n_nodes         = 0; 
    this->dim             = 0;
    this->n_colors = 0;
}



void Mesh::read(std::string filename)
{
    this->n_face_elements = 0;
    this->n_elements = 0;
    this->n_nodes = 0; 
    this->dim = 0;
    this->n_colors = 0;

    if(filename.find(".msh") != string::npos)
    {
        gmsh_reader(filename.c_str());
    }
    else
    {
        runtime_error("Mesh::Read: File not suported!");
    }
    
    this->n_colors = 1;
    this->coloring.resize(1);
    this->coloring[0] = this->n_elements;
}

Mesh::~Mesh()
{
    this->conn.clear();
    this->coord.clear();
    this->offset.clear();
    this->type.clear();
    this->physical_tag.clear();
    this->physical_map.clear();
    this->coloring.clear();
    this->face_to_element.clear();
}

unsigned int Mesh::get_mesh_dimension()
{
    return this->dim;
}

unsigned int Mesh::get_n_surface_elements()
{
    return this->n_face_elements;
}


unsigned int Mesh::get_n_elements()
{
    return this->n_elements;
}

unsigned int Mesh::get_n_nodes()
{
    return this->n_nodes;
}

unsigned int Mesh::get_n_colors()
{
    return this->n_colors;
}

unsigned int Mesh::get_color_size(unsigned int c)
{
    assert(c>=0 && c < this->n_colors);
    return this->coloring[c];
}

void Mesh::get_surface_element_connectivity(unsigned int iel, std::vector<unsigned int> &_conn)
{
    assert(iel >=0 && iel < this->n_face_elements);

    unsigned int offset_start = this->offset[iel];
    unsigned int offset_end   = this->offset[iel+1];
    unsigned int size = offset_end - offset_start;
    _conn.resize(size);
    for(unsigned int j= 0, i = offset_start; i < offset_end; i++, j++)
    {
        _conn[j] = this->conn[i];
    }
}

void Mesh::get_element_connectivity(unsigned int iel, std::vector<unsigned int> &_conn)
{
    assert(iel >=0 && iel < this->n_elements);
    unsigned int offset_start = this->offset[this->n_face_elements + iel];
    unsigned int offset_end   = this->offset[this->n_face_elements+iel+1];
    unsigned int size = offset_end - offset_start;
    _conn.resize(size);
    for(unsigned int j= 0, i = offset_start; i < offset_end; i++, j++)
    {
        _conn[j] = this->conn[i];
    }
}


void Mesh::get_element_vertices(const std::vector<unsigned int> &elem_conn, std::vector<Point> &points)
{
    points.resize(elem_conn.size());
    for(int i = 0; i < elem_conn.size();i++)
    {
        unsigned int node = elem_conn[i];
        points[i](0) = this->coord[node*3  ];
        points[i](1) = this->coord[node*3+1];
        points[i](2) = this->coord[node*3+2];
    }
}

unsigned int Mesh::get_node_id(unsigned int node)
{
    assert(node >= 0 && node < this->n_nodes);

    return this->node_index[node];

}

unsigned short Mesh::get_surface_element_type(unsigned int iel)
{
    assert(iel >=0 && iel < this->n_face_elements);
    return this->type[iel];
}

unsigned short Mesh::get_element_type(unsigned int iel)
{
    assert(iel >=0 && iel < this->n_elements);
    return this->type[iel+this->n_face_elements];
}


unsigned int Mesh::get_surface_element_physical_tag(unsigned int iel)
{
    assert(iel >=0 && iel < this->n_face_elements);
    return this->physical_tag[iel];
}

unsigned int Mesh::get_element_physical_tag(unsigned int iel)
{
    assert(iel >=0 && iel < this->n_elements);
    return this->physical_tag[iel];
}


void Mesh::apply_colors(ColoringMode mode, int block_size)
{
    if(mode == ColoringMode::NONE) return; 

    std::cout << "Starting mesh coloring...\n";
    
    switch(mode)
    {
        case ColoringMode::DEFAULT:
            MeshColoring::apply_coloring(*this);

            break;
        case ColoringMode::BLOCKED:
            MeshColoring::apply_blocked_coloring(*this);
            break;
        default:
            break; 
    }
    
}


unsigned short Mesh::get_mesh_element_type()
{
    return this->type[this->n_face_elements];
}

unsigned short Mesh::get_surface_mesh_element_type()
{
    return this->type[0];
}

unsigned int Mesh::get_element_with_surface_element(unsigned int face_elem_id)
{
    assert(face_elem_id>=0 && face_elem_id < this->n_face_elements);

    return this->face_to_element[face_elem_id];

}

void   Mesh::set_n_elements(unsigned int _n_elements)
{
    this->n_elements = _n_elements;
}

void   Mesh::set_n_surface_elements(unsigned int _n_surface_elements)
{
    this->n_face_elements = _n_surface_elements;
}

void   Mesh::set_n_nodes(unsigned int _n_nodes)
{
    this->n_nodes;
    this->coord.resize(this->n_nodes*3);
}

void   Mesh::set_n_colors(unsigned int _n_colors)
{
    this->n_colors = _n_colors;
}

void   Mesh::set_coordinate_vector(std::vector<double>& _coords)
{
    this->coord.swap(_coords);
}

void  Mesh::set_connectivity_vector(std::vector<unsigned int> &_conn)
{
    this->conn.swap(_conn);
}

void  Mesh::set_offset_vector(std::vector<unsigned int>&   _offset)
{
    this->offset.swap(_offset);
}

void  Mesh::set_element_physical_tag_vector(std::vector<int>& _physical_tag)
{
    this->physical_tag.swap(_physical_tag);
}

void   Mesh::set_element_type_vector(std::vector<unsigned short>& _type)
{
    this->type.swap(_type);

}

void   Mesh::set_node_index_vector(std::vector<unsigned int> &_node_index)
{
    this->node_index.swap(_node_index);
}

void   Mesh::set_mesh_element_type(unsigned int _new_type)
{
    this->element_type = _new_type;
}

void   Mesh::set_surface_mesh_element_type(unsigned int _new_type)
{
    this->surface_element_type = _new_type;
}

void   Mesh::set_mesh_dimension(unsigned int _dim)
{
    this->dim = _dim;
}

void   Mesh::set_colors_vector( std::vector<unsigned int> & _colors)
{
    this->coloring.swap(_colors);
}

void   Mesh::set_face_to_element_vector(std::vector<unsigned int>& _new_face_to_elem)
{
    this->face_to_element.swap(_new_face_to_elem);
}

void  Mesh::set_physical_map(const std::map<int, PhysicalData> & _new_map)
{
    this->physical_map = _new_map;
}

std::map<int, PhysicalData> & Mesh::get_physical_map()
{
    return this->physical_map;
}

std::vector<double>&      Mesh::get_coordinate_vector()
{
    return this->coord;
}

std::vector<unsigned int>&   Mesh::get_connectivity_vector()
{
    return this->conn;
}

std::vector<unsigned int>&   Mesh::get_offset_vector()
{
    return this->offset;
}

std::vector<unsigned short>& Mesh::get_element_type_vector()
{
    return this->type;
} 

std::vector<int>&            Mesh::get_element_physical_tag_vector()
{
    return this->physical_tag;
}

std::vector<unsigned int>&   Mesh::get_face_to_element_vector()
{
    return this->face_to_element;
}

std::vector<unsigned int>&   Mesh::get_colors_vector()
{
    return this->coloring;
}

std::vector<unsigned int>&   Mesh::get_node_index_vector()
{
    return this->node_index;
}

// void Mesh::set_physical_map(std::map<int, physical_data_t> &physical_map)
// {
//     this->physical_map = physical_map;
// }

// void Mesh::set_n_face_elements(unsigned int n_face_elements)
// {
//     this->n_face_elements = n_face_elements;
// }

// void Mesh::set_n_elements(unsigned int n_elements)
// {
//     this->n_elements = n_elements;
// }



// void Mesh::extract_boundary_nodes(std::vector<unsigned int>& nodes)
// {  
//     for(auto it = physical_map.begin(); it != physical_map.end(); ++it)
//     {
//         if(it->second.first != (dim-1)) continue;

//         std::set<int>  node_on_boundary;

//         for(int iel=0; iel < this->n_face_elements; iel++)
//         {
//             if(tag[iel] == it->first)
//             {
//                 unsigned int connsize = getSurfaceElementConnSize(iel); 
//                 unsigned int* conn     = getSurfaceElementConn(iel); 
//                 unsigned int start =  this->offset[iel];
//                 unsigned int end   =  this->offset[iel+1];
//                 for(int i = start; i < end; ++i)
//                     node_on_boundary.insert(this->conn[i]);
//             }
//         }
//     }
// }


// void Mesh::getElement(unsigned int element_id, Element& elem)
// {
//     get_element_connectivity(element_id,elem._conn);
//     get_element_coordinates(element_id,elem._coords);
//     elem._type= this->getElementType(element_id);
//     elem._tag = this->getElementTag(element_id);
// }

// void Mesh::getSurfaceElement(unsigned int surface_element_id, SurfaceElement& surface_elem)
// {
//     get_surface_element_connectivity(surface_element_id,surface_elem._conn);
//     get_surface_element_coordinates(surface_element_id,surface_elem._coords);
//     surface_elem._type= this->getSurfaceElementType(surface_element_id);
//     surface_elem._tag = this->getSurfaceElementTag(surface_element_id);
    
//     Element internal_element;
//     this->getElement(this->face_to_element[surface_element_id], internal_element);
//     surface_elem.set_internal_element(internal_element);
// }

// unsigned int Mesh::getElementConnectivitySize()
// {
//     return this->coord.size() - this->offset[this->n_face_elements];
// }

// unsigned int Mesh::getBoundaryElementConnectivitySize()
// {
//     return this->offset[this->n_face_elements];
// }

// unsigned int* Mesh::getElementConnectivityData()
// {
//     unsigned int nfo = this->offset[this->n_face_elements];
//     return &this->conn[nfo];
// }

// unsigned int* Mesh::getBoundaryElementsConnectivityData()
// {
//     return &this->conn[0];
// }

// double * Mesh::getCoordinatesData()
// {
//     return &this->coord[0];
// }

// // By the element type this method returns the number of nodes at the element's faces
// int Mesh::getVTKElemContourNNodes(int vtk_type)
// {
//     switch (vtk_type)
//     {
//         case 3: return 2;   // EDGE2
//         case 5: return 2;   // TRI3
//         case 9: return 2;   // QUAD4
//         case 10: return 3;  // TET4
//         case 12: return 4;  // HEX8
//         default: return -1;
//         break;
//     }
// }

// // By the element type this method returns the number of element's faces
// int Mesh::getVTKElemContourNFaces(int vtk_type)
// {
//     switch (vtk_type)
//     {
//         case 3: return 2;  // EDGE2
//         case 5: return 3;  // TRI3
//         case 9: return 4;  // QUAD4
//         case 10: return 4; // TET4
//         case 12: return 6; // HEX8
//         default: return -1;
//         break;
//     }
// }

// // By the element type this method returns the number of element's faces
// unsigned int* getVTKElemConnSequence(int vtk_type)
// {
//     switch (vtk_type)
//     {
//         case 3:  return edge2_faces; // EDGE2
//         case 5:  return tri3_faces;  // TRI3
//         case 9:  return quad4_faces; // QUAD4
//         case 10: return tet4_faces;  // TET4
//         case 12: return hex8_faces;  // HEX8
//         default: return nullptr;
//         break;
//     }
// }


// int Mesh::getGmshElemNNodes(int type)
// {
//     switch (type)
//     {
//         case 1: return 2; // EDGE2
//         case 2: return 3; // TRI3
//         case 3: return 4; // QUAD4
//         case 4: return 4; // TET4
//         case 5: return 8; // HEX8
//         case 15: return 1;
//         default: return -1;
//         break;
//     }
// }

// int Mesh::getGmshElemTypeDim(int type)
// {
//     switch (type)
//     {
//         case 1: return 1; // EDGE2
//         case 2: return 2; // TRI3
//         case 3: return 2; // QUAD4
//         case 4: return 3; // TET4
//         case 5: return 3; // HEX8
//         case 15: return 0;
//         default: return -1;
//         break;
//     }
// }



void Mesh::process_face_to_element()
{
    int n_face_elements  = this->n_face_elements;
    int n_elements       = this->n_elements;
    int dim              = this->dim;

    this->face_to_element.resize(n_face_elements);
    std::fill(face_to_element.begin(), face_to_element.end(),-1);

    unordered_map<unsigned long long, unsigned int> face_elements_hash;

    // Calculating hash to each surface element
    for (int i = 0; i < n_face_elements; i++) 
    {
        unsigned offset_start = this->offset[i];
        unsigned offset_end   = this->offset[i+1];

        std::vector<unsigned int> conn_tmp(offset_end - offset_start);

        for (unsigned int j = 0, i = offset_start; i < offset_end; i++, j++) {
            conn_tmp[j] = this->conn[i];
        }

        std::sort(conn_tmp.begin(), conn_tmp.end());

        unsigned long long element_hash = compute_hash(conn_tmp.size(), conn_tmp.data());

        // unsigned long long element_hash = conn_tmp[0];
        // for (unsigned short conn_i = 1 ; conn_i < surf_element_nnodes ; conn_i++){
        //     element_hash = cantor_pairing(element_hash, conn_tmp[conn_i]);
        // }

        // unordered_map[hash] = face_id
        face_elements_hash[element_hash] = i;

#ifdef NDEGUG
        std::cout << "Element " << i << " hash: " << element_hash << " Nodes: ";
        for(int j = 0; j < surf_element_nnodes; j++) {
            std::cout << " " << conn_tmp[j];
        }
        std::cout << std::endl;
#endif
    }

    if (face_elements_hash.size() != n_face_elements)
    {
        std::cout << "Face to element relation wasn't calculated correctly, ";
        std::cout << "there are equal hashs to different elements, exiting..." << std::endl;
        exit(1);
    }


    // Filling face_to_element array
    for(unsigned int elem_i = 0; elem_i < n_elements; elem_i++) 
    {

        unsigned int offset_start = this->offset[n_face_elements + elem_i    ];
        unsigned int offset_end   = this->offset[n_face_elements + elem_i + 1];
        unsigned int n_nodes_element = offset_end-offset_start;
        std::vector<unsigned int> element_conn_vec(n_nodes_element);
        for (int j = 0, i = offset_start; i < offset_end; i++, j++) 
            element_conn_vec[j] = this->conn[i];

        // Getting element's faces
        int n_faces      = MeshHelper::VTKIdToNumberFaces[element_type]; 
                    
        // getting face nodes
        int  n_face_nodes = MeshHelper::VTKIdToNumberFaceNodes[element_type];

        // getint face to node mapping
        unsigned int* face_map     = MeshHelper::VtkIdToFaceMap[element_type];
       
        for(int face_i = 0; face_i < n_faces; face_i++)
        {

            std::vector<unsigned int> face_nodes(n_face_nodes);
            for(int face_node_i = 0; face_node_i < n_face_nodes ; face_node_i++)
            {
                int local_node = face_map[face_i*n_face_nodes + face_node_i];
                face_nodes[face_node_i] = element_conn_vec[local_node];
            }

            std::sort(face_nodes.begin(), face_nodes.end());
            unsigned long element_hash = compute_hash(n_face_nodes, face_nodes.data());

            // unsigned long element_hash = face_nodes[0];
            // for (unsigned short conn_i = 1 ; conn_i < face_nodes.size() ; conn_i++)
            //     element_hash = cantor_pairing(element_hash, face_nodes[conn_i]);

#ifdef NDEGUG
            std::cout << "Element " << elem_i << " face " << face_i << " hash: " << element_hash << " Nodes: ";
            for(int j = 0; j < face_nodes.size(); j++) {
                std::cout << " " << face_nodes[j];
            }
            std::cout << std::endl;
#endif

            if (face_elements_hash.find(element_hash) != face_elements_hash.end()) {
                unsigned int face_id = face_elements_hash[element_hash];
                face_to_element[face_id] = elem_i;
            }
        }


        // Verifying if all the faces are related with its internal elements
        bool all_faces_found = true;
        if( std::find(face_to_element.begin(), face_to_element.end(), -1) != face_to_element.end() )
            all_faces_found = false;


        if (all_faces_found){
            return ;
        }
    }
    
    std::cout << "Face to element relation wasn't calculated correctly, exiting..." << std::endl;
    exit(1);
}


void Mesh::write_glvis(string filename, MeshIODataAppended* info)
{
    if(filename.find(".mesh") != std::string::npos)
    {

        glvisWriter writer;
        if(writer.open(filename))
        {
            writer.write_mesh(*this);
            writer.close();
        }
    }
}

void Mesh::write_vtk(string basename, MeshIODataAppended* info)
{
   
    vtkWriter writer;
    if(writer.open(basename))
    {
        writer.write_mesh(*this);
        writer.close();
    }
    
}