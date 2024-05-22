
#include "mesh_generation.h"


std::unique_ptr<Mesh> MeshGeneration::build_square(unsigned int nx, unsigned int ny, double xmin, double xmax, double ymin, double ymax)
{
    std::unique_ptr<Mesh> mesh(new Mesh());

    mesh->set_mesh_dimension(2);

    double dx = (xmax - xmin) / nx;
    double dy = (ymax - ymin) / ny;

    unsigned int n_points = (nx+1)*(ny+1);

    auto &coords = mesh->get_coordinate_vector();
    auto &offset = mesh->get_offset_vector();
    auto &conn   = mesh->get_connectivity_vector();
    auto &types  = mesh->get_element_type_vector();
    auto &tag    = mesh->get_element_physical_tag_vector();
    auto &node_index = mesh->get_node_index_vector();   

    coords.reserve(3*n_points);
    node_index.reserve(n_points);
    offset.reserve(2*nx + 2*ny + nx*ny + 1);
    conn.reserve(4*nx + 4*ny + 4*nx*ny);
    types.reserve(2*nx + 2*ny + nx*ny);
    tag.reserve(2*nx + 2*ny + nx*ny);
    
    unsigned int index = 0;

    for(unsigned int j = 0; j < ny+1; j++)
    {
        for(unsigned int i = 0; i < nx+1; i++)
        {
            double x = xmin + i*dx;
            double y = ymin + j*dy;
            double z = 0.0; 
            coords.emplace_back(x);
            coords.emplace_back(y);
            coords.emplace_back(z);
            node_index.emplace_back(index);
            index++;
        }
    }

    // create surface elements
    // bottom elements
    unsigned int _n_surface_elements = 0;
    unsigned int _n_elements = 0;
    unsigned int offset_counter = 0;
    offset.emplace_back(0);
    for(unsigned int i = 0; i < nx; i++)
    {
        unsigned int n0 = i;
        unsigned int n1 = i + 1;
        conn.emplace_back(n0);
        conn.emplace_back(n1);
        offset_counter+=2;
        offset.emplace_back(offset_counter);
        types.emplace_back(EDGE2);
        tag.emplace_back(1);
    }

    // right elements
    for(unsigned int j = 0; j < ny; j++)
    {
        unsigned int n0 = j*(nx+1) + nx;
        unsigned int n1 = (j+1)*(nx+1) + nx;
        conn.emplace_back(n0);
        conn.emplace_back(n1);
        offset_counter+=2;
        offset.emplace_back(offset_counter);
        types.emplace_back(EDGE2);
        tag.emplace_back(2);
        _n_surface_elements++;

    }

    mesh->set_n_nodes(n_points);

    // top elements
    for(unsigned int i = nx; i > 0; i--)
    {
        unsigned int n0 = (ny+1)*nx + i;
        unsigned int n1 = (ny+1)*nx + i - 1;
        conn.emplace_back(n0);
        conn.emplace_back(n1);
        offset_counter+=2;
        offset.emplace_back(offset_counter);
        types.emplace_back(EDGE2);
        tag.emplace_back(3);
        _n_surface_elements++;
    }

    // left elements
    for(unsigned int j = ny; j > 0; j--)
    {
        unsigned int n0 = j*(nx+1);
        unsigned int n1 = (j-1)*(nx+1);
        conn.emplace_back(n0);
        conn.emplace_back(n1);
        offset_counter+=2;
        offset.emplace_back(offset_counter);
        types.emplace_back(EDGE2);
        tag.emplace_back(4);
        _n_surface_elements++;
    }

    mesh->set_n_surface_elements(_n_surface_elements);
    

    for(unsigned int j = 0; j < ny; j++)
    {
        for(unsigned int i = 0; i < nx; i++)
        {
            unsigned int n0 = j*(nx+1) + i;
            unsigned int n1 = j*(nx+1) + i + 1;
            unsigned int n2 = (j+1)*(nx+1) + i + 1;
            unsigned int n3 = (j+1)*(nx+1) + i;

            conn.emplace_back(n0);
            conn.emplace_back(n1);
            conn.emplace_back(n2);
            conn.emplace_back(n3);

            offset_counter+=4;
            offset.emplace_back(offset_counter);
            types.emplace_back(QUAD4);
            tag.emplace_back(5);
            _n_elements++;

        }
    }

    mesh->set_n_elements(_n_elements);

    auto & physical_map = mesh->get_physical_map();
    physical_map[1] = std::make_pair(1, "bottom");
    physical_map[2] = std::make_pair(1, "right");
    physical_map[3] = std::make_pair(1, "top");
    physical_map[4] = std::make_pair(1, "left");
    physical_map[5] = std::make_pair(1, "surface");

    mesh->set_mesh_element_type(QUAD4);
    mesh->set_surface_mesh_element_type(EDGE2);

    mesh->process_face_to_element();

    return mesh;

}



