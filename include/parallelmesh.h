#include <iostream>
#include "mesh.h"

#ifndef PARALLELMESH__H_
#define PARALLELMESH__H_

class SharedNodes{
    public:
        SharedNodes();
        ~SharedNodes();
        
    private:
        unsigned int id_processador_vizinho;
        unsigned int n_share_nodes;
        std::vector<unsigned int> nodes; // lista de nos
};

class ParallelMesh : public Mesh{
    public:
        ParallelMesh();
        ~ParallelMesh();

        void readParallelMesh();
        void readParallelMeshBin();

    private:
        std::vector<unsigned int> local_to_global;
        int n_processadores_vizinhos;
        std::vector<SharedNodes> communication_map;
};

#endif // PARALLELMESH__H_