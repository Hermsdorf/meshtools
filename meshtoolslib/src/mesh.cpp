#include <iostream>
#include <cassert>
#include <set>
#include <unordered_map>
#include "mesh.h" 

using namespace std;

//unsigned int edge2_faces[2][1] = {{0},{1}};

unsigned int edge2_faces[2] = {0,1};

//unsigned int quad4_faces[4][2] = {{0,1},{1,2},{2,3},{3,0}};
unsigned int quad4_faces[8] = {0,1,1,2,2,3,3,0}; // 2D


//unsigned int tri3_faces[3][2]  = {{0,1},{1,2},{2,0}};
unsigned int tri3_faces[6]  = {0,1,1,2,2,0}; // 2D

//unsigned int tet4_faces[4][3]  = {{0,2,1},{0,3,2},{0,1,3},{1,2,3}};
unsigned int tet4_faces[12]  = {0,2,1,0,3,2,0,1,3,1,2,3}; // 3D

//unsigned int hex8_faces[6][4]  = {{0,1,2,3},{4,5,6,7},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7}};
unsigned int hex8_faces[24]  = {0,1,2,3,4,5,6,7,0,1,5,4,1,2,6,5,2,3,7,6,3,0,4,7}; // 3D

Mesh::Mesh()
{
    this->n_face_elements = 0;
    this->n_elements      = 0;
    this->n_nodes         = 0; 
    this->dim             = 0;
    this->n_colors = 0;
}

Mesh::Mesh(const char* filename)
{
    this->n_face_elements = 0;
    this->n_elements = 0;
    this->n_nodes = 0; 
    this->dim = 0;
    this->n_colors = 0;
    MeshGmshReader(filename);
}

Mesh::Mesh(std::string filename)
{
    const char * filename_converted = filename.c_str();

    this->n_face_elements = 0;
    this->n_elements = 0;
    this->n_nodes = 0; 
    this->dim = 0;
    this->n_colors = 0;
    MeshGmshReader(filename_converted);
}

Mesh::~Mesh()
{
    this->conn.clear();
    this->coord.clear();
    this->offset.clear();
    this->type.clear();
    this->physical_tag.clear();
    this->physical_map.clear();
    this->filename.clear();
    this->coloring.clear();
    this->face_to_element.clear();
}

unsigned int Mesh::get_n_face_elements()
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

std::vector<double>& Mesh::getCoord()
{
    return this->coord;
}

std::vector<unsigned int>& Mesh::getConn()
{
    return this->conn;
}

std::vector<unsigned int>& Mesh::getOffset()
{
    return this->offset;
}

std::vector<unsigned short>& Mesh::getType()
{
    return this->type;
}

std::vector<int>& Mesh::getPhysicalTag()
{
    return this->physical_tag;
}

std::vector<unsigned int>& Mesh::getColoring()
{
    return this->coloring;
}

std::map<int, physical_data_t>& Mesh::getPhysicalMap()
{
    return this->physical_map;
}

int Mesh::getDim()
{
    return this->dim;
}

std::string Mesh::getFilename()
{
    return this->filename;
}

unsigned short Mesh::getElementType(unsigned int element_num)
{
    return this->type[element_num + this->n_face_elements];
} 

unsigned short Mesh::getSurfaceElementType(unsigned int element_num)
{
    return this->type[element_num];
} 

void Mesh::setCoord(std::vector<double> &coord)
{
    this->coord.resize(coord.size());
    std::copy(coord.begin(),coord.end(), this->coord.begin());
}

void Mesh::setConn(std::vector<unsigned int> &conn)
{
    this->conn.resize(conn.size());
    std::copy(conn.begin(),conn.end(), this->conn.begin());
}

void Mesh::setConnPosition(unsigned int value, unsigned int position)
{
    this->conn[position] = value;
}

void Mesh::setOffset(std::vector<unsigned int> &offset)
{
    this->offset.resize(offset.size());
    std::copy(offset.begin(), offset.end(), this->offset.begin());
}

void Mesh::setOffsetPosition(unsigned int value, unsigned int position)
{
    this->offset[position] = value;
}

void Mesh::setType(std::vector<unsigned short> &type)
{
    //this->type = type;
    this->type.resize(type.size());
    std::copy(type.begin(), type.end(), this->type.begin());
    
}

void Mesh::setTypePosition(unsigned short value, unsigned int position)
{
    this->type[position] = value;
}

void Mesh::set_physical_tag(std::vector<int> &physical_tag)
{
    //this->physical_tag = physical_tag;
    this->physical_tag.resize(physical_tag.size());
    std::copy(physical_tag.begin(), physical_tag.end(), this->physical_tag.begin());
}

void Mesh::set_n_colors(unsigned int n_colors)
{
    this->n_colors = n_colors;
}

