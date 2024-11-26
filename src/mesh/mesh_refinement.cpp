
#include "mesh.h"
#include "hash.h"
#include "mesh_helper.h"
#include "mesh_refinement.h"

#include <unordered_set>

typedef struct 
{
    unsigned long hash;
    unsigned int  id;
} NodeHash;


MeshRefinement::MeshRefinement(std::unique_ptr<ParallelMesh> &mesh): mesh(mesh)
{
}

MeshRefinement::~MeshRefinement()
{
}

void MeshRefinement::refine()
{

    MeshTools::PrintDebug("Begin Refinement\n");

    // Clear the maps
    edge_map.clear();
    face_map.clear();
    cell_map.clear();
    node_map.clear();

    
    shared_processors_per_node.clear();
    new_shared_processors_per_node.clear();

    unsigned int n_nodes = mesh->get_n_nodes();
    unsigned int nse     = mesh->get_n_surface_elements();
    unsigned int ne      = mesh->get_n_elements();

    // Get the mesh data
    auto &coords       = mesh->get_coordinate_vector();
    auto &conn         = mesh->get_connectivity_vector();
    auto &offset       = mesh->get_offset_vector();
    auto &type         = mesh->get_element_type_vector();
    auto &physical_tag = mesh->get_element_physical_tag_vector();
    

    unsigned int n_new_surface_elements = 0;
    unsigned int n_new_elements         = 0;

    // New mesh data
    //std::vector<double>         new_coords;
    std::vector<unsigned int>   new_conn;
    std::vector<unsigned int>   new_offset;
    std::vector<unsigned short> new_type;
    std::vector<int>            new_physical_tag;

    std::vector<unsigned int> new_neighbors_processors;
    std::vector<unsigned int> new_shared_nodes;
    std::vector<unsigned int> new_shared_nodes_offset;



    // reserve memory
    new_conn.reserve(4*conn.size());
    new_offset.reserve(4*offset.size());
    new_type.reserve(4*type.size());
    new_physical_tag.reserve(4*physical_tag.size());

    new_neighbors_processors.reserve(mesh->get_neighbors_processors_vector().size());
    new_shared_nodes.reserve(mesh->get_shared_nodes_vector().size());
    new_shared_nodes_offset.reserve(mesh->get_shared_nodes_offset_vector().size());
    

    // Build the shared processor per node map
    build_shared_processor_per_node_map();

    new_offset.emplace_back(0);
    unsigned int offset_count = 0;

    for(int i = 0; i < nse; i++)
    {
        std::vector<unsigned int> element_conn;
        unsigned int n_count_elements = 0;
        mesh->get_surface_element_connectivity(i, element_conn);
        switch (type[i])
        {
        case EDGE2:
            edge_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements);  
            break;
        case TRI3:
            triangle_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements); 
            break;
        case QUAD4:
            quad_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements); 
            break;
        default:
            break;
        }
        
        n_new_surface_elements += n_count_elements;
    }


    for(int i = 0; i < ne; i++)
    {
        std::vector<unsigned int> element_conn;
        
        unsigned int n_count_elements = 0;
        mesh->get_element_connectivity(i, element_conn);

        switch (mesh->get_element_type(i))
        {
        case TRI3:
            triangle_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements); 
            break;
        case QUAD4:
            quad_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements); 
            break;
        case TET4:
            tetrahedron_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements); 
            break;
        case HEX8:
            hexahedron_refinement_template(coords, element_conn, physical_tag[i], 
                                     new_conn, new_offset,
                                     new_type, new_physical_tag, n_nodes, n_count_elements); 
            break;
        default:
            MeshTools::PrintDebug("Element Type not implemented\n");
            break;
        }
        n_new_elements += n_count_elements;
    }

    MeshTools::PrintDebug("begin update mesh\n");
    mesh->set_n_nodes(n_nodes);
    mesh->set_n_elements(n_new_elements);
    mesh->set_n_surface_elements(n_new_surface_elements);


    conn         = new_conn;
    offset       = new_offset ;
    type         = new_type ;
    physical_tag = new_physical_tag;
    MeshTools::PrintDebug("end update mesh\n");
    
    mesh->process_face_to_element();

    // update the mesh arrays
    //update_mesh_arrays(mesh, new_conn, new_offset, new_type, new_physical_tag);
    this->rebuild_comunication_map();

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
    
    std::vector<unsigned int> edge_conn_global(2);
    edge_conn_global[0] = mesh->get_node_id(edge_conn[0]);
    edge_conn_global[1] = mesh->get_node_id(edge_conn[1]);

    //cout << "Criando ponto central da aresta: " << edge_conn_global[0] << " ----- " << edge_conn_global[1] << endl;

    unsigned long hash = compute_hash(edge_conn_global);
    // Check if the edge has been refined
    if (edge_map.find(hash) == edge_map.end())
    {
        //cout<< "** Aresta nao refinada **"<<endl;
    
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
        coords.emplace_back(x);
        coords.emplace_back(y);
        coords.emplace_back(z);

        //std::cout << "Edge Central Vertice: " << n_nodes << " " << x << " " << y << " " << z << std::endl;
        MeshTools::PrintDebug("Edge (%d - %d)  Central Vertice: %d %f %f %f - hash: %ld\n",edge_conn_global[0],edge_conn_global[1],n_nodes, x, y, z, hash);

        // Add the new vertex to the edge map
        edge_map[hash]    = n_nodes;
        node_map[n_nodes] = hash;
        n_nodes++;
    
    }

    //cout<< "Nócentral da aresta: "<<edge_map[hash]<<endl<<endl;

    return edge_map[hash];

}

