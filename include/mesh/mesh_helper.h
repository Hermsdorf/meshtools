#ifndef MESH_HELPER_H
#define MESH_HELPER_H

#include <map>
#include <string>

namespace MeshHelper{

static unsigned int edge2_faces[2] = {0,1};

//unsigned int quad4_faces[4][2] = {{0,1},{1,2},{2,3},{3,0}};
static unsigned int quad4_faces[8] = {0,1,1,2,2,3,3,0}; // 2D

//unsigned int tri3_faces[3][2]  = {{0,1},{1,2},{2,0}};
static unsigned int tri3_faces[6]  = {0,1,1,2,2,0}; // 2D

//unsigned int tet4_faces[4][3]  = {{0,2,1},{0,3,2},{0,1,3},{1,2,3}};
static unsigned int tet4_faces[12]  = {0,2,1,0,3,2,0,1,3,1,2,3}; // 3D

//unsigned int hex8_faces[6][4]  = {{0,1,2,3},{4,5,6,7},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7}};
static unsigned int hex8_faces[24]  = {0,1,2,3,4,5,6,7,0,1,5,4,1,2,6,5,2,3,7,6,3,0,4,7}; // 3D

// Gmsh Elements  GMSH iD    VTK ID     Vtk_Nome
// Edge2          1          3          VTK_LINE
// Tri3           2          5          VTK_TRIANGLE
// Quad4          3          9          VTK_QUAD
// TET4           4          10         VTK_TETRA
// Hex8           5          12         VTK_HEXAHEDRON

static std::map<int,int> GmshIdToVtkId          = {{1,3},{2,5},{3,9},{4,10},{5,12}};
static std::map<int,int> GmshIdToNumberOfNodes  = {{1,2},{2,3},{3,4},{4,4},{5,8}};
static std::map<int,int> GmshIdToElementDim     = {{1,1},{2,2},{3,2},{4,3},{5,3}};

static std::map<int,int> VtkIdToGmshId          = {{3,1},{5,2},{9,3},{10,4},{12,5}};
static std::map<int,std::string> GmshIdToVtkStr = {{1,"VTK_LINE"},{2,"VTK_TRIANGLE"},{3,"VTK_QUAD"},{4,"VTK_TETRA"},{5,"VTK_HEXAHEDRON"}};
static std::map<int,std::string> VtkIdToVtkStr  = {{3,"VTK_LINE"},{5,"VTK_TRIANGLE"},{9,"VTK_QUAD"},{10,"VTK_TETRA"},{12,"VTK_HEXAHEDRON"}};

static std::map<int ,int> VTKIdToNumberFaces     =  {{3,2},{5,3},{9,4},{10,4},{12,8}};
static std::map<int ,int> VTKIdToGmshFaceId      =  {{3,0},{5,1},{9,1},{10,2},{12,3}};
static std::map<int ,int> VTKIdToNumberFaceNodes =  {{3,1},{5,2},{9,2},{10,3},{12,4}};

static std::map<int ,unsigned int*> VtkIdToFaceMap =   {{3, edge2_faces}, {5,tri3_faces}, 
                                                        {9,quad4_faces}, {10,tet4_faces}, {12,hex8_faces}};

}

// GmshIdToVtkId[2] retorna 5 


#endif /* MESH_HELPER_H */
