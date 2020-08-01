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

vector<int> Mesh::getConn()
{
    return this->conn;
}

vector<int> Mesh::getOffset()
{
    return this->offset;
}

vector<unsigned short> Mesh::getType()
{
    return this->type;
}

vector<int> Mesh::get_Physical_tag()
{
    return this->physical_tag;
}

int* Mesh::get_Mesh_coloring_internal()
{
    return this->mesh_coloring_internal;
}

int Mesh::get_N_internal_colors()
{
    return this->n_internal_colors;
}

map<int, physical_data_t> Mesh::getPhysical_map()
{
    return this->physical_map;
}

int Mesh::get_N_face_elements()
{
    return this->n_face_elements;
}

int Mesh::get_N_elements()
{
    return this->n_elements;
}

int Mesh::get_N_nodes()
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

void Mesh::setCoord(vector<double> coord)
{
    this->coord = coord;
}

void Mesh::setConn(vector<int> conn)
{
    this->conn = conn;
}

void Mesh::setConnPosition(unsigned int value, int position)
{
    this->conn[position] = value;
}

void Mesh::setOffset(vector<int> offset)
{
    this->offset = offset;
}

void Mesh::setOffsetPosition(unsigned int value, int position)
{
    this->offset[position] = value;
}

void Mesh::setType(vector<unsigned short> type)
{
    this->type = type;
}

void Mesh::set_Physical_tag(vector<int> physical_tag)
{
    this->physical_tag = physical_tag;
}

void Mesh::set_Mesh_coloring_internal(int* mesh_coloring_internal)
{
    this->mesh_coloring_internal = mesh_coloring_internal;
}

void Mesh::set_N_internal_colors(int n_internal_colors)
{
    this->n_internal_colors = n_internal_colors;
}

void Mesh::set_Physical_map(map<int, physical_data_t> physical_map)
{
    this->physical_map = physical_map;
}

void Mesh::set_N_face_elements(int n_face_elements)
{
    this->n_face_elements = n_face_elements;
}

void Mesh::set_N_elements(int n_elements)
{
    this->n_elements = n_elements;
}

void Mesh::set_N_nodes(int n_nodes)
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

int* Mesh::getElementConn(int element_num)
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

int* Mesh::getElementOffset(int element_num)
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

int* Mesh::getSurfaceElementConn(int element_num)
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

int* Mesh::getSurfaceElementOffset(int element_num)
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

int Mesh::getElementConnSize(int element_num)
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

int Mesh::getSurfaceElementConnSize(int element_num)
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