void Mesh::set_physical_map(std::map<int, physical_data_t> &physical_map)
{
    this->physical_map = physical_map;
    //physical_map.
    //this->physical_map.resize(physical_map.size());
    //std::copy(physical_map.begin(), physical_map.end(), this->physical_map.begin());
}

void Mesh::set_n_face_elements(unsigned int n_face_elements)
{
    this->n_face_elements = n_face_elements;
}

void Mesh::set_n_elements(unsigned int n_elements)
{
    this->n_elements = n_elements;
}

void Mesh::set_n_nodes(unsigned int n_nodes)
{
    this->n_nodes = n_nodes;
}


void Mesh::setDim(int dim)
{
    this->dim = dim;
}

void Mesh::setFilename(std::string filename)
{
    this->filename = filename;
}

unsigned int* Mesh::getElementConn(unsigned int element_num)
{

    assert(element_num < this->n_elements);
    return &this->conn[this->offset[element_num + this->n_face_elements]];

}

unsigned int* Mesh::getElementOffset(unsigned int element_num)
{

    assert(element_num <= this->n_elements);
    return &this->offset[element_num + this->n_face_elements];
    
} 

unsigned int* Mesh::getSurfaceElementConn(unsigned int element_num)
{

    assert(element_num < this->n_face_elements || (this->n_face_elements == 0 && element_num ==0));
    return &this->conn[this->offset[element_num]];

} 

unsigned int* Mesh::getSurfaceElementOffset(unsigned int element_num)
{

    assert(element_num < this->n_face_elements || (this->n_face_elements == 0 && element_num ==0));
    return &this->offset[element_num];

}

unsigned int Mesh::getElementConnSize(unsigned int element_num)
{

    assert(element_num < this->n_elements);
    return (this->offset[this->n_face_elements + element_num + 1] - this->offset[this->n_face_elements + element_num]);
    
}

unsigned int Mesh::getSurfaceElementConnSize(unsigned int element_num)
{

    assert(element_num < this->n_face_elements);
    return (this->offset[element_num + 1] - this->offset[element_num]);

}

void Mesh::extract_boundary_nodes(std::vector<int>& tag)
{  
    for(auto it = physical_map.begin(); it != physical_map.end(); ++it)
    {
        if(it->second.first != (dim-1)) continue;

        std::set<int>  node_on_boundary;

        for(int iel=0; iel < this->n_face_elements; iel++)
        {
            if(tag[iel] == it->first)
            {
                unsigned int connsize = getSurfaceElementConnSize(iel); 
                unsigned int* conn     = getSurfaceElementConn(iel); 
                for(int ino = 0; ino < connsize; ++ino)
                    node_on_boundary.insert(conn[ino]);
            }
        }
    }
}

void Mesh::get_element_coordinates(int element_id, std::vector<Point> &coordinates)
{
    int nnoel                = this->getElementConnSize(element_id);
    const unsigned int* conn = this->getElementConn(element_id);

    coordinates.resize(nnoel);

    for(int ino = 0; ino < nnoel; ++ino)
    {
        coordinates[ino](0) = this->coord[conn[ino]*3+0];
        coordinates[ino](1) = this->coord[conn[ino]*3+1];
        coordinates[ino](2) = this->coord[conn[ino]*3+2];
    }
}

void Mesh::get_element_connectivity(int element_id, std::vector<unsigned int> &connectivity)
{
    int nnoel                = this->getElementConnSize(element_id);
    const unsigned int* conn = this->getElementConn(element_id);

    connectivity.resize(nnoel);

    for(int ino = 0; ino < nnoel; ++ino)
        connectivity[ino] = conn[ino];
}

void Mesh::getElement(unsigned int elemen_it, Element& elem)
{
    get_element_connectivity(elemen_it,elem._conn);
    get_element_coordinates(elemen_it,elem._coords);
    elem._type= this->getElementType(elemen_it);
    elem._tag = this->getElementTag(elemen_it);
}

unsigned int Mesh::getElementConnectivitySize()
{
    return this->coord.size() - this->offset[this->n_face_elements];
}

unsigned int Mesh::getBoundaryElementConnectivitySize()
{
    return this->offset[this->n_face_elements];
}

unsigned int* Mesh::getElementConnectivityData()
{
    unsigned int nfo = this->offset[this->n_face_elements];
    return &this->conn[nfo];
}

unsigned int* Mesh::getBoundaryElementsConnectivityData()
{
    return &this->conn[0];
}

double * Mesh::getCoordinatesData()
{
    return &this->coord[0];
}

// By the element type this method returns the number of nodes at the element's faces
int Mesh::getVTKElemContourNNodes(int vtk_type)
{
    switch (vtk_type)
    {
        case 3: return 2; // EDGE2
        case 5: return 2; // TRI3
        case 9: return 2; // QUAD4
        case 10: return 3; // TET4
        case 12: return 4; // HEX8
        default: return -1;
        break;
    }
}

