
#include "mesh.h"
#include "hash.h"
#include "mesh_helper.h"
#include "mesh_refinement.h"

MeshRefinement::MeshRefinement(ParallelMesh &mesh): mesh(mesh)
{
}

MeshRefinement::~MeshRefinement()
{
}

void MeshRefinement::uniform_refinement(unsigned int n_refinements)
{
    
    // Clear the maps
    edge_map.clear();
    face_map.clear();
    cell_map.clear();

    unsigned int nse = mesh.get_n_surface_elements();
    unsigned int ne  = mesh.get_n_elements();

    // Get the mesh data
    auto coords       = mesh.get_coordinate_vector();
    auto conn         = mesh.get_connectivity_vector();
    auto offset       = mesh.get_offset_vector();
    auto type         = mesh.get_element_type_vector();
    auto physical_tag = mesh.get_element_physical_tag_vector();


    unsigned int n_nodes = mesh.get_n_nodes();
    unsigned int new_n_elements;

    unsigned int n_new_surface_elements = 0;
    unsigned int n_new_elements         = 0;

    // New mesh data
    std::vector<double> new_coords;
    std::vector<unsigned int> new_conn;
    std::vector<unsigned int> new_offset;
    std::vector<unsigned short> new_type;
    std::vector<int> new_physical_tag;

    std::vector<unsigned int> new_neighbors_processors;
    std::vector<unsigned int> new_shared_nodes;
    std::vector<unsigned int> new_shared_nodes_offset;

    int nnoel         = MeshHelper::VtkIdToNumberOfNodes[mesh.get_mesh_element_type()];
    int nnoel_surface = MeshHelper::VtkIdToNumberOfNodes[mesh.get_surface_mesh_element_type()];
    unsigned int size_conn = nnoel_surface*nse + nnoel*ne; 
    
    // reserve memory
    coords.reserve(2*n_nodes);
    new_conn.reserve(size_conn);
    new_offset.reserve(8*ne+4*nse);
    new_type.reserve(8*ne+4*nse);
    new_physical_tag.reserve(8*ne+4*nse);

    new_neighbors_processors.reserve(mesh.get_neighbors_processors_vector().size());
    new_shared_nodes.reserve(mesh.get_shared_nodes_vector().size());
    new_shared_nodes_offset.reserve(mesh.get_shared_nodes_offset_vector().size());
    
    // copia os vertices
    new_coords.insert(new_coords.end(), coords.begin(), coords.end());

    // Build the shared processor per node map
    build_shared_processor_per_node_map(mesh);

    new_offset.emplace_back(0);
    unsigned int offset_count = 0;

    for(int i = 0; i < nse; i++)
    {
        std::vector<unsigned int> element_conn;
        std::vector<unsigned int> refine_conn;
        unsigned int n_count_elements = 0;
        mesh.get_surface_element_connectivity(i, element_conn);
        switch (type[i])
        {
        case EDGE2:
            edge_refinement_template(new_coords, element_conn, refine_conn, n_nodes, n_count_elements);  
            break;
        case TRI3:
            triangle_refinement_template(new_coords, element_conn, refine_conn, n_nodes, n_count_elements);
            break;
        case QUAD4:
            quad_refinement_template(new_coords, element_conn, refine_conn, n_nodes, n_count_elements);
            break;
        default:
            break;
        }
        parent2child(offset_count, type[i], physical_tag[i], MeshHelper::VtkIdToNumberOfNodes[type[i]], n_count_elements, refine_conn, new_conn, new_offset, new_type, new_physical_tag);
        n_new_surface_elements += n_count_elements;
    }

    for(int i = 0; i < ne; i++)
    {
        std::vector<unsigned int> element_conn;
        std::vector<unsigned int> refine_conn;
        unsigned int n_count_elements = 0;
        mesh.get_surface_element_connectivity(i, element_conn);
        switch (type[i])
        {
        case TRI3:
            edge_refinement_template(new_coords, element_conn, refine_conn, n_nodes, n_count_elements);  
            break;
        case QUAD4:
            triangle_refinement_template(new_coords, element_conn, refine_conn, n_nodes, n_count_elements);
            break;
        case TET4:
            tetrahedron_refinement_template(new_coords, element_conn, refine_conn, n_nodes, n_count_elements);
            break;
        default:
            break;
        }
        parent2child(offset_count, type[i], physical_tag[i], MeshHelper::VtkIdToNumberOfNodes[type[i]], n_count_elements, refine_conn, new_conn, new_offset, new_type, new_physical_tag);
        n_new_elements += n_count_elements;
    }

    mesh.set_n_nodes(n_nodes);
    mesh.set_n_elements(n_new_elements);
    mesh.set_n_surface_elements(n_new_surface_elements);
    coords.swap(new_coords);
    conn.swap(new_conn);
    offset.swap(new_offset);
    type.swap(new_type);
    physical_tag.swap(new_physical_tag);
    
    // update the mesh arrays
    //update_mesh_arrays(mesh, new_conn, new_offset, new_type, new_physical_tag);
    rebuild_communication_map();

}



