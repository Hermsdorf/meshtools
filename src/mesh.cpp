#include <iostream>
#include <cassert>
#include "mesh.h" 

using namespace std;

Mesh::Mesh()
{
    this->n_face_elements = 0;
    this->n_elements = 0;
    this->n_nodes = 0; 
    this->dim = 0;
}

Mesh::Mesh(const char* filename)
{
    this->n_face_elements = 0;
    this->n_elements = 0;
    this->n_nodes = 0; 
    this->dim = 0;

    MeshGmshReader(filename);
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
    delete [] this->mesh_coloring_internal;
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

unsigned int Mesh::get_n_internal_colors()
{
    return this->n_internal_colors;
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

std::vector<int>& Mesh::get_physical_tag()
{
    return this->physical_tag;
}

int* Mesh::get_mesh_coloring_internal()
{
    return this->mesh_coloring_internal;
}

std::map<int, physical_data_t> Mesh::get_physical_map()
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

void Mesh::setCoord(std::vector<double> coord)
{
    this->coord = coord;
}

void Mesh::setConn(std::vector<unsigned int> conn)
{
    this->conn = conn;
}

void Mesh::setConnPosition(unsigned int value, unsigned int position)
{
    this->conn[position] = value;
}

void Mesh::setOffset(std::vector<unsigned int> offset)
{
    this->offset = offset;
}

void Mesh::setOffsetPosition(unsigned int value, unsigned int position)
{
    this->offset[position] = value;
}

void Mesh::setType(std::vector<unsigned short> type)
{
    this->type = type;
}

void Mesh::set_physical_tag(std::vector<int> physical_tag)
{
    this->physical_tag = physical_tag;
}

void Mesh::set_mesh_coloring_internal(int* mesh_coloring_internal)
{
    this->mesh_coloring_internal = mesh_coloring_internal;
}

void Mesh::set_n_internal_colors(unsigned int n_internal_colors)
{
    this->n_internal_colors = n_internal_colors;
}

void Mesh::set_physical_map(std::map<int, physical_data_t> physical_map)
{
    this->physical_map = physical_map;
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
