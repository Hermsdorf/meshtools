#ifndef PARALLEL_MESH_H
#define PARALLEL_MESH_H

#include <iostream>

#include "mesh.h"

typedef struct 
{
    unsigned int processor_id;
    std::vector<unsigned int> nodes;
} MessageInformation;


class ParallelMesh : public Mesh{
    public:
        ParallelMesh();
        ~ParallelMesh();

        void readParallelMesh(const char* filename);
        void readParallelMeshBin(const char* filename);

        void writePVTK(const char* fname, MeshIODataAppended* info = nullptr);
        void WritePMesh(const char *fname);
        
        unsigned int                get_n_global_nodes(); 
        void                        set_n_global_nodes(unsigned int n_global_nodes);
        unsigned int                get_n_global_elements(); 
        void                        set_n_global_elements(unsigned int n_global_elements);
        void                        set_n_global_face_elements(unsigned int n_global_face_elements);
        unsigned int                get_n_local_nodes();
        unsigned int                get_start_global_index();
        void                        set_n_processors(int n_processors);
        int                         get_n_neighbor_processors();
        void                        set_n_neighbor_processors(int n_neighbor_processors);
        void                        add_neighbor_shared_nodes(unsigned int p, unsigned int n_shared_nodes, const unsigned *node_list);
        unsigned int                n_neighbor_shared_nodes(unsigned int p);
        const unsigned int *        get_neighbor_shared_nodes(unsigned int p);
        bool                        get_internal_mesh();
        void                        set_internal_mesh(bool internal_mesh); 

        void                        set_start_node_index(unsigned int start_node_index);

        std::vector<unsigned int>&  getNeighborsProcessors();
        std::vector<unsigned int>&  getSharedNodesOffset();
        std::vector<unsigned int>&  getSharedNodes();
        std::vector<unsigned int>&  getLocal2Global();
        void                        setNeighborProcessors(std::vector<unsigned int> neighbors_processors);
        void                        setSharedNodesOffset(std::vector<unsigned int> shared_nodes_offset);
        void                        setSharedNodes(std::vector<unsigned int> shared_nodes);
        void                        setLocal2Global(std::vector<unsigned int> local2global);

        std::vector<MessageInformation>& get_sendto_info();
        std::vector<MessageInformation>& get_recvfrom_info();
        void                             set_sendto_info(std::vector<MessageInformation> sendto_info);
        void                             set_recvfrom_info(std::vector<MessageInformation> recvfrom_info);
        
        void getGhostNodesIds(std::vector<unsigned int>& local_ghosts_nodes, std::vector<unsigned int>& global_ghosts_nodes);
        void renumbering();
        void build_communication_map();
    private:
        
        bool internal_mesh;
        unsigned int n_global_nodes;
        unsigned int n_global_elements;
        unsigned int n_global_faces;
        unsigned int n_global_internal_elements;
        unsigned int n_local_nodes;
        unsigned int start_node_index; // is the equivalent of _first_global_equation_index of equation_manager ?
        
        std::vector<unsigned int> local_to_global;
        // Parallel Context attributes
        int processor_id;
        int n_processors;
        int n_neighbor_processors;
        std::vector<unsigned int> neighbor_processors;
        std::vector<unsigned int> shared_nodes_offset;
        std::vector<unsigned int> shared_nodes;
        
        std::vector <MessageInformation> sendto_info;
        std::vector <MessageInformation> recvfrom_info;
};

#endif /* PARALLEL_MESH_H */