/*

    Input:  edge_vertices: the vertices of the edge
            edge_conn:     the connectivity of the edge
            edges_conn:    the connectivity of the edges

      v0                v1
      +------------------+


     v0        v2        v1
      +--------+---------+

*/

unsigned int MeshRefinement::edge_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &edge_conn)
{
    std::sort(edge_conn.begin(), edge_conn.end());
    unsigned long hash = compute_hash(edge_conn.size(), edge_conn.data());
    // Check if the edge has been refined
    if (edge_map.find(hash) == edge_map.end())
    {
    
        double x0 = coords[3*edge_conn[0]  ];
        double y0 = coords[3*edge_conn[0]+1];
        double z0 = coords[3*edge_conn[0]+2];

        double x1 = coords[3*edge_conn[1]  ];
        double y1 = coords[3*edge_conn[1]+1];
        double z1 = coords[3*edge_conn[1]+2];

        // Compute the new vertex
        double x = 0.5*(x0 + x1);
        double y = 0.5*(y0 + y1);
        double z = 0.5*(z0 + z1); 
    
        // Add the new vertex
        coords.push_back(x);
        coords.push_back(y);
        coords.push_back(z);
    
        // Add the new vertex to the edge map
        edge_map[hash] = n_nodes;
        n_nodes++;
    
    }

    return edge_map[hash];

}

unsigned int MeshRefinement::face_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn)
{

    std::sort(face_conn.begin(), face_conn.end());
    unsigned long hash = compute_hash(face_conn.size(), face_conn.data());
    // Check if the face has been refined
    if (face_map.find(hash) == face_map.end())
    {
        // Compute the new vertex
        double x = 0.0;
        double y = 0.0;
        double z = 0.0; 
    
        for(int i = 0; i < face_conn.size(); i++)
        {
            x += coords[3*face_conn[i]  ];
            y += coords[3*face_conn[i]+1];
            z += coords[3*face_conn[i]+2];
        }

        x /= face_conn.size();
        y /= face_conn.size();
        z /= face_conn.size();
    
        // Add the new vertex
        coords.push_back(x);
        coords.push_back(y);
        coords.push_back(z);
    
        // Add the new vertex to the face map
        face_map[hash] = n_nodes;
        n_nodes++;
    
    }

    return face_map[hash];

}

unsigned int MeshRefinement::cell_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &cell_conn)
{
    std::sort(cell_conn.begin(), cell_conn.end());
    unsigned long hash = compute_hash(cell_conn.size(), cell_conn.data());
    // Check if the face has been refined
    if (cell_map.find(hash) == cell_map.end())
    {
        // Compute the new vertex
        double x = 0.0;
        double y = 0.0;
        double z = 0.0; 
    
        for(int i = 0; i < cell_conn.size(); i++)
        {
            x += coords[3*cell_conn[i]  ];
            y += coords[3*cell_conn[i]+1];
            z += coords[3*cell_conn[i]+2];
        }

        x /= cell_conn.size();
        y /= cell_conn.size();
        z /= cell_conn.size();
    
        // Add the new vertex
        coords.push_back(x);
        coords.push_back(y);
        coords.push_back(z);
    
        // Add the new vertex to the face map
        cell_map[hash] = n_nodes;
        n_nodes++;
    
    }

    return cell_map[hash];
    
}

void MeshRefinement::build_shared_processor_per_node_map()
{

    auto& shared_nodes_offset  = mesh.get_shared_nodes_offset_vector();
    auto& shared_nodes         = mesh.get_shared_nodes_vector();
    auto& neighbors_processors = mesh.get_neighbors_processors_vector();

    for(auto process : neighbors_processors)
    {
        unsigned int offset = shared_nodes_offset[process];
        unsigned int n_shared_nodes = shared_nodes_offset[process+1] - offset;
        for(unsigned int i = 0; i < n_shared_nodes; i++)
        {
            shared_processors_per_node[shared_nodes[offset+i]].insert(process);
        }
    }

}