// By the element type this method returns the number of element's faces
int Mesh::getVTKElemContourNFaces(int vtk_type)
{
    switch (vtk_type)
    {
        case 3: return 2; // EDGE2
        case 5: return 3; // TRI3
        case 9: return 4; // QUAD4
        case 10: return 4; // TET4
        case 12: return 6; // HEX8
        default: return -1;
        break;
    }
}

// By the element type this method returns the number of element's faces
unsigned int* getVTKElemConnSequence(int vtk_type)
{
    switch (vtk_type)
    {
        case 3:  return edge2_faces; // EDGE2
        case 5:  return tri3_faces; // TRI3
        case 9:  return quad4_faces; // QUAD4
        case 10: return tet4_faces; // TET4
        case 12: return hex8_faces; // HEX8
        default: return nullptr;
        break;
    }
}


int Mesh::getGmshElemNNodes(int type)
{
    switch (type)
    {
        case 1: return 2; // EDGE2
        case 2: return 3; // TRI3
        case 3: return 4; // QUAD4
        case 4: return 4; // TET4
        case 5: return 8; // HEX8
        case 15: return 1;
        default: return -1;
        break;
    }
}

int Mesh::getGmshElemTypeDim(int type)
{
    switch (type)
    {
        case 1: return 1; // EDGE2
        case 2: return 2; // TRI3
        case 3: return 2; // QUAD4
        case 4: return 3; // TET4
        case 5: return 3; // HEX8
        case 15: return 0;
        default: return -1;
        break;
    }
}

// Hash function to fill face_to_element array
// Referencia: cantor pairing function
//  http://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
unsigned long cantor_pairing(unsigned int a, unsigned int b) {
    unsigned long hash = (a + b + 1) * (a + b) / 2 + b; 
    return hash;
}

void Mesh::process_face_to_element()
{
    int n_face_elements = this->get_n_face_elements();
    int n_elements = this->get_n_elements();
    int dim = this->getDim();

    std::vector<int> face_to_element(n_face_elements, -1); // -1 means no element yet

    unordered_map<unsigned long, int> face_elements_hash;

    // Calculating hash to each surface element
    for (int i = 0; i < n_face_elements; i++) {
        unsigned short surf_element_nnodes = this->getSurfaceElementConnSize(i);
        unsigned int* surf_element_conn = this->getSurfaceElementConn(i);

        std::vector<unsigned int> conn_tmp(surf_element_nnodes);
        for (int j = 0; j < surf_element_nnodes; j++) {
            conn_tmp[j] = surf_element_conn[j];
        }

        std::sort(conn_tmp.begin(), conn_tmp.end());

        unsigned long element_hash = conn_tmp[0];
        for (unsigned short conn_i = 1 ; conn_i < surf_element_nnodes ; conn_i++){
            element_hash = cantor_pairing(element_hash, conn_tmp[conn_i]);
        }

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


    // Filling face_to_element array
    for(unsigned int elem_i = 0; elem_i < n_elements; elem_i++) {
        unsigned short element_nnodes = this->getElementConnSize(elem_i);
        unsigned int*  element_conn = this->getElementConn(elem_i);
        unsigned short element_type = this->getElementType(elem_i);

        std::vector<unsigned int> element_conn_vec(element_nnodes);
        for (int j = 0; j < element_nnodes; j++) 
            element_conn_vec[j] = element_conn[j];

        //std::sort(element_conn_vec.begin(), element_conn_vec.end());

        // Getting element's faces
        int n_faces = getVTKElemContourNFaces(element_type);
        for(int face_i = 0; face_i < n_faces; face_i++)
        {
            // getting face nodes
            int  n_face_nodes = getVTKElemContourNNodes(element_type);
            auto face_map = getVTKElemConnSequence(element_type);

            std::vector<unsigned int> face_nodes(n_face_nodes);
            for(int face_node_i = 0; face_node_i < n_face_nodes ; face_node_i++)
            {
                int local_node = face_map[face_i*n_face_nodes + face_node_i];
                face_nodes[face_node_i] = element_conn_vec[local_node];
            }

            std::sort(face_nodes.begin(), face_nodes.end());
            unsigned long element_hash = face_nodes[0];
            for (unsigned short conn_i = 1 ; conn_i < face_nodes.size() ; conn_i++)
                element_hash = cantor_pairing(element_hash, face_nodes[conn_i]);

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
            this->setFaceToElement(face_to_element);
            return ;
        }
    }
    std::cout << "Face to element relation wasn't calculated correctly, exiting..." << std::endl;
    exit(1);
}



