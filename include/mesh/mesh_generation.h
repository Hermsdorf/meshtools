#ifndef MESH_GENERATION_H__
#define MESH_GENERATION_H__

#include "meshtools.h"
#include "mesh.h"

class MeshGeneration 
{
    public:
    MeshGeneration() = delete;
    MeshGeneration(const MeshGeneration&) = delete;

    static std::unique_ptr<Mesh> build_square(unsigned int nx, unsigned int ny, double xmin, double xmax, double ymin, double ymax);

};

#endif /* MESH_GENERATION_H__ */