void MeshRefinement::find_processor_neighbours_edge(std::vector<unsigned int> &conn, unsigned int new_node)
{

    if(shared_processors_per_node[new_node].size() > 0)
    {
        return;
    }
    /*
        find the processors that share the mid point of the edge
        1. Get the processors that share the vertices of the edge   
        2. Get the intersection of the processors that share the vertices of the edge
    */

   // verifica se os vertices do edge compartilham processadores
   if(shared_processors_per_node.find(conn[0]) == shared_processors_per_node.end() || 
      shared_processors_per_node.find(conn[1]) == shared_processors_per_node.end())
   {
       return;
   }
   // Get the processors that share the vertices of the edge
   std::set<unsigned int> processors_v1 = shared_processors_per_node[conn[0]];
   std::set<unsigned int> processors_v2 = shared_processors_per_node[conn[1]];

    // Get the intersection of the processors that share the vertices of the edge
    std::set<unsigned int> processors;
    std::set_intersection(processors_v1.begin(), processors_v1.end(), processors_v2.begin(), processors_v2.end(), std::inserter(processors, processors.begin()));

    // Add the new node to the shared nodes
    shared_processors_per_node[new_node] = processors;
}

void MeshRefinement::find_processor_neighbours_face_3_edges(std::vector<unsigned int> &conn, unsigned int new_node)
{
    if(shared_processors_per_node[new_node].size() > 0)
    {
        return;
    }
    /*
        find the processors that share the mid point of the face
        1. Get the processors that share the vertices of the face   
        2. Get the intersection of the processors that share the vertices of the face
    */

   if(shared_processors_per_node.find(conn[0]) == shared_processors_per_node.end() || 
      shared_processors_per_node.find(conn[1]) == shared_processors_per_node.end() ||
      shared_processors_per_node.find(conn[2]) == shared_processors_per_node.end())
   {
       return;
   }

   // Get the processors that share the vertices of the face
   std::set<unsigned int> processors_v1 = shared_processors_per_node[conn[0]];
   std::set<unsigned int> processors_v2 = shared_processors_per_node[conn[1]];
   std::set<unsigned int> processors_v3 = shared_processors_per_node[conn[2]];

    // Get the intersection of the processors that share the vertices of the face
    std::set<unsigned int> intersect1;
    std::set_intersection(processors_v1.begin(), processors_v1.end(), processors_v2.begin(), processors_v2.end(), std::inserter(intersect1, intersect1.begin()));

    std::set<unsigned int> result;
    std::set_intersection(intersect1.begin(), intersect1.end(), processors_v3.begin(), processors_v3.end(), std::inserter(result, result.begin()));

    // Add the new node to the shared nodes
    shared_processors_per_node[new_node] = result;
}

void MeshRefinement::find_processor_neighbours_face_4_edges(std::vector<unsigned int> &conn, unsigned int new_node)
{
    if(shared_processors_per_node[new_node].size() > 0)
    {
        return;
    }
    /*
        find the processors that share the mid point of the face
        1. Get the processors that share the vertices of the face   
        2. Get the intersection of the processors that share the vertices of the face
    */

    if( shared_processors_per_node.find(conn[0]) == shared_processors_per_node.end() || 
        shared_processors_per_node.find(conn[1]) == shared_processors_per_node.end() ||
        shared_processors_per_node.find(conn[2]) == shared_processors_per_node.end() ||
        shared_processors_per_node.find(conn[3]) == shared_processors_per_node.end())
    {
        return;
    }

   // Get the processors that share the vertices of the face
   std::set<unsigned int> processors_v1 = shared_processors_per_node[conn[0]];
   std::set<unsigned int> processors_v2 = shared_processors_per_node[conn[1]];
   std::set<unsigned int> processors_v3 = shared_processors_per_node[conn[2]];
   std::set<unsigned int> processors_v4 = shared_processors_per_node[conn[3]];

    // Get the intersection of the processors that share the vertices of the face
    std::set<unsigned int> intersect1;
    std::set_intersection(processors_v1.begin(), processors_v1.end(), processors_v2.begin(), processors_v2.end(), std::inserter(intersect1, intersect1.begin()));

    std::set<unsigned int> intersect2;
    std::set_intersection(processors_v3.begin(), processors_v3.end(), processors_v4.begin(), processors_v4.end(), std::inserter(intersect2, intersect2.begin()));

    std::set<unsigned int> result;
    std::set_intersection(intersect1.begin(), intersect1.end(), intersect2.begin(), intersect2.end(), std::inserter(result, result.begin()));

    // Add the new node to the shared nodes
    shared_processors_per_node[new_node] = result;
}


