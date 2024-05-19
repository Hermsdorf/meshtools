#ifndef MESH_REFINEMENT_H__
#define MESH_REFINEMENT_H__

#include <vector>
#include <unordered_map>
#include <set>

#include "parallel_mesh.h"

struct EdgeHash
{
    EdgeHash() : new_vertex(0), is_divided(false), hash(0) {}
    unsigned int  new_vertex;
    bool          is_divided; 
    unsigned long hash;
};

typedef EdgeHash FaceHash;
typedef EdgeHash CellHash;


class MeshRefinement
{
    public:
        MeshRefinement(ParallelMesh &mesh);
        ~MeshRefinement();

    void refine(unsigned int n_refinements);

    private:
        void edge_refinement_template(std::vector<double>&      coords, 
                                              std::vector<unsigned int> &conn,
                                              std::vector<unsigned int> &new_conn,
                                              unsigned int              &n_nodes,
                                              unsigned int              &n_edges
                                              );

        void triangle_refinement_template(std::vector<double>&      coords, 
                                              std::vector<unsigned int> &conn,
                                              std::vector<unsigned int> &new_conn,
                                              unsigned int              &n_nodes,
                                              unsigned int              &n_edges
                                                  );

        void quad_refinement_template(std::vector<double>&      coords,
                                              std::vector<unsigned int> &conn,
                                              std::vector<unsigned int> &new_conn,
                                              unsigned int              &n_nodes,
                                              unsigned int              &n_edges
                                                );

        void tetrahedron_refinement_template(std::vector<double>&   coords, 
                                                  std::vector<unsigned int> &tetra_conn,
                                                  std::vector<unsigned int> &new_conn,
                                                  unsigned int              &n_vertices,
                                                  unsigned int              &n_tets
                                              );
        
        void hexahedron_refinement_template(std::vector<double>&   coords, 
                                                  std::vector<unsigned int> &hexa_conn,
                                                  std::vector<unsigned int> &new_conn,
                                                  unsigned int              &n_vertices,
                                                  unsigned int              &n_elements
                                              );


        void parent2child(unsigned int &offset, unsigned short type, int tag, unsigned int nnoel, unsigned int n_new_element, 
                        std::vector<unsigned int> &refine_conn, 
                        std::vector<unsigned int> &new_conn, 
                        std::vector<unsigned int> &new_offset, 
                        std::vector<unsigned int> &new_type, 
                        std::vector<unsigned int> &new_physical_tag);

        void build_shared_processor_per_node_map(ParallelMesh& mesh);

        void rebuild_communication_map();

        void update_mesh_arrays(std::vector<unsigned int> &new_conn, std::vector<unsigned int> &new_offset, std::vector<unsigned short> &new_type, std::vector<int> &new_tag);

        void find_processor_neighbours_edge(std::vector<unsigned int> &conn, unsigned int new_node);
        void find_processor_neighbours_face_3_edges(std::vector<unsigned int> &conn, unsigned int new_node);
        void find_processor_neighbours_face_4_edges(std::vector<unsigned int> &conn, unsigned int new_node);

        unsigned int edge_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn);
        unsigned int face_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn);
        unsigned int cell_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn);

        std::unordered_map<unsigned long, unsigned int> edge_map;
        std::unordered_map<unsigned long, unsigned int> face_map;
        std::unordered_map<unsigned long, unsigned int> cell_map;

        std::unordered_map<unsigned int, std::set<unsigned int> > shared_processors_per_node;    

        ParallelMesh &mesh;

};

#endif /* MESH_REFINEMENT_H__ */
