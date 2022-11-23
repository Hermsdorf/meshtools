#include <iostream>
#include <cassert>
#include <set>
#include "mesh.h" 

using namespace std;

Mesh::Mesh()
{
    this->n_face_elements = 0;
    this->n_elements      = 0;
    this->n_nodes         = 0; 
    this->dim             = 0;
    //this->mesh_coloring_internal = nullptr;
    this->n_colors = 0;
}

Mesh::Mesh(const char* filename)
{
    this->n_face_elements = 0;
    this->n_elements = 0;
    this->n_nodes = 0; 
    this->dim = 0;
    //this->mesh_coloring_internal = nullptr;
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
    //this->mesh_coloring_internal = nullptr;
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

    //if(this->mesh_coloring_internal)
    //    delete [] this->mesh_coloring_internal;
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
    //this->coord = coord;
}

void Mesh::setConn(std::vector<unsigned int> &conn)
{
    //this->conn = conn;
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
    //this->offset = offset;
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
int Mesh::getElemContourNNodes(int type)
{
    switch (type)
    {
        case 1: return 1; // EDGE2
        case 2: return 2; // TRI3
        case 3: return 2; // QUAD4
        case 4: return 3; // TET4
        case 5: return 4; // HEX8
        default: return -1;
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