void MeshRefinement::edge_refinement_template(std::vector<double>&      coords, 
                                              std::vector<unsigned int> &conn,
                                              std::vector<unsigned int> &new_conn,
                                              unsigned int              &n_nodes,
                                              unsigned int              &n_edges
                                              )
{
    assert(conn.size() == 2);


    // Get the edge
    unsigned int v0 = conn[0];
    unsigned int v1 = conn[1];
    unsigned int v2 = edge_central_vertice(n_nodes, coords, conn);

    find_processor_neighbours_edge(conn, v2);

    // Add the new vertex to the new connectivity
    new_conn.push_back(v0);
    new_conn.push_back(v2);

    new_conn.push_back(v2);
    new_conn.push_back(v1);

    n_edges = 2;
}

/*
    Triange Refinement Template
    3                     2
    +                     +
    |\                    | \
    | \                   |  \
    |  \                  |   \
    |   \                 |    \
    |    \              5 +-----+ 4     
    |     \               |\    /\
    |      \              | \  /  \
    |       \             |  \/    \
    +-------+             +---+-----+
    1       2             0   3     1

*/


void MeshRefinement::triangle_refinement_template(std::vector<double>&      coords, 
                                                  std::vector<unsigned int> &triangle_conn,
                                                  std::vector<unsigned int> &new_conn,
                                                  unsigned int              &n_nodes,
                                                  unsigned int              &n_triangles
                                              )
{

    assert(triangle_conn.size() == 3);

    std::vector<unsigned int> nodes(6);

    nodes[0] = triangle_conn[0];
    nodes[1] = triangle_conn[1];
    nodes[2] = triangle_conn[2];

    // Get the triangle edges
    for(int edge=0; edge < 3; edge++)
    {
        std::vector<unsigned int> edge_conn(2);
        MeshHelper::triangle_face_connectivity(edge, triangle_conn, edge_conn);
        nodes[3+edge] = edge_central_vertice(n_nodes, coords, edge_conn);
        find_processor_neighbours_edge(edge_conn, nodes[3+edge]);
    }

    // Add the new vertices to the new connectivity
    new_conn.push_back(nodes[0]);
    new_conn.push_back(nodes[3]);
    new_conn.push_back(nodes[5]);

    new_conn.push_back(nodes[3]);
    new_conn.push_back(nodes[1]);
    new_conn.push_back(nodes[4]);

    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[2]);

    new_conn.push_back(nodes[3]);
    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[5]);

    n_triangles = 4;

}

// Quad Refinement Template
/*
    3                 2         3        6         2
    +-----------------+         +--------+---------+
    |                 |         |        |         |
    |                 |         |        | 8       |
    |                 |      7  +--------+---------+ 5
    |                 |         |        |         |
    |                 |         |        |         |
    +-----------------+         +--------+---------+
    0                 1         0        4         1
*/

void MeshRefinement::quad_refinement_template(std::vector<double>&        coords,
                                                std::vector<unsigned int> &quad_conn,
                                                std::vector<unsigned int> &new_conn,
                                                unsigned int              &n_nodes,
                                                unsigned int              &n_quads
                                                )
{

    assert(quad_conn.size() == 4);

    std::vector<unsigned int> nodes(9);

    nodes[0] = quad_conn[0];
    nodes[1] = quad_conn[1];
    nodes[2] = quad_conn[2];
    nodes[3] = quad_conn[3];

    // Get the quad edges
    for(int edge=0; edge < 4; edge++)
    {
        std::vector<unsigned int> edge_conn(2);
        MeshHelper::quad_face_connectivity(edge, quad_conn, edge_conn);
        nodes[4+edge] = edge_central_vertice(n_nodes, coords, edge_conn);
        find_processor_neighbours_edge(edge_conn, nodes[4+edge]);
    }

    nodes[8] = face_central_vertice(n_nodes, coords, quad_conn);
    find_processor_neighbours_face_4_edges(quad_conn, nodes[8]);

    // Add the new vertices to the new connectivity

    new_conn.push_back(nodes[0]);
    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[8]);
    new_conn.push_back(nodes[7]);

    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[1]);
    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[8]);

    new_conn.push_back(nodes[8]);
    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[2]);
    new_conn.push_back(nodes[6]);

    new_conn.push_back(nodes[7]);
    new_conn.push_back(nodes[8]);
    new_conn.push_back(nodes[6]);
    new_conn.push_back(nodes[3]);

    n_quads = 4;
}

