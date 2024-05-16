
#include "mesh_helper.h"

namespace MeshHelper {

void triangle_face_connectivity(int face, std::vector<unsigned int> &conn, std::vector<unsigned int> &face_conn)
{
    face_conn[0] = conn[MeshHelper::tri3_faces[2*face + 0]];
    face_conn[1] = conn[MeshHelper::tri3_faces[2*face + 1]];
}           


void quad_face_connectivity(int face, std::vector<unsigned int> &conn, std::vector<unsigned int> &face_conn)
{
    face_conn[0] = conn[MeshHelper::quad4_faces[2*face + 0]];
    face_conn[1] = conn[MeshHelper::quad4_faces[2*face + 1]];
}

void tetrahedron_edge_connectivity(int edge, std::vector<unsigned int> &conn, std::vector<unsigned int> &edge_conn)
{
    edge_conn[0] = conn[MeshHelper::tet4_edges[2*edge + 0]];
    edge_conn[1] = conn[MeshHelper::tet4_edges[2*edge + 1]];
}

void tetrahedron_face_connectivity(int face, std::vector<unsigned int> &conn, std::vector<unsigned int> &face_conn)
{
    face_conn[0] = conn[MeshHelper::tet4_faces[3*face + 0]];
    face_conn[1] = conn[MeshHelper::tet4_faces[3*face + 1]];
    face_conn[2] = conn[MeshHelper::tet4_faces[3*face + 2]];
}

void hexahedron_face_connectivity(int face, std::vector<unsigned int> &conn, std::vector<unsigned int> &face_conn)
{
    face_conn[0] = conn[MeshHelper::hex8_faces[4*face + 0]];
    face_conn[1] = conn[MeshHelper::hex8_faces[4*face + 1]];
    face_conn[2] = conn[MeshHelper::hex8_faces[4*face + 2]];
    face_conn[3] = conn[MeshHelper::hex8_faces[4*face + 3]];
}

void hexahedron_edge_connectivity(int edge, std::vector<unsigned int> &conn, std::vector<unsigned int> &edge_conn)
{
    edge_conn[0] = conn[MeshHelper::hex8_edges[2*edge + 0]];
    edge_conn[1] = conn[MeshHelper::hex8_edges[2*edge + 1]];
}

}

