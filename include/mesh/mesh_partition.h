#ifndef MESH_PARTITION_H
#define MESH_PARTITION_H

#include <iostream>
#include <set>
#include <memory>


#include "parallel_mesh.h"

class MeshPartition
 {
    public:

        enum class PartitionMode {METIS=0, SCOTCH=1};
        
        
        MeshPartition();
        ~MeshPartition();

        int  get_n_partitions();
        std::vector<int>& get_nodal_part();
        std::vector<int>& get_elem_part();
        void set_n_partitions(int n_partitions);


        /**
         * @brief 
         * 
         * @param mesh 
         * @return ParallelMesh* 
         */
        std::unique_ptr<ParallelMesh> distributed_mesh(std::unique_ptr<Mesh>& mesh);


        void apply_metis_partition(std::unique_ptr<Mesh>& mesh, int nparts);


    private:
        int  n_partitions;                // Número de partições.
        std::vector<int> nodal_part;      // Informações nodais da partição.
        std::vector<int> elem_part;       // Informações elementares da partição.
        std::vector<int> face_part;      
        bool applied; 

        std::unique_ptr<ParallelMesh> recv_local_data_from_master();

        void get_node_partition(std::unique_ptr<Mesh>& mesh, std::map<unsigned int, std::set<unsigned int> > &node_partition);

        void get_shared_nodes(std::unique_ptr<Mesh>& mesh , std::map<unsigned, std::set<unsigned> > &shared_nodes);
        
        void get_and_send_local_data(std::unique_ptr<Mesh>& mesh, int sendto,
            int *array_sizes,
            std::map<unsigned int, std::set<unsigned int> > &node_partition,
            std::vector<double>         & coords,
            std::vector<unsigned int>   & node_index,
            std::vector<unsigned int>   & conn,
            std::vector<unsigned int>   & offset,
            std::vector<unsigned short> & type,
            std::vector<int>            & tag,
            std::vector<unsigned int>   & neighbors,
            std::vector<unsigned int>   & neighbors_offset,
            std::vector<unsigned int>   & neighbors_nodes,
            std::vector<unsigned int>    & face_to_element,
            bool                        enable_send
        );

};

#endif /* MESH_PARTITION_H */