unsigned int MeshRefinement::face_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn)
{

    std::vector<unsigned int> face_conn_global(face_conn.size());
    for(int i = 0; i < face_conn.size(); i++)
    {
        face_conn_global[i] = mesh->get_node_id(face_conn[i]);
    }

    unsigned long hash = compute_hash(face_conn_global);

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
        coords.emplace_back(x);
        coords.emplace_back(y);
        coords.emplace_back(z);
    
        //std::cout << "Face Central Vertice: " << n_nodes << " " << x << " " << y << " " << z << std::endl;
        MeshTools::PrintDebug("Face Central Vertice: %d %f %f %f\n", n_nodes, x, y, z);

        // Add the new vertex to the face map
        face_map[hash] = n_nodes;
        node_map[n_nodes] = hash;
        n_nodes++;
    
    }

    return face_map[hash];

}

unsigned int MeshRefinement::cell_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &cell_conn)
{

    std::vector<unsigned int> cell_conn_global(cell_conn.size());
    for(int i = 0; i < cell_conn.size(); i++)
    {
        cell_conn_global[i] = mesh->get_node_id(cell_conn[i]);
    }
    unsigned long hash = compute_hash(cell_conn_global);
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
        coords.emplace_back(x);
        coords.emplace_back(y);
        coords.emplace_back(z);

        MeshTools::PrintDebug("Cell Central Vertice: %d %f %f %f\n", n_nodes, x, y, z);
    
        // Add the new vertex to the face map
        cell_map[hash]    = n_nodes;
        node_map[n_nodes] = hash;
        n_nodes++;
    
    }

    return cell_map[hash];
    
}

void MeshRefinement::build_shared_processor_per_node_map()
{

    //cout << "Build Shared Processor Per Node Map\n";
    auto& shared_nodes_offset  = mesh->get_shared_nodes_offset_vector();
    auto& shared_nodes         = mesh->get_shared_nodes_vector();
    auto& neighbors_processors = mesh->get_neighbors_processors_vector();

    for(int p = 0; p < neighbors_processors.size(); p++)
    {
        unsigned int process        = neighbors_processors[p];
        unsigned int offset_start   = shared_nodes_offset[p];
        unsigned int offset_end     = shared_nodes_offset[p+1];
        for(unsigned int i = offset_start; i < offset_end; i++)
        {
            shared_processors_per_node[shared_nodes[i]].insert(process);
        }
    }

#ifdef NDEBUG
    MeshTools::PrintDebug("Build Shared Processor Per Node Map\n");
    for(auto it = shared_processors_per_node.begin(); it != shared_processors_per_node.end(); it++)
    {
        MeshTools::PrintDebug("Node: %d ", it->first);
        MeshTools::PrintDebug(" shared with processors: ");
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            MeshTools::PrintDebug(" %d", *it2);
        }
        MeshTools::PrintDebug("\n");
    }
#endif
}

void MeshRefinement::find_processor_neighbours_edge(std::vector<unsigned int> &conn, unsigned int new_node)
{

   if(new_shared_processors_per_node.find(new_node) != new_shared_processors_per_node.end())
        return;
    
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
    std::set<unsigned int> result;
    std::set_intersection(processors_v1.begin(), processors_v1.end(), processors_v2.begin(), processors_v2.end(), std::inserter(result, result.begin()));

    // Add the new node to the shared nodes
    new_shared_processors_per_node[new_node] = result;

#ifdef NDEBUG
    MeshTools::PrintDebug("Node %d is shared with ", new_node);
    for(auto p: result)
        MeshTools::PrintDebug("%d ", p);
    MeshTools::PrintDebug("\n");
#endif

}

