#ifndef MESHPART_H
#define MESHPART_H

#include <iostream>
#include <set>

#include "parallelmesh.h"

class Mesh_partition_t {
    public:
        Mesh_partition_t();
        ~Mesh_partition_t();
        int get_n_partitions();
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
        void MeshPartitioner(Mesh* mesh, int nparts);

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
        ParallelMesh* PartitionerInternalMPI(Mesh* mesh);

        /**
         * @brief 
         * 
         * @param mesh 
         * @return ParallelMesh* 
         */
        ParallelMesh* PartitionerMPI(Mesh* mesh);

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
         */
        void ProcessLocalArrays(std::vector<double> &coord_local, std::vector<unsigned int> &conn_local, std::vector<unsigned int> &offset_local,
                                std::vector<unsigned short> &type_local, std::vector<unsigned int> &local_to_global, std::vector<unsigned int> &global_to_local,
                                std::vector<unsigned int> &shared_out, std::map<unsigned int, std::set<unsigned int>> &node_partition, std::vector<unsigned int> &interface_nodes,
                                Mesh* mesh, ParallelMesh* pmesh, int i);

    private:
        int  n_partitions;  // Número de partições.
        int* nodal_part;    // Informações nodais da partição.
        int* elem_part;     // Informações elementares da partição.
};

#endif // MESHPART_H