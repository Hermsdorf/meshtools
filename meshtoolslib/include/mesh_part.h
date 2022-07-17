#ifndef MESH_PART_H
#define MESH_PART_H

#include <iostream>
#include <set>

#include "parallel_mesh.h"

class MeshPartition
 {
    public:
        MeshPartition();
        ~MeshPartition();
        int  get_n_partitions();
        int* get_nodal_part();
        int* get_elem_part();
        void set_n_partitions(int n_partitions);
        void set_nodal_part(int* nodal_part);
        void set_elem_part(int* elem_part);

        /**
         * @brief 
         * 
         * @param mesh 
         * @param nparts 
         */
        void ApplyPartitioner(Mesh* mesh, int nparts);

        /**
         * @brief 
         * 
         * @param mesh 
         * @param nparts 
         */
        void MeshPartitionerInternal(Mesh* mesh, int nparts);

        /**
         * @brief 
         * 
         * @param mesh 
         */
        void WriteInternalPartition(Mesh* mesh);

        /**
         * @brief 
         * 
         * @param mesh 
         */
        void WriteInternalPartitionBin(Mesh* mesh);

        /**
         * @brief 
         * 
         * @param mesh 
         */
        void WritePartition(Mesh* mesh);

        /**
         * @brief 
         * 
         * @param mesh 
         */
        void WritePartitionBin(Mesh* mesh);

        /**
         * @brief 
         * 
         * @param mesh 
         * @return ParallelMesh* 
         */
        ParallelMesh* DistributedMeshInternal(Mesh* mesh, int processor_id=0, int n_processor=1);

        /**
         * @brief 
         * 
         * @param mesh 
         * @return ParallelMesh* 
         */
        ParallelMesh* DistributedMesh(Mesh* mesh);

        /**
         * @brief 
         * 
         * @param coord_local 
         * @param conn_local 
         * @param offset_local 
         * @param type_local 
         * @param local_to_global 
         * @param global_to_local 
         * @param shared_out 
         * @param node_partition 
         * @param interface_nodes 
         * @param mesh 
         * @param pmesh 
         * @param i 
         
        void ProcessLocalArrays(std::vector<double> &coord_local, std::vector<unsigned int> &conn_local, std::vector<unsigned int> &offset_local,
                                std::vector<unsigned short> &type_local, std::vector<unsigned int> &local_to_global, std::vector<unsigned int> &global_to_local,
                                std::vector<unsigned int> &shared_out, std::map<unsigned int, std::set<unsigned int>> &node_partition, std::vector<unsigned int> &interface_nodes,
                                Mesh* mesh, ParallelMesh* pmesh, int i);
        */
        void GetSharedNodes(Mesh* mesh, std::map<unsigned, std::set<unsigned> > &shared_nodes);
        void GetAndSendLocalData(Mesh *mesh, int sendto,
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
            bool                        enable_send
        );

        ParallelMesh* RecvLocalDataFromMaster();
        void WriteAscii(Mesh* mesh, int n_processors, const char *fname);
        void WriteVTK(Mesh* mesh, const char* fname);
        void WriteDistributedMesh(Mesh* mesh, int processor, int n_processors, const char* basename);
    private:
        int  n_partitions;    // Número de partições.
        int* nodal_part;      // Informações nodais da partição.
        int* elem_part;       // Informações elementares da partição.
        int *face_part;
        bool applied; 

    void WritePartionData(
        const char* basename,
        int processor,
        int *array_sizes,
        std::vector<double>         & coord,
        std::vector<unsigned int>   & node_index,
        std::vector<unsigned int>   & conn,
        std::vector<unsigned int>   & offset,
        std::vector<unsigned short> & type,
        std::vector<int>            & tag,
        std::vector<unsigned int>   & neighbors,
        std::vector<unsigned int>   & neighbors_offset,
        std::vector<unsigned int>   & neighbors_nodes);

    void GetNodePartition(Mesh *mesh, std::map<unsigned int, std::set<unsigned int> > &node_partition);

};

#endif /* MESH_PART_H */
