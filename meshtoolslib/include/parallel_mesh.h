#ifndef PARALLELMESH__H_
#define PARALLELMESH__H_

#include <iostream>

#include "mesh.h"

class SharedNodes{
    public:
        SharedNodes();
        ~SharedNodes();

        unsigned int get_id_processador_vizinho();
        unsigned int get_n_shared_nodes();
        std::vector<unsigned int>& get_nodes();
        void set_id_processador_vizinho(unsigned int id_processador_vizinho);
        void set_n_shared_nodes(unsigned int n_shared_nodes);
        void set_nodes(std::vector<unsigned int> nodes);

        friend class ParallelMesh;

    private:
        unsigned int id_processador_vizinho;  // trocar para ingles
        unsigned int n_shared_nodes;
        std::vector<unsigned int> nodes; // lista de nos

        
};

class ParallelMesh : public Mesh{
    public:
        ParallelMesh();
        ~ParallelMesh();

        void readParallelMesh(const char* filename);
        void readParallelMeshBin(const char* filename);
        void writeParallelMesh();
        void writeParallelMeshBin();

        std::vector<unsigned int>& get_local_to_global();
        int get_n_processadores_vizinhos();
        std::vector<SharedNodes>& get_communication_map();
        void set_local_to_global(std::vector<unsigned int> local_to_global);
        void set_n_processadores_vizinhos(int n_processadores_vizinhos);
        void set_communication_map(std::vector<SharedNodes> communication_map);
        bool get_internal_mesh();
        void set_internal_mesh(bool internal_mesh);

    private:
        std::vector<unsigned int> local_to_global;
        int n_processadores_vizinhos; // trocar para ingles
        std::vector<SharedNodes> communication_map;
        bool internal_mesh;

        void writePvtu();
};

#endif // PARALLELMESH__H_