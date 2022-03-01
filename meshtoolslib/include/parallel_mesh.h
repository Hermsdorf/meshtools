#ifndef PARALLELMESH__H_
#define PARALLELMESH__H_

#include <iostream>

#include "mesh.h"

class ParallelMesh : public Mesh{
    public:
        ParallelMesh();
        ~ParallelMesh();

        void readParallelMesh(const char* filename);
        void readParallelMeshBin(const char* filename);
        void writeParallelMesh();
        void writeParallelMeshBin();

        
        int                         get_n_neighbor_processors();
        void                        set_n_neighbor_processors(int n_neighbor_processors);
        bool                        get_internal_mesh();
        void                        set_internal_mesh(bool internal_mesh);
        unsigned int                get_n_global_nodes();
        void                        set_n_global_nodes(unsigned int n_global_nodes);
        unsigned int                get_n_global_elements();
        void                        set_n_global_elements(unsigned int n_global_elements);
        void                        set_n_global_face_elements(unsigned int n_global_face_elements);
        unsigned int                get_n_global_internal_elements();
        void                        set_n_global_internal_elements(unsigned int n_global_internal_elements);
        void                        add_neighbor_shared_nodes(unsigned int p, unsigned int n_shared_nodes, const unsigned *node_list);
        unsigned int                n_neighbor_shared_nodes(unsigned int p);
        const unsigned int *        get_neighbor_shared_nodes(unsigned int p);
        std::vector<unsigned int>&  getNeigborsProcessors();
        std::vector<unsigned int>&  getSharedNodesOffset();
        std::vector<unsigned int>&  getSharedNodes();
        std::vector<unsigned int>&  getLocal2Global();
       
        void renumbering();
    private:
        
        bool internal_mesh;
        unsigned int n_global_nodes;
        unsigned int n_global_elements;
        unsigned int n_global_faces;
        unsigned int n_global_internal_elements;
        std::vector<unsigned int> local_to_global;
        // Parallel Context attributes
        int processor_id;
        int n_processors;
        int n_neighbor_processors;
        std::vector<unsigned int> neighbor_processors;
        std::vector<unsigned int> shared_nodes_offset;
        std::vector<unsigned int> shared_nodes;
        
        std::vector<unsigned int> sendto_neighbors_map;
        std::vector<unsigned int> recvfrom_neighbors_map;
        
        void writePvtu();
        void BuildCommunicationMap();
};

#endif // PARALLELMESH__H_