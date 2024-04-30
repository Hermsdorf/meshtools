#ifndef MESH_REORDERING_H
#define MESH_REORDERING_H

#include <memory>

#include "mesh.h"

class MeshReordering
{

    enum class ReorderingMode{RCM=0, ND=1};

    MeshReordering() = delete;
    MeshReordering(const MeshReordering& ) = delete;
    public: 
        static void reordering(std::unique_ptr<Mesh>& mesh, ReorderingMode mode = ReorderingMode::RCM);    

    protected:

        static void apply_rcm_ordering(std::unique_ptr<Mesh>& mesh);

        static void apply_nd_ordering(std::unique_ptr<Mesh>& mesh);

        static void apply_reordering(std::unique_ptr<Mesh>& mesh, int *perm, int* iperm);

};


#endif /* MESH_REORDERING_H */
