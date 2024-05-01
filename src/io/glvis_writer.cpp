
#include "glvis_writer.h"
#include "mesh_helper.h"


glvisWriter::glvisWriter()
{}

glvisWriter::~glvisWriter()
{
    close();
}

bool glvisWriter::open(std::string name)
{
    if(name.find(".mesh") == std::string::npos)
    {
        std::cout << "[glvisWriter]: Mesh format unknown" << std::endl;
        return false;
    }

    fout.open(name);
    if(!fout.is_open()) return false;
    fout << "MFEM mesh v1.0\n" << std::endl;
    return true;
}

void glvisWriter::write_mesh(Mesh& mesh)
{
    if(!fout.is_open()) return;
    int dim             = mesh.get_mesh_dimension();
    unsigned int nnodes = mesh.get_n_nodes();
    unsigned int n_elem = mesh.get_n_elements();
    unsigned int n_surface_element = mesh.get_n_surface_elements();

    //# Space dimension:
    fout << "dimension" << std::endl;
    fout <<  dim << std::endl;

    //# Mesh elements,
    fout << "\nelements" << std::endl;
    fout <<  n_elem << std::endl;
    for(int i = 0; i < n_elem; i++)
    {
        std::vector<unsigned int> conn;
        mesh.get_element_connectivity(i,conn);
        unsigned int elem_tag  = mesh.get_element_physical_tag(i);
        unsigned int elem_type = mesh.get_element_type(i);
        fout << elem_tag << " " << MeshHelper::VtkIdToGmshId[elem_type];
        for(int j = 0; j < conn.size(); j++)
            fout << " " << conn[j];
        fout << std::endl;
    }

    //# Mesh faces/edges on the boundary,/
    fout << "\nboundary"       << std::endl;
    fout <<  n_surface_element << std::endl;
    for(int i = 0; i < n_surface_element; i++)
    {
        std::vector<unsigned int> conn;
        mesh.get_surface_element_connectivity(i,conn);
        unsigned int elem_tag  = mesh.get_surface_element_physical_tag(i);
        unsigned int elem_type = mesh.get_surface_element_type(i);
        fout << elem_tag << " " << MeshHelper::VtkIdToGmshId[elem_type];
        for(int j = 0; j < conn.size(); j++)
            fout << " " << conn[j];
        fout << std::endl;
    }

    // # Vertex coordinates
    fout << "\nvertices"       << std::endl;
    fout <<  nnodes << std::endl;
    fout <<  dim << std::endl;
    for(int n = 0; n < nnodes; n++)
    {
        for(int d = 0; d < dim; d++)
            fout << mesh.get_coordinate_vector()[n*3+d] << " ";
        fout << std::endl;
    }

}

void   glvisWriter::close()
{
    if(fout)  fout.close();
}