void MeshRefinement::find_processor_neighbours_face_3_edges(std::vector<unsigned int> &conn, unsigned int new_node)
{
    if(new_shared_processors_per_node.find(new_node) != new_shared_processors_per_node.end())
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
    new_shared_processors_per_node[new_node] = result;
#ifdef NDEBUG
    MeshTools::PrintDebug("Node %d is shared with ", new_node);
    for(auto p: result)
        MeshTools::PrintDebug("%d ", p);
    MeshTools::PrintDebug("\n");
#endif

}

void MeshRefinement::find_processor_neighbours_face_4_edges(std::vector<unsigned int> &conn, unsigned int new_node)
{
    if(new_shared_processors_per_node.find(new_node) != new_shared_processors_per_node.end())
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
    new_shared_processors_per_node[new_node] = result;

#ifdef NDEBUG
    MeshTools::PrintDebug("Node %d is shared with ", new_node);
    for(auto p: result)
        MeshTools::PrintDebug("%d ", p);
    MeshTools::PrintDebug("\n");
#endif

}


void MeshRefinement::edge_refinement_template(std::vector<double>         &coords, 
                                              std::vector<unsigned int>   &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>    &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                              )
{
    assert(conn.size() == 2);


    // Get the edge
    unsigned int v0 = conn[0];
    unsigned int v1 = conn[1];
    unsigned int v2 = edge_central_vertice(n_nodes, coords, conn);

    find_processor_neighbours_edge(conn, v2);

    // Add the new vertex to the new connectivity
    new_conn.emplace_back(v0);
    new_conn.emplace_back(v2);
    
    //get last offset store
    unsigned int last_offset = new_offset.back();
    new_offset.emplace_back(last_offset + 2);
    new_type.emplace_back(EDGE2);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(v2);
    new_conn.emplace_back(v1);
    new_offset.emplace_back(last_offset + 4);
    new_type.emplace_back(EDGE2);
    new_physical_tag.emplace_back(parent_tag);

    n_children = 2;

    MeshTools::PrintDebug("Edge Refinement Template\n");
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


void MeshRefinement:: triangle_refinement_template(std::vector<double>&      coords, 
                                              std::vector<unsigned int>    &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                              )
{

    assert(conn.size() == 3);

    std::vector<unsigned int> nodes(6);

    nodes[0] = conn[0];
    nodes[1] = conn[1]; 
    nodes[2] = conn[2];

    // Get the triangle edges
    for(int edge=0; edge < 3; edge++)
    {
        std::vector<unsigned int> edge_conn(2);
        MeshHelper::triangle_face_connectivity(edge, conn, edge_conn);
        nodes[3+edge] = edge_central_vertice(n_nodes, coords, edge_conn);
        find_processor_neighbours_edge(edge_conn, nodes[3+edge]);
    }

    //cout<< "new con = [" << nodes[0] << ", " << nodes[1] << ", " << nodes[2] << ", " << nodes[3] << ", " << nodes[4] << ", " << nodes[5] << "]" << endl <<endl<<endl;

    // Add the new vertices to the new connectivity
    unsigned int last_offset = new_offset.back();

    new_conn.emplace_back(nodes[0]);
    new_conn.emplace_back(nodes[3]);
    new_conn.emplace_back(nodes[5]);
    new_offset.emplace_back(last_offset + 3);
    new_type.emplace_back(TRI3);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[3]);
    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[5]);
    new_offset.emplace_back(last_offset + 6);
    new_type.emplace_back(TRI3);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[3]);
    new_conn.emplace_back(nodes[1]);
    new_conn.emplace_back(nodes[4]);
    new_offset.emplace_back(last_offset + 9);
    new_type.emplace_back(TRI3);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[2]);
    new_offset.emplace_back(last_offset + 12);
    new_type.emplace_back(TRI3);
    new_physical_tag.emplace_back(parent_tag);

    n_children = 4;
    MeshTools::PrintDebug("Triangle Refinement Template\n");

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
                                              std::vector<unsigned int>    &quad_conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
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

    unsigned int last_offset = new_offset.back();

    new_conn.emplace_back(nodes[0]);
    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[7]);
    new_offset.emplace_back(last_offset + 4);
    new_type.emplace_back(QUAD4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[1]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[8]);
    new_offset.emplace_back(last_offset + 8);
    new_type.emplace_back(QUAD4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[2]);
    new_conn.emplace_back(nodes[6]);
    new_offset.emplace_back(last_offset + 12);
    new_type.emplace_back(QUAD4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[3]);
    new_offset.emplace_back(last_offset + 16);
    new_type.emplace_back(QUAD4);
    new_physical_tag.emplace_back(parent_tag);

    n_children = 4;

    MeshTools::PrintDebug("Quadrilateral Refinement Template\n");
}

void MeshRefinement::tetrahedron_refinement_template(std::vector<double>&   coords, 
                                              std::vector<unsigned int>    &tetra_conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                              )
{
    assert(tetra_conn.size() == 4);

     std::vector<unsigned int> nodes(10);

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
    }
    
    //cout<<"new con = [ " << nodes[0] << ", " << nodes[1] << ", " << nodes[2] << ", " << nodes[3] << ", " << nodes[4] << ", " << nodes[5] << ", " << nodes[6] << ", " << nodes[7] << ", " << nodes[8] << ", " << nodes[9] << " ]"<<endl;

    unsigned int last_offset = new_offset.back();

    new_conn.emplace_back(nodes[0]);
    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[7]);
    new_offset.emplace_back(last_offset + 4);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[1]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[8]);
    new_offset.emplace_back(last_offset + 8);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[2]);
    new_conn.emplace_back(nodes[9]);
    new_offset.emplace_back(last_offset + 12);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[3]);
    new_offset.emplace_back(last_offset + 16);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[8]);
    new_offset.emplace_back(last_offset + 20);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[6]);
    new_offset.emplace_back(last_offset + 24);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[9]);
    new_offset.emplace_back(last_offset + 28);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[5]);
    new_offset.emplace_back(last_offset + 32);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    /*
  
    new_conn.emplace_back(nodes[0]);
    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[7]);
    new_offset.emplace_back(last_offset + 4);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[1]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[9]);
    new_offset.emplace_back(last_offset + 8);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[2]);
    new_conn.emplace_back(nodes[8]);
    new_offset.emplace_back(last_offset + 12);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[3]);
    new_offset.emplace_back(last_offset + 16);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[9]);
    new_offset.emplace_back(last_offset + 20);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[6]);
    new_offset.emplace_back(last_offset + 24);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[7]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[8]);
    new_offset.emplace_back(last_offset + 28);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[5]);
    new_offset.emplace_back(last_offset + 32);
    new_type.emplace_back(TET4);
    new_physical_tag.emplace_back(parent_tag);

    */

    n_children = 8;


}



void MeshRefinement::hexahedron_refinement_template(std::vector<double>&   coords, 
                                              std::vector<unsigned int>    &hexa_conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
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
//1
    unsigned int last_offset = new_offset.back();
    new_conn.emplace_back(nodes[0]);
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[24]);
    new_conn.emplace_back(nodes[11]);
    new_conn.emplace_back(nodes[16]);
    new_conn.emplace_back(nodes[20]);
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[23]);
    new_offset.emplace_back(last_offset + 8);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//2
    new_conn.emplace_back(nodes[16]);
    new_conn.emplace_back(nodes[20]);
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[23]);
    new_conn.emplace_back(nodes[4]);
    new_conn.emplace_back(nodes[12]);
    new_conn.emplace_back(nodes[25]);
    new_conn.emplace_back(nodes[15]);
    new_offset.emplace_back(last_offset + 16);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//3
    new_conn.emplace_back(nodes[8]);
    new_conn.emplace_back(nodes[1]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[24]);
    new_conn.emplace_back(nodes[20]);
    new_conn.emplace_back(nodes[17]); 
    new_conn.emplace_back(nodes[21]);
    new_conn.emplace_back(nodes[26]);
    new_offset.emplace_back(last_offset + 24);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//4
    new_conn.emplace_back(nodes[20]);
    new_conn.emplace_back(nodes[17]);
    new_conn.emplace_back(nodes[21]);
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[12]);
    new_conn.emplace_back(nodes[5]);
    new_conn.emplace_back(nodes[13]);
    new_conn.emplace_back(nodes[25]);
    new_offset.emplace_back(last_offset + 32);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//5
    new_conn.emplace_back(nodes[11]);
    new_conn.emplace_back(nodes[24]);
    new_conn.emplace_back(nodes[10]);
    new_conn.emplace_back(nodes[3]);
    new_conn.emplace_back(nodes[23]);
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[22]);
    new_conn.emplace_back(nodes[19]);
    new_offset.emplace_back(last_offset + 40);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//6
    new_conn.emplace_back(nodes[23]);
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[22]);
    new_conn.emplace_back(nodes[19]);
    new_conn.emplace_back(nodes[15]);
    new_conn.emplace_back(nodes[25]);
    new_conn.emplace_back(nodes[14]);
    new_conn.emplace_back(nodes[7]);
    new_offset.emplace_back(last_offset + 48);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//7
    new_conn.emplace_back(nodes[24]);
    new_conn.emplace_back(nodes[9]);
    new_conn.emplace_back(nodes[2]);
    new_conn.emplace_back(nodes[10]);
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[21]);
    new_conn.emplace_back(nodes[18]);
    new_conn.emplace_back(nodes[22]);
    new_offset.emplace_back(last_offset + 56);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);
