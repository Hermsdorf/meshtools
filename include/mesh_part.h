#ifndef MESHPART_H
#define MESHPART_H

#include <iostream>

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

        void MeshPartitioner(Mesh* mesh, int nparts);
        void MeshPartitionerInternal(Mesh* mesh, int nparts);
        void WritePartitionInternal(Mesh* mesh);

    private:
        int  n_partitions;  // Número de partições.
        int* nodal_part;    // Informações nodais da partição.
        int* elem_part;     // Informações elementares da partição.
};

#endif // MESHPART_H