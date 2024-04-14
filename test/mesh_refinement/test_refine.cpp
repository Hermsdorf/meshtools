#include "meshtools.h"
#include "mesh.h"
#include "edge.h"

#define LINE 1
#define TRIANGLE 2
#define QUADRANGLE 3
#define TETRAHEDRON 4
#define HEXAHEDRON 5

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

int Tri3Edge[3][2] = {{0, 1}, {1, 2}, {2, 0}};
    int Quad4Edge[4][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
    int Tetra6Edge[6][2] = {{0, 1}, {0, 3}, {1, 2}, {1, 3}, {2, 0}, {2, 3}};
    int Hexa8Edge[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

unordered_map<unsigned long int, Edge> EdgeMap;

//-------------------------------- Keys functons --------------------------------//
unsigned int EdgeKey(unsigned int a, unsigned int b)
{
    if (a < b)
    {
        swap(a, b);
    }
    unsigned long c = 0;
    mix(a, b, c);
    final(a, b, c);
    return c;
}

//------------------------------ END Keys functons ------------------------------//

void Mesh::GetEdges()
{
  for(int i = 0; i < get_n_face_elements(); i++)
  {
    if(getElementType(i) == LINE)
    {
      vector<unsigned int> conn;
      unsigned int key ;
      get_surface_element_connectivity(i, conn);
      key = EdgeKey(conn[0], conn[1]);
      if(EdgeMap.find(key) == EdgeMap.end())
      {
        Edge e (conn[0], conn[1]);
        EdgeMap [key] = e;
      }
    }// END LINE

    if(getElementType(i == TRIANGLE))
    {
      vector<unsigned int> conn;
      unsigned long key;
      if(dim = 2)
      {
        get_element_connectivity(i, conn);
      }
      if(dim = 3)
      {
        get_surface_element_connectivity(i, conn);
      }
      
      for(int j = 0; j < 3; j++)
      {
        unsigned int nl1 = Tri3Edge[j][0];
        unsigned int nl2 = Tri3Edge[j][1];

        unsigned int ng1 = conn[nl1];
        unsigned int ng2 = conn[nl2];

        key = EdgeKey(ng1, ng2);
        if(EdgeMap.find(key) == EdgeMap.end())
        {
          Edge e(ng1, ng2);
          EdgeMap[key] = e;
        }
      }
    }// END TRIANGLE

    if(getElementType(i == QUADRANGLE))
    {
      vector<unsigned int> conn;
      unsigned long key;
      if(dim = 2)
      {
        get_element_connectivity(i, conn);
      }
      if(dim = 3)
      {
        get_surface_element_connectivity(i, conn);
      }
      for(int j = 0; j < 4; j++)
      {
        unsigned int nl1 = Quad4Edge[j][0];
        unsigned int nl2 = Quad4Edge[j][1];

        unsigned int ng1 = conn[nl1];
        unsigned int ng2 = conn[nl2];

        key = EdgeKey(ng1, ng2);
        if(EdgeMap.find(key) == EdgeMap.end())
        {
          Edge e(ng1, ng2);
          EdgeMap[key] = e;
        }
      }
    }// EDN QUADRANGLE

    if(getElementType(i == TETRAHEDRON))
    {
      vector<unsigned int> conn;
      unsigned long key;
      get_element_connectivity(i, conn);
      for(int j = 0; j < 6; j++)
      {
        unsigned int nl1 = Tetra6Edge[j][0];
        unsigned int nl2 = Tetra6Edge[j][1];

        unsigned int ng1 = conn[nl1];
        unsigned int ng2 = conn[nl2];

        key = EdgeKey(ng1, ng2);
        if(EdgeMap.find(key) == EdgeMap.end())
        {
          Edge e(ng1, ng2);
          EdgeMap[key] = e;
        }
      }
    }// EDN TETRAHEDRON

    if (getElementType(i == HEXAHEDRON))
    {
      vector<unsigned int> conn;
      unsigned long key;
      get_element_connectivity(i, conn);
      for(int j = 0; j < 12; j++)
      {
        unsigned int nl1 = Hexa8Edge[j][0];
        unsigned int nl2 = Hexa8Edge[j][1];

        unsigned int ng1 = conn[nl1];
        unsigned int ng2 = conn[nl2];

        key = EdgeKey(ng1, ng2);
        if(EdgeMap.find(key) == EdgeMap.end())
        {
          Edge e(ng1, ng2);
          EdgeMap[key] = e;
        }
      }
    }// EDN HEXAHEDRON

  }
}

int main(int argc, char *argv[])
{
    //PetscErrorCode ierr;

    Mesh mesh; 
    //mesh.read("test/mesh_refinement/mesh.msh");
    //mesh.refine();

    return 0;
}
