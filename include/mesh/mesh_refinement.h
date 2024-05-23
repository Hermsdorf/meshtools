#ifndef MESH_REFINEMENT_H
#define MESH_REFINEMENT_H

#include <vector>
#include <unordered_map>
#include <set>
#include <memory>

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
        MeshRefinement(std::unique_ptr<ParallelMesh> &mesh);
        MeshRefinement(const MeshRefinement&) = delete;
        ~MeshRefinement();

    void refine();

    private:

        /**
         * @brief Refinamento de arestas:
         *        Subdivide as arestas em dois elementos
         * 
         * @param coords 
         * @param conn 
         * @param new_conn 
         * @param n_nodes 
         * @param n_edges 
         */
        void edge_refinement_template(std::vector<double>&      coords, 
                                              std::vector<unsigned int>    &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                              );

        /**
         * @brief Refina o elemento triangular em 4 elementos
         * 
         * @param coords 
         * @param conn 
         * @param new_conn 
         * @param n_nodes 
         * @param n_edges 
         */
        void triangle_refinement_template(std::vector<double>&      coords, 
                                              std::vector<unsigned int>    &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                                  );

        /**
         * @brief Refina o elemento quadrilateral em 4 elementos
         * 
         * @param coords 
         * @param conn 
         * @param new_conn 
         * @param n_nodes 
         * @param n_edges 
         */
        void quad_refinement_template(std::vector<double>&      coords,
                                              std::vector<unsigned int>    &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                                );

        /**
         * @brief Refina o elemento tetraédrico em 4 elementos
         * 
         * @param coords 
         * @param tetra_conn 
         * @param new_conn 
         * @param n_vertices 
         * @param n_tets 
         */
        void tetrahedron_refinement_template(std::vector<double>&   coords, 
                                              std::vector<unsigned int>    &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                              );
        
        /**
         * @brief Refina o elemento hexaédrico em 8 elementos
         * 
         * @param coords 
         * @param hexa_conn 
         * @param new_conn 
         * @param n_vertices 
         * @param n_elements 
         */
        void hexahedron_refinement_template(std::vector<double>&   coords, 
                                              std::vector<unsigned int>    &conn,
                                              int                         &parent_tag,
                                              std::vector<unsigned int>         &new_conn,
                                              std::vector<unsigned int>   &new_offset,
                                              std::vector<unsigned short> &new_type,
                                              std::vector<int>            &new_physical_tag,
                                              unsigned int                &n_nodes,
                                              unsigned int                &n_children
                                              );

        /**
         * @brief Copia os dados do elemento pai para o elemento filho
         * 
         * @param offset 
         * @param type 
         * @param tag 
         * @param nnoel 
         * @param n_new_element 
         * @param refine_conn 
         * @param new_conn 
         * @param new_offset 
         * @param new_type 
         * @param new_physical_tag 
         */
        // void parent2child(unsigned int &offset, unsigned short type, int tag, unsigned int nnoel, unsigned int n_new_element, 
        //                 std::vector<unsigned short> &new_type, 
        //                 std::vector<int> &new_physical_tag);

        /**
         * @brief mapeia os processadores vizinhos de cada nó do domínio
         * 
         */
        void build_shared_processor_per_node_map();

        /**
         * @brief Reconstroi o mapa de comunicação entre os processadores
         * 
         */
        void rebuild_comunication_map();

        /**
         * @brief Atualiza os arrays de conectividade, offset, tipo e tag
         * 
         * @param new_conn 
         * @param new_offset 
         * @param new_type 
         * @param new_tag 
         */
        void update_mesh_arrays(std::vector<unsigned int> &new_conn, std::vector<unsigned int> &new_offset, std::vector<unsigned short> &new_type, std::vector<int> &new_tag);

        /**
         * @brief obtem a lista de processadores que compartilha o nó central da aresta
         * 
         * @param conn 
         * @param new_node 
         */
        void find_processor_neighbours_edge(std::vector<unsigned int> &conn, unsigned int new_node);
        
        /**
         * @brief Obtem a lista de processadores que compartilha o nó central da face triangular
         * 
         * @param conn 
         * @param new_node 
         */
        void find_processor_neighbours_face_3_edges(std::vector<unsigned int> &conn, unsigned int new_node);
        
        /**
         * @brief Obtem a lista de processadores que compartilha o nó central da face quadrilateral
         * 
         * @param conn 
         * @param new_node 
         */
        void find_processor_neighbours_face_4_edges(std::vector<unsigned int> &conn, unsigned int new_node);

        /**
         * @brief Obtem o nó central da aresta
         * 
         * @param n_nodes 
         * @param coords 
         * @param face_conn 
         * @return unsigned int 
         */
        unsigned int edge_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn);
        
        /**
         * @brief Obtem o nó central da face
         * 
         * @param n_nodes 
         * @param coords 
         * @param face_conn 
         * @return unsigned int 
         */
        unsigned int face_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn);
        
        /**
         * @brief Obtem o nó central do elemento
         * 
         * @param n_nodes 
         * @param coords 
         * @param face_conn 
         * @return unsigned int 
         */
        unsigned int cell_central_vertice(unsigned int &n_nodes, std::vector<double> &coords, std::vector<unsigned int> &face_conn);

        std::unordered_map<unsigned long, unsigned int> edge_map;
        std::unordered_map<unsigned long, unsigned int> face_map;
        std::unordered_map<unsigned long, unsigned int> cell_map;

        std::unordered_map<unsigned int, unsigned long> node_map;
  
        std::unordered_map<unsigned int, std::set<unsigned int> > shared_processors_per_node; 

        std::unordered_map<unsigned int, std::set<unsigned int> > new_shared_processors_per_node; 

        std::unique_ptr<ParallelMesh> &mesh;

};

#endif /* MESH_REFINEMENT_H */