//8
    new_conn.emplace_back(nodes[26]);
    new_conn.emplace_back(nodes[21]);
    new_conn.emplace_back(nodes[18]);
    new_conn.emplace_back(nodes[22]);
    new_conn.emplace_back(nodes[25]);
    new_conn.emplace_back(nodes[13]);
    new_conn.emplace_back(nodes[6]);
    new_conn.emplace_back(nodes[14]);
    new_offset.emplace_back(last_offset + 64);
    new_type.emplace_back(HEX8);
    new_physical_tag.emplace_back(parent_tag);

    n_children = 8;



}
                                        


void MeshRefinement::rebuild_comunication_map()
{

    auto & coords               = mesh->get_coordinate_vector();
    auto & shared_nodes         = mesh->get_shared_nodes_vector();
    auto & shared_nodes_offset  = mesh->get_shared_nodes_offset_vector();
    auto & neighbors_processors = mesh->get_neighbors_processors_vector();

    std::vector<unsigned int> new_shared_nodes_offset;
    std::vector<unsigned int> new_shared_nodes;

    new_shared_nodes_offset.emplace_back(0);
    unsigned int offset = 0;


    // mapeia o processador e os nós que compartilhados
    std::map<unsigned int, std::unordered_set<unsigned int> > processor_node_map;

    // para cada nó obtem a lista de processadores que o compartilham
    for(auto& node : new_shared_processors_per_node)
    {
        // da lista de processadores, insere o nó no mapeiamento do processador
        for(auto& process : node.second)
        {
            processor_node_map[process].insert(node.first);
        }
    }

    for(int p = 0; p < neighbors_processors.size();p++)
    {
        unsigned int start = shared_nodes_offset[p];
        unsigned int end = shared_nodes_offset[p+1];
        
        // adiciona os nós ja existentes no novo vetor
        for(int i = start; i < end; i++)
            new_shared_nodes.emplace_back(shared_nodes[i]);
        offset+=(end-start);

        // os novos nós compartilhados devem ser ordenados em função da hash
        // para casar nas partições
        auto node_list = processor_node_map[neighbors_processors[p]];

        std::vector<std::pair<unsigned long, unsigned int> > nodes;
        for(auto node : node_list) {
            nodes.push_back(std::pair<unsigned long, unsigned int>(node_map[node], node));
        }

        std::sort(nodes.begin(), nodes.end(), [](std::pair<unsigned long, unsigned int> &a, std::pair<unsigned long, unsigned int> &b) { return a.first < b.first; });

        // insere os novos nos no novo vetor.
        for(int i = 0; i < nodes.size(); i++)
            new_shared_nodes.emplace_back(nodes[i].second);

        offset+=nodes.size();
        new_shared_nodes_offset.emplace_back(offset);

    }
    

    mesh->get_shared_nodes_offset_vector() = new_shared_nodes_offset;
    mesh->get_shared_nodes_vector()        = new_shared_nodes;
    

#ifdef NDEBUG

    for(int i = 0; i < neighbors_processors.size(); i++)
    {
        MeshTools::PrintDebug("Processor: %d\n", neighbors_processors[i]);
        for(int j = shared_nodes_offset[i]; j < shared_nodes_offset[i+1]; j++)
        {
            unsigned int node_id = shared_nodes[j];
            auto res =  node_map.find(node_id);
            unsigned int hash = (res != node_map.end()) ?  res->second : 0;
            
            MeshTools::PrintDebug("Node: %d (%f, %f %f ) - hash: %ld \n", node_id, coords[node_id*3],coords[node_id*3+1], coords[node_id*3+2], hash);
        }
    }
#endif

    mesh->build_communication_map();
    mesh->fill_node_index();

}