void MeshRefinement::tetrahedron_refinement_template(std::vector<double>&   coords, 
                                                  std::vector<unsigned int> &tetra_conn,
                                                  std::vector<unsigned int> &new_conn,
                                                  unsigned int              &n_nodes,
                                                  unsigned int              &n_tets
                                              )
{
    assert(tetra_conn.size() == 4);

    std::vector<unsigned int> nodes(6);

    nodes[0] = tetra_conn[0];
    nodes[1] = tetra_conn[1];
    nodes[2] = tetra_conn[2];
    nodes[3] = tetra_conn[3];

    // Get the tetrahedron edges
    for(int edge=0; edge < 6; edge++)
    {
        std::vector<unsigned int> edge_conn(2);
        MeshHelper::tetrahedron_edge_connectivity(edge, tetra_conn, edge_conn);
        nodes[4+edge] = edge_central_vertice(n_nodes, coords, edge_conn);
        find_processor_neighbours_edge(edge_conn, nodes[4+edge]);
    }

    nodes[10] = cell_central_vertice(n_nodes, coords, tetra_conn);

    // Add the new vertices to the new connectivity 
    new_conn.push_back(nodes[0]);
    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[10]);

    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[1]);
    new_conn.push_back(nodes[6]);
    new_conn.push_back(nodes[10]);

    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[6]);
    new_conn.push_back(nodes[2]);
    new_conn.push_back(nodes[10]);

    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[6]);
    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[10]);

    n_tets = 4;

}



