#include <iostream>
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
    delete [] this->mesh_coloring_internal;
}

vector<double> Mesh::getCoord()
{
    return this->coord;
}

vector<unsigned int> Mesh::getConn()
{
    return this->conn;
}

vector<unsigned int> Mesh::getOffset()
{
    return this->offset;
}

vector<unsigned short> Mesh::getType()
{
    return this->type;
}

vector<int> Mesh::get_physical_tag()
{
    return this->physical_tag;
}

int* Mesh::get_mesh_coloring_internal()
{
    return this->mesh_coloring_internal;
}

unsigned int Mesh::get_n_internal_colors()
{
    return this->n_internal_colors;
}

map<int, physical_data_t> Mesh::get_physical_map()
{
    return this->physical_map;
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

int Mesh::getDim()
{
    return this->dim;
}

string Mesh::getFilename()
{
    return this->filename;
}

unsigned short Mesh::getElementType(unsigned int element_num)
{
    return this->type[element_num + this->n_face_elements];
} // [0, n_elements)

unsigned short Mesh::getSurfaceElementType(unsigned int element_num)
{
    return this->type[element_num];
} // [0, n_face_elements)

void Mesh::setCoord(vector<double> coord)
{
    this->coord = coord;
}

void Mesh::setConn(vector<unsigned int> conn)
{
    this->conn = conn;
}

void Mesh::setConnPosition(unsigned int value, unsigned int position)
{
    this->conn[position] = value;
}

void Mesh::setOffset(vector<unsigned int> offset)
{
    this->offset = offset;
}

void Mesh::setOffsetPosition(unsigned int value, unsigned int position)
{
    this->offset[position] = value;
}

void Mesh::setType(vector<unsigned short> type)
{
    this->type = type;
}

void Mesh::set_physical_tag(vector<int> physical_tag)
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

void Mesh::set_physical_map(map<int, physical_data_t> physical_map)
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

void Mesh::setFilename(string filename)
{
    this->filename = filename;
}

unsigned int* Mesh::getElementConn(unsigned int element_num)
{
    if(element_num < this->n_elements)
    {
        return &this->conn[this->offset[element_num + this->n_face_elements]];
    }
    else
    {
        cout << "ERROR getElementConn: element number = " << element_num << " >= n_elements"  << endl;
        exit(1);
    }
    
} // element_num = [0, n_elements);

unsigned int* Mesh::getElementOffset(unsigned int element_num)
{
    if(element_num <= this->n_elements)
    {
        return &this->offset[element_num + this->n_face_elements];
    }
    else
    {
        cout << "ERROR getElementOffset: element number = " << element_num << " > n_elements"  << endl;
        exit(1);
    }
    
} 

unsigned int* Mesh::getSurfaceElementConn(unsigned int element_num)
{
    if(element_num < this->n_face_elements)
    {
        return &this->conn[this->offset[element_num]];
    }
    else
    {
        cout << "ERROR getSurfaceElementConn: element number = " << element_num << " >= n_elements"  << endl;
        exit(1);
    }
    
} // element_num = [0, n_surface_elements);

unsigned int* Mesh::getSurfaceElementOffset(unsigned int element_num)
{
    if(element_num < this->n_face_elements)
    {
        return &this->offset[element_num];
    }
    else
    {
        cout << "ERROR getSurfaceElementOffset: element number = " << element_num << " >= n_elements"  << endl;
        exit(1);
    }
}

unsigned int Mesh::getElementConnSize(unsigned int element_num)
{
    if(element_num < this->n_elements)
    {
        return (this->offset[this->n_face_elements + element_num + 1] - this->offset[this->n_face_elements + element_num]);
    }
    else
    {
        cout << "ERROR getElementConnSize: element number = " << element_num << " >= n_elements"  << endl;
    }
    
}

unsigned int Mesh::getSurfaceElementConnSize(unsigned int element_num)
{
    if(element_num < this->n_face_elements)
    {
        return (this->offset[element_num + 1] - this->offset[element_num]);
    }
    else
    {
        cout << "ERROR getSurfaceElementConnSize: element number = " << element_num << " >= n_elements"  << endl;
        exit(1);
    }
}