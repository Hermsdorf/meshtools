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

        ParallelMesh(const ParallelMesh&) = delete; 
        ~ParallelMesh();

        void write(std::string filename);

        /**
         * @brief Get the n global nodes 
         * 
         * @return unsigned int 
         */
        unsigned int                get_n_global_nodes(); 

        /**
         * @brief Set the n global nodes 
         * 
         * @param n_global_nodes 
         */
        void                        set_n_global_nodes(unsigned int n_global_nodes);

        /**
         * @brief Get the n global elements object
         * 
         * @return unsigned int 
         */
        unsigned int                get_n_global_elements(); 

        /**
         * @brief Set the n global elements object
         * 
         * @param n_global_elements 
         */
        void                        set_n_global_elements(unsigned int n_global_elements);

        /**
         * @brief Set the n global surface elements object
         * 
         * @param n_global_face_elements 
         */
        void                        set_n_global_surface_elements(unsigned int n_global_face_elements);

        /**
         * @brief Get the n local nodes object
         * 
         * @return unsigned int 
         */
        unsigned int                get_n_local_nodes();

        /**
         * @brief Set the n local nodes object
         * 
         * @param n_local_nodes 
         */
        void                        set_n_local_nodes(unsigned int n_local_nodes);

        /**
         * @brief Get the start global index object
         * 
         * @return unsigned int 
         */
        unsigned int                get_start_global_index();

        /**
         * @brief Set the n processors object
         * 
         * @param n_processors 
         */
        void                        set_n_processors(int n_processors);

        /**
         * @brief Get the n neighbor processors object
         * 
         * @return int 
         */
        int                         get_n_neighbor_processors();

        /**
         * @brief Set the n neighbor processors object
         * 
         * @param n_neighbor_processors 
         */
        void                        set_n_neighbor_processors(int n_neighbor_processors);


        void                        add_neighbor_shared_nodes(unsigned int p, unsigned int n_shared_nodes, const unsigned *node_list);

        unsigned int                n_neighbor_shared_nodes(unsigned int p);

        const unsigned int *        get_neighbor_shared_nodes(unsigned int p);
       
        void                        set_start_node_index(unsigned int start_node_index);

        std::vector<unsigned int>&  get_neighbors_processors_vector();
        std::vector<unsigned int>&  get_shared_nodes_offset_vector();
        std::vector<unsigned int>&  get_shared_nodes_vector();


        void                        set_neighbor_processors_vector(std::vector<unsigned int>& neighbors_processors);
        void                        set_shared_nodes_offset_vector(std::vector<unsigned int>& shared_nodes_offset);
        void                        set_shared_nodes_vector(std::vector<unsigned int>& shared_nodes);
    


        std::vector<MessageInformation>& get_sendto_info();
        std::vector<MessageInformation>& get_recvfrom_info();
        void                             set_sendto_info(std::vector<MessageInformation>& sendto_info);
        void                             set_recvfrom_info(std::vector<MessageInformation>& recvfrom_info);
        
        void get_ghost_nodes_ids(std::vector<unsigned int>& local_ghosts_nodes, std::vector<unsigned int>& global_ghosts_nodes);
        void fill_node_index();
        void build_communication_map();

    private:
        
        unsigned int n_global_nodes;
        unsigned int n_global_elements;
        unsigned int n_global_surface_elements;
        unsigned int n_global_internal_elements;
        unsigned int n_local_nodes;
        unsigned int start_node_index; // is the equivalent of _first_global_equation_index of equation_manager ?
        
        // Parallel Context attributes
        int processor_id;
        int n_processors;
        int n_neighbor_processors;

        std::vector<unsigned int> neighbor_processors; // armazena os ids dos processadores vizinhos
        std::vector<unsigned int> shared_nodes_offset; // Indica oa posição do dos nós compartilhados
        std::vector<unsigned int> shared_nodes;
        
        std::vector <MessageInformation> sendto_info;
        std::vector <MessageInformation> recvfrom_info;
};

#endif /* PARALLEL_MESH_H */