void MeshRefinement::hexahedron_refinement_template(std::vector<double>&   coords, 
                                                  std::vector<unsigned int> &hexa_conn,
                                                  std::vector<unsigned int> &new_conn,
                                                  unsigned int              &n_nodes,
                                                  unsigned int              &n_elements
                                              )
{
    assert(hexa_conn.size() == 8);

    std::vector<unsigned int> nodes(27);

    nodes[0] = hexa_conn[0];
    nodes[1] = hexa_conn[1];
    nodes[2] = hexa_conn[2];
    nodes[3] = hexa_conn[3];
    nodes[4] = hexa_conn[4];
    nodes[5] = hexa_conn[5];
    nodes[6] = hexa_conn[6];
    nodes[7] = hexa_conn[7];

    // Get the hexahedron edges
    for(int edge=0; edge < 12; edge++)
    {
        std::vector<unsigned int> edge_conn(2);
        MeshHelper::hexahedron_edge_connectivity(edge, hexa_conn, edge_conn);
        nodes[8+edge] = edge_central_vertice(n_nodes, coords, edge_conn);
        find_processor_neighbours_edge(edge_conn, nodes[8+edge]);
    }

    // Get the hexahedron faces
    for(int face=0; face < 6; face++)
    {
        std::vector<unsigned int> face_conn(4);
        MeshHelper::hexahedron_face_connectivity(face, hexa_conn, face_conn);
        nodes[20+face] = face_central_vertice(n_nodes, coords, face_conn);
        find_processor_neighbours_face_4_edges(face_conn, nodes[20+face]);
    }

    // Get the hexahedron volume
    nodes[26] = cell_central_vertice(n_nodes, coords, hexa_conn);
/*

    Hexahedron Refinement Template

    3        10        2       15        23          14       7      18         6
    +--------+--------+         +--------+---------+       +--------+---------+
    |        |        |         |        |         |       |        |         |
    |        | 20     |         |        |26       |       |        | 25      |
  11+--------+--------+ 9    24 +--------+---------+22   19+--------+---------+ 17
    |        |        |         |        |         |       |        |         |
    |        |        |         |        |         |       |        |         |
    +--------+--------+         +--------+---------+       +--------+---------+
    0        8        1         12      21         13     4        16        5
    
*/
    // Add the new vertices to the new connectivity
    // TODO: Add the new vertices to the new connectivity
    new_conn.push_back(nodes[0]);
    new_conn.push_back(nodes[8]);
    new_conn.push_back(nodes[20]);
    new_conn.push_back(nodes[11]);
    new_conn.push_back(nodes[12]);
    new_conn.push_back(nodes[21]);
    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[24]);

    new_conn.push_back(nodes[8]);
    new_conn.push_back(nodes[1]);
    new_conn.push_back(nodes[9]);
    new_conn.push_back(nodes[20]);
    new_conn.push_back(nodes[21]);
    new_conn.push_back(nodes[13]);
    new_conn.push_back(nodes[22]);
    new_conn.push_back(nodes[26]);

    new_conn.push_back(nodes[11]);
    new_conn.push_back(nodes[20]);
    new_conn.push_back(nodes[10]);
    new_conn.push_back(nodes[3]);
    new_conn.push_back(nodes[24]);
    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[23]);
    new_conn.push_back(nodes[15]);

    new_conn.push_back(nodes[20]);
    new_conn.push_back(nodes[9]);
    new_conn.push_back(nodes[2]);
    new_conn.push_back(nodes[10]);
    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[22]);
    new_conn.push_back(nodes[14]);
    new_conn.push_back(nodes[23]);

    new_conn.push_back(nodes[12]);
    new_conn.push_back(nodes[21]);
    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[24]);
    new_conn.push_back(nodes[4]);
    new_conn.push_back(nodes[16]);
    new_conn.push_back(nodes[25]);
    new_conn.push_back(nodes[19]);

    new_conn.push_back(nodes[21]);
    new_conn.push_back(nodes[13]);
    new_conn.push_back(nodes[22]);
    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[16]);
    new_conn.push_back(nodes[5]);
    new_conn.push_back(nodes[17]);
    new_conn.push_back(nodes[25]);

    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[22]);
    new_conn.push_back(nodes[14]);
    new_conn.push_back(nodes[23]);
    new_conn.push_back(nodes[25]);
    new_conn.push_back(nodes[17]);
    new_conn.push_back(nodes[6]);
    new_conn.push_back(nodes[18]);

    new_conn.push_back(nodes[24]);
    new_conn.push_back(nodes[26]);
    new_conn.push_back(nodes[23]);
    new_conn.push_back(nodes[15]);
    new_conn.push_back(nodes[19]);
    new_conn.push_back(nodes[25]);
    new_conn.push_back(nodes[18]);
    new_conn.push_back(nodes[7]);

    n_elements = 8;



}
                                        

void MeshRefinement::parent2child(unsigned int &offset, unsigned short type, int tag, unsigned int nnoel, unsigned int n_new_element, std::vector<unsigned int> &refine_conn, std::vector<unsigned int> &new_conn, std::vector<unsigned int> &new_offset, std::vector<unsigned short> &new_type, std::vector<int> &new_physical_tag)
{
    new_conn.insert(new_conn.end(), refine_conn.begin(), refine_conn.end());
    for(int i = 0; i < n_new_element; i++)
    {
        offset += nnoel;
        new_offset.push_back(offset);
        new_type.push_back(type);
        new_physical_tag.push_back(tag);
    }   
}

void MeshRefinement::rebuild_communication_map()
{
    std::map<unsigned int, std::set<unsigned int>> shared_nodes_map;

    // loop over nodes that are shared
    for(auto& shared_nodes : shared_processors_per_node)
    {
        // loop over the processors that share the node
        for(auto& process : shared_nodes.second)
        {
            shared_nodes_map[process].insert(shared_nodes.first);
        }
    }

    std::vector<unsigned int> neighbors_processors;
    std::vector<unsigned int> shared_nodes_offset;
    std::vector<unsigned int> shared_nodes;

    shared_nodes_offset.push_back(0);
    unsigned int offset = 0;
    for(auto& nodes : shared_nodes_map)
    {
        neighbors_processors.push_back(nodes.first);
        offset += nodes.second.size();
        shared_nodes_offset.push_back(offset);
        shared_nodes.insert(shared_nodes.end(), nodes.second.begin(), nodes.second.end());
    }

    mesh.get_shared_nodes_vector().swap(shared_nodes);
    mesh.get_shared_nodes_offset_vector().swap(shared_nodes_offset);
    mesh.get_neighbors_processors_vector().swap(neighbors_processors);

    mesh.build_communication_map();
    mesh.fill_node_index();

}

