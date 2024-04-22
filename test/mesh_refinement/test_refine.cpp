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

//Elements templates
int Tri3Edge[3][2] = {{0, 1}, {1, 2}, {2, 0}};
int Quad4Edge[4][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
int Tetra6Edge[6][2] = {{0, 1}, {0, 3}, {1, 2}, {1, 3}, {2, 0}, {2, 3}};
int Hexa8Edge[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

// Refine Elements templates
int RefineTriangle[4][3] = {{0, 3, 5}, {3, 4, 5}, {3, 1, 4}, {5, 4, 2}};
int RefineSquare[4][4] = {{0, 4, 8, 7}, {4, 1, 5, 8}, {8, 5, 2, 6}, {7, 8, 6, 3}};
int RefineTetrahedron[8][4] = {{0, 4, 6, 7}, {4, 1, 5, 9}, {6, 5, 2, 8}, {7, 9, 8, 3},
                               {4, 6, 7, 9}, {4, 9, 5, 6}, {6, 7, 9, 8}, {6, 8, 9, 5}};
int RefineHexahedron[8][8] = {{0, 8, 24, 11, 16, 20, 26, 23}, {16, 20, 26, 23, 4, 12, 25, 15}, 
                              {8, 1, 9, 24, 20, 17, 21, 26}, {20, 17, 21, 26, 12, 5, 13, 25},
                              {11, 24, 10, 3, 23, 26, 22, 19}, {23, 26, 22, 19, 15, 25, 14, 7},
                              {24, 9, 2, 10, 26, 21, 18, 22}, {26, 21, 18, 22, 25, 13, 6, 14}};
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

//------------------------------ Midpoint Functions -----------------------------//

Point EdgeMidPoint(Point p1, Point p2)
{
  Point p;
  p = p1.operator+(p2);
  p.operator/=(2);
  return p;
}

Point SquareMidPoint(Point p1, Point p2, Point p3, Point p4)
{
  Point p(0,0,0);
  p.operator+=(p1);
  p.operator+=(p2);
  p.operator+=(p3);
  p.operator+=(p4);
  p.operator/=(4);
  return p;
}

//---------------------------- END Midpoint Functions ---------------------------//

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

void Mesh:: refine(int n_refinaments)
{
  for(int n = 0; n < n_refinaments; n++)
  {
    vector <unsigned int> new_conn;
    vector <unsigned int> new_offset;

    if (dim == 2)
    {
      for(int i = 0; i < get_n_face_elements(); i++)
      {
        if(getElementType(i) == LINE)
        {
          vector<unsigned int> new_elements_conn = LINE_refine(i, dim);
          for (int j = 0; j < 2; j++)
          {
            new_offset.emplace_back(new_conn.size());
            new_conn.emplace_back(new_elements_conn[0 + j]);
            new_conn.emplace_back(new_elements_conn[1 + j]);
          }//Como linkar tags e outras caracteristicas do elemento anterior para os novos?
        }

        if(getElementType(i) == TRIANGLE)
        {
          vector<unsigned int> new_elements_conn = TRIANGLE_refine(i, dim);
          for(int i = 0; i< 4; i++)
          {
            new_offset.emplace_back(new_conn.size());
            unsigned int idx = i*3;
            new_conn.emplace_back(new_elements_conn[idx + 1]);
            new_conn.emplace_back(new_elements_conn[idx + 2]);
            new_conn.emplace_back(new_elements_conn[idx + 3]);
          }
        }
        if(getElementType(i) == QUADRANGLE)
        {}


      }
    }
  }
  
}

vector<unsigned int> Mesh::LINE_refine(unsigned int element_id, int dimension)
{
  vector<unsigned int> conn;
  vector<Point> points;
  get_surface_element_connectivity(element_id, conn);
  get_surface_element_coordinates(element_id, points);
  unsigned long key = EdgeKey(conn[0], conn[1]);
  unsigned long NewVertexId[3];
  NewVertexId[0] = conn[0];
  NewVertexId[2] = conn[1];
  if(EdgeMap[key].divided == false)
  {
    n_nodes++;
    Point p = EdgeMidPoint(points[0], points[1]);
    EdgeMap[key].divided = true;
    EdgeMap[key].new_node_id = n_nodes;
    node_index.push_back(n_nodes);
    NewVertexId[1] = n_nodes;
    for(int i = 0; i < 3; i++)
    {
      coord.push_back(p.operator()(i));
    }
  }
  else
  {
    NewVertexId[1] = EdgeMap[key].new_node_id;
  }
  vector<unsigned int> new_elements_coon;
  for(int i = 0; i < 2; i++)
  {
    new_elements_coon.emplace_back(NewVertexId[i+1]);
    new_elements_coon.emplace_back(NewVertexId[i+1]);
  }

  return new_elements_coon;  
}

vector<unsigned int> Mesh::TRIANGLE_refine(unsigned int element_id, unsigned int dim)
{
  vector<unsigned int> conn;
  vector<Point> points;
  unsigned long NewVertexId[3];
  if(dim == 2)
  {
    get_surface_element_connectivity(element_id, conn);
    get_surface_element_coordinates(element_id, points);
  }
  if(dim == 3)
  {
    get_element_connectivity(element_id, conn);
    get_element_coordinates(element_id, points);
  }

  for (int i = 0; i < 3; i++)
  {
    unsigned int nl1 = Tri3Edge[i][0];
    unsigned int nl2 = Tri3Edge[i][1];

    unsigned int ng1 = conn[nl1];
    unsigned int ng2 = conn[nl2];
    unsigned long key = EdgeKey(ng1, ng2);
    NewVertexId[i] = ng1;
    if (EdgeMap[key].divided == false)
    {
      n_nodes++;
      Point p = EdgeMidPoint(points[nl1], points[nl2]);
      EdgeMap[key].divided = true;
      EdgeMap[key].new_node_id = n_nodes;
      node_index.push_back(n_nodes);
      NewVertexId[3 + i] = n_nodes;
      for (int i = 0; i < 3; i++)
      {
        coord.push_back(p.operator()(i));
      }
    }
    else
    {
      NewVertexId[3 + i] = EdgeMap[key].new_node_id;
    }

  }

  vector<unsigned int> new_elements_coon;
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      unsigned int idx = RefineTriangle[i][j];
      new_elements_coon.emplace_back(idx);
    }      
  }

  return new_elements_coon;
}



int main(int argc, char *argv[])
{
  //PetscErrorCode ierr;

  Mesh m; 
  //mesh.read("test/mesh_refinement/mesh.msh");
  //mesh.refine();

    return 0;
}
