
#include "gmsh_io.h"
#include "mesh_helper.h"

#include <iostream>
#include <fstream>

GmshIO::GmshIO()
{}

GmshIO::~GmshIO()
{

}

void GmshIO::read(const std::string &filename, Mesh &mesh)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return;
    }
}

void GmshIO::write(const std::string &filename, Mesh &mesh)
{
    std::ofstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return;
    }

    auto& physical_region = mesh.get_physical_map();
    auto& conn = mesh.get_connectivity_vector();
    auto& offset = mesh.get_offset_vector();
    auto& type = mesh.get_element_type_vector();
    auto& physical_tag = mesh.get_element_physical_tag_vector();

    // Write the header
    file << "$MeshFormat\n";
    file << "2.2 0 8\n";
    file << "$EndMeshFormat\n";

    // Write the Physical Names
    file << "$PhysicalNames\n";
    file << physical_region.size() << "\n";
    for (auto &region : physical_region)
    {
        file  << region.second.first << " " << region.first << " " << region.second.second << "\n";
    }
    file << "$EndPhysicalNames\n";

    // Write the Nodes
    file << "$Nodes\n";
    file << mesh.get_n_nodes() << "\n";
    for (int i = 0; i < mesh.get_n_nodes(); i++)
    {
        auto& coords = mesh.get_coordinate_vector();
        file << (i +1) << " " << coords[i*3+0] << " " << coords[i*3+1] << " " << coords[i*3+2] << "\n";
    }
    file << "$EndNodes\n";

    unsigned int nse = mesh.get_n_surface_elements();
    unsigned int ne  = mesh.get_n_elements();
    // Write the Elements
    file << "$Elements\n";
    file << nse+ne << "\n";
    // loop sobre os elementos de superficie
    for (int i = 0; i < nse; i++)
    {
        
        file << (i + 1) << " " << MeshHelper::VtkIdToGmshId[type[i]] << " " << 1 <<" " << physical_tag[i] << " ";
        for (int j = offset[i]; j < offset[i+1]; j++)
        {
            file << (conn[j] + 1) << " ";
        }
        file << "\n";
    }

    // loop sobre os elementos de volume
    for (int i = nse; i < nse+ne; i++)
    {
        file << (i + 1) << " " << MeshHelper::VtkIdToGmshId[type[i]] << " " << 1 <<" " << physical_tag[i] << " " ;
        for (int j = offset[i]; j < offset[i+1]; j++)
        {
            file << (conn[j] + 1) << " ";
        }
        file << "\n";
    }
    file << "$EndElements\n";

    file.close();
    
}

