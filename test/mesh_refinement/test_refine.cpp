#include "meshtools.h"
#include "mesh.h"
#include "edge.h"
#include <unordered_map>

#define LINE 1
#define TRIANGLE 2
#define QUADRANGLE 3
#define TETRAHEDRON 4
#define HEXAHEDRON 5

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define mix(a, b, c) \
  {                  \
    a -= c;          \
    a ^= rot(c, 4);  \
    c += b;          \
    b -= a;          \
    b ^= rot(a, 6);  \
    a += c;          \
    c -= b;          \
    c ^= rot(b, 8);  \
    b += a;          \
    a -= c;          \
    a ^= rot(c, 16); \
    c += b;          \
    b -= a;          \
    b ^= rot(a, 19); \
    a += c;          \
    c -= b;          \
    c ^= rot(b, 4);  \
    b += a;          \
  }

#define final(a, b, c) \
  {                    \
    c ^= b;            \
    c -= rot(b, 14);   \
    a ^= c;            \
    a -= rot(c, 11);   \
    b ^= a;            \
    b -= rot(a, 25);   \
    c ^= b;            \
    c -= rot(b, 16);   \
    a ^= c;            \
    a -= rot(c, 4);    \
    b ^= a;            \
    b -= rot(a, 14);   \
    c ^= b;            \
    c -= rot(b, 24);   \
  }

// Elements templates
int Tri3Edge[3][2] = {{0, 1}, {1, 2}, {2, 0}};
int Quad4Edge[4][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
int Tetra6Edge[6][2] = {{0, 1}, {0, 3}, {1, 2}, {1, 3}, {2, 0}, {2, 3}};
int Hexa12Edge[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

// Refine Elements templates
int RefineLine [2][2] = {{0,1}, {1,2}};
int RefineTriangle[4][3] = {{0, 3, 5}, {3, 4, 5}, {3, 1, 4}, {5, 4, 2}};
int RefineQuadrangle[4][4] = {{0, 4, 8, 7}, {4, 1, 5, 8}, {8, 5, 2, 6}, {7, 8, 6, 3}};
int RefineTetrahedron[8][4] = {{0, 4, 6, 7}, {4, 1, 5, 9}, {6, 5, 2, 8}, {7, 9, 8, 3}, {4, 6, 7, 9}, {4, 9, 5, 6}, {6, 7, 9, 8}, {6, 8, 9, 5}};
int RefineHexahedron[8][8] = {{0, 8, 24, 11, 16, 20, 26, 23}, {16, 20, 26, 23, 4, 12, 25, 15}, {8, 1, 9, 24, 20, 17, 21, 26}, {20, 17, 21, 26, 12, 5, 13, 25}, {11, 24, 10, 3, 23, 26, 22, 19}, {23, 26, 22, 19, 15, 25, 14, 7}, {24, 9, 2, 10, 26, 21, 18, 22}, {26, 21, 18, 22, 25, 13, 6, 14}};
//-------------------------------

unordered_map<unsigned long int, Edge> EdgeMap;
unordered_map<unsigned long int, unsigned int> cpQuadrangle;
unordered_map<unsigned long int, unsigned int> cpHexaedron;

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

unsigned long QuadKey(unsigned long a, unsigned long b,
                      unsigned long c, unsigned long d)
{
  unsigned long vet[4] = {a, b, c, d};
  sort(vet, vet + 4);
  mix(vet[0], vet[1], vet[2]);
  vet[0] += vet[3];
  final(vet[0], vet[1], vet[2]);
  return vet[2];
}

unsigned long HexaKey(unsigned long a, unsigned long b,
                      unsigned long c, unsigned long d,
                      unsigned long e, unsigned long f,
                      unsigned long g, unsigned long h)
{
  unsigned long vet[8] = {a, b, c, d, e, f, g, h};

  std::sort(vet, vet + 8);
  mix(vet[0], vet[1], vet[2]);
  vet[0] += vet[3];
  vet[1] += vet[4];
  vet[2] += vet[5];
  mix(vet[0], vet[1], vet[2]);
  vet[0] += vet[6];
  vet[1] += vet[7];
  final(vet[0], vet[1], vet[2]);
  return vet[2];
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

Point QuadMidPoint(Point p1, Point p2, Point p3, Point p4)
{
  Point p(0, 0, 0);
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
  if (dim == 2)
  {
    for (int i = 0; i < get_n_face_elements(); i++)
    {
      if (gmshEleType[i] == LINE)
      {
        vector<unsigned int> conn;
        unsigned int key;
        get_surface_element_connectivity(i, conn);
        Edge e(conn[0], conn[1]);
        key = EdgeKey(e.v1, e.v2);
        if (EdgeMap.find(key) == EdgeMap.end())
        {
          EdgeMap[key] = e;
        }
      } // END LINE
    }

    for (int i = 0; i < get_n_elements(); i++)
    {
      if (gmshEleType[i + n_face_elements] == TRIANGLE)
      {
        vector<unsigned int> conn;
        unsigned long key;
        get_element_connectivity(i, conn);

        for (int j = 0; j < 3; j++)
        {
          unsigned int nl1 = Tri3Edge[j][0];
          unsigned int nl2 = Tri3Edge[j][1];

          unsigned int ng1 = conn[nl1];
          unsigned int ng2 = conn[nl2];

          Edge e(ng1, ng2);
          key = EdgeKey(e.v1, e.v2);
          if (EdgeMap.find(key) == EdgeMap.end())
          {
            EdgeMap[key] = e;
          }
        }
      }

      if (gmshEleType[i + n_face_elements] == QUADRANGLE)
      {
        vector<unsigned int> conn;
        unsigned long key;
        get_element_connectivity(i, conn);
        for (int j = 0; j < 4; j++)
        {
          unsigned int nl1 = Quad4Edge[j][0];
          unsigned int nl2 = Quad4Edge[j][1];

          unsigned int ng1 = conn[nl1];
          unsigned int ng2 = conn[nl2];

          key = EdgeKey(ng1, ng2);
          if (EdgeMap.find(key) == EdgeMap.end())
          {
            Edge e(ng1, ng2);
            EdgeMap[key] = e;
          }
        }
      }
    }
  }

  if(dim == 3)
  {
    for(int i = 0; i < n_face_elements; i++)
    {
      if (gmshEleType[i] == TRIANGLE)
      {
        vector<unsigned int> conn;
        unsigned long key;
        get_surface_element_connectivity(i, conn); 

        for (int j = 0; j < 3; j++)
        {
          unsigned int nl1 = Tri3Edge[j][0];
          unsigned int nl2 = Tri3Edge[j][1];

          unsigned int ng1 = conn[nl1];
          unsigned int ng2 = conn[nl2];

          Edge e(ng1, ng2);
          key = EdgeKey(e.v1, e.v2);
          if (EdgeMap.find(key) == EdgeMap.end())
          {
            EdgeMap[key] = e;
          }
        }
      }

      if (gmshEleType[i] == QUADRANGLE)
      {
        vector<unsigned int> conn;
        unsigned long key;
        get_surface_element_connectivity(i, conn);
        for (int j = 0; j < 4; j++)
        {
          unsigned int nl1 = Quad4Edge[j][0];
          unsigned int nl2 = Quad4Edge[j][1];

          unsigned int ng1 = conn[nl1];
          unsigned int ng2 = conn[nl2];

          key = EdgeKey(ng1, ng2);
          if (EdgeMap.find(key) == EdgeMap.end())
          {
            Edge e(ng1, ng2);
            EdgeMap[key] = e;
          }
        }
      }
    }

    for(int i = 0; i< n_elements; i++)
    {
      if(gmshEleType[i + n_face_elements] == TETRAHEDRON)
      {
        vector<unsigned int> conn;
        unsigned long key;
        get_element_connectivity(i,conn);
        for(int j = 0; j < 6; j++)
        {
          unsigned int nl1 = Tetra6Edge[j][0];
          unsigned int nl2 = Tetra6Edge[j][1];

          unsigned int ng1 = conn[nl1];
          unsigned int ng2 = conn[nl2];

          key = EdgeKey(ng1, ng2);
          if (EdgeMap.find(key) == EdgeMap.end())
          {
            Edge e(ng1, ng2);
            EdgeMap[key] = e;
          }
        }
      }

      if(gmshEleType[i + n_face_elements] == HEXAHEDRON)
      {
        vector<unsigned int> conn;
        unsigned long key;
        get_element_connectivity(i,conn);
        for(int j = 0; j < 12; j++)
        {
          unsigned int nl1 = Hexa12Edge[j][0];
          unsigned int nl2 = Hexa12Edge[j][1];

          unsigned int ng1 = conn[nl1];
          unsigned int ng2 = conn[nl2];

          key = EdgeKey(ng1, ng2);
          if (EdgeMap.find(key) == EdgeMap.end())
          {
            Edge e(ng1, ng2);
            EdgeMap[key] = e;
          }
        }
      }
    }
  }
}

void Mesh::refine(int n_refinaments)
{
  vector<unsigned int> new_conn;
  vector<unsigned int> new_offset;
  vector<unsigned short> new_element_type;
  vector<unsigned int > new_GmshElement_type;
  vector<int> new_element_physical_tag;
  unsigned int new_n_surface_elements = 0;
  unsigned int new_n_elements = 0;
  //GetEdges();
  for (int n = 0; n < n_refinaments; n++)
  {

    if (dim == 2)
    {
      for (int i = 0; i < get_n_face_elements(); i++)
      {
        if (gmshEleType[i] == LINE)
        {
          vector<unsigned int> new_elements_conn = LINE_refine(i, dim);
          for (int j = 0; j < 2; j++)
          {
            unsigned int idx = j * 2;
            new_offset.emplace_back(new_conn.size());
            new_element_type.emplace_back(type[i]);
            new_GmshElement_type.emplace_back(gmshEleType[i]);
            new_element_physical_tag.emplace_back(physical_tag[i]);
            new_conn.emplace_back(new_elements_conn[idx + 0]);
            new_conn.emplace_back(new_elements_conn[idx + 1]);
          }
          new_n_surface_elements += 2;
        }
      }

      for (int i = 0; i < get_n_elements(); i++)
      {
        if (gmshEleType[i + n_face_elements] == TRIANGLE)
        {
          vector<unsigned int> new_elements_conn = TRIANGLE_refine(i, dim);
          for (int j = 0; j < 4; j++)
          {
            new_offset.emplace_back(new_conn.size());
            unsigned int idx = j * 3;
            new_element_type.emplace_back(type[i]);
            new_GmshElement_type.emplace_back(gmshEleType[i + n_face_elements]);
            new_element_physical_tag.emplace_back(physical_tag[i + n_face_elements]);
            new_conn.emplace_back(new_elements_conn[idx + 0]);
            new_conn.emplace_back(new_elements_conn[idx + 1]);
            new_conn.emplace_back(new_elements_conn[idx + 2]);
          }
          new_n_elements += 4;
        }
        if (gmshEleType[i + n_face_elements] == QUADRANGLE)
        {
          cout << "Entrou no quadrado"<<endl;//log
          vector<unsigned int> new_elements_conn = QUADRANGLE_refine(i, dim);
          for (int j = 0; j < 4; j++)
          {
            new_offset.emplace_back(new_conn.size());
            unsigned int idx = j * 4;
            new_element_type.emplace_back(type[i]);
            new_GmshElement_type.emplace_back(gmshEleType[i + n_face_elements]);
            new_element_physical_tag.emplace_back(physical_tag[i + n_face_elements]);
            new_conn.emplace_back(new_elements_conn[idx + 0]);
            new_conn.emplace_back(new_elements_conn[idx + 1]);
            new_conn.emplace_back(new_elements_conn[idx + 2]);
            new_conn.emplace_back(new_elements_conn[idx + 3]);
          }
          new_n_elements += 4;
        }
      }
    }

    //----------------------------------------

    if(dim == 3)
    {
      for (int i = 0; i < n_face_elements; i++)
      {
        if (gmshEleType[i] == TRIANGLE)
        {
          vector<unsigned int> new_elements_conn = TRIANGLE_refine(i, dim);
          for (int j = 0; j < 4; j++)
          {
            new_offset.emplace_back(new_conn.size());
            unsigned int idx = j * 3;
            new_element_type.emplace_back(type[i]);
            new_GmshElement_type.emplace_back(gmshEleType[i]);
            new_element_physical_tag.emplace_back(physical_tag[i]);
            new_conn.emplace_back(new_elements_conn[idx + 0]);
            new_conn.emplace_back(new_elements_conn[idx + 1]);
            new_conn.emplace_back(new_elements_conn[idx + 2]);
          }
          new_n_surface_elements += 4;
        }

        if (gmshEleType[i] == QUADRANGLE)
        {
          cout << "Entrou no quadrado"<<endl;//log
          vector<unsigned int> new_elements_conn = QUADRANGLE_refine(i, dim);
          for (int j = 0; j < 4; j++)
          {
            new_offset.emplace_back(new_conn.size());
            unsigned int idx = j * 4;
            new_element_type.emplace_back(type[i]);
            new_GmshElement_type.emplace_back(gmshEleType[i]);
            new_element_physical_tag.emplace_back(physical_tag[i]);
            new_conn.emplace_back(new_elements_conn[idx + 0]);
            new_conn.emplace_back(new_elements_conn[idx + 1]);
            new_conn.emplace_back(new_elements_conn[idx + 2]);
            new_conn.emplace_back(new_elements_conn[idx + 3]);
          }
          new_n_surface_elements += 4;
        }
      }

      for(int i = 0; i < n_elements; i++)
      {
        if(gmshEleType[i + n_face_elements] == TETRAHEDRON)
        {
          vector<unsigned int> new_elements_conn = TETRAHEDRON_refine(i, dim);
          for (int j = 0; j < 8; j++)
          {
            new_offset.emplace_back(new_conn.size());
            unsigned int idx = j * 4;
            new_element_type.emplace_back(type[i]);
            new_GmshElement_type.emplace_back(gmshEleType[i + n_face_elements]);
            new_element_physical_tag.emplace_back(physical_tag[i + n_face_elements]);
            new_conn.emplace_back(new_elements_conn[idx + 0]);
            new_conn.emplace_back(new_elements_conn[idx + 1]);
            new_conn.emplace_back(new_elements_conn[idx + 2]);
            new_conn.emplace_back(new_elements_conn[idx + 3]);
          }
          new_n_elements += 8;
        }

      }

    }
    new_offset.emplace_back(new_conn.size());
  }

  conn = new_conn;
  offset = new_offset;
  type = new_element_type;
  gmshEleType = new_GmshElement_type;
  physical_tag = new_element_physical_tag;
  n_elements = new_n_elements;
  n_face_elements = new_n_surface_elements;
  //process_face_to_element(); // Resolver
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
  if (EdgeMap[key].divided == false)
  {
    Point p = EdgeMidPoint(points[0], points[1]);
    EdgeMap[key].divided = true;
    EdgeMap[key].new_node_id = n_nodes;
    node_index.push_back(n_nodes);
    NewVertexId[1] = n_nodes;
    n_nodes++;
    for (int i = 0; i < 3; i++)
    {
      coord.push_back(p.operator()(i));
    }
  }
  else
  {
    NewVertexId[1] = EdgeMap[key].new_node_id;
  }
  vector<unsigned int> new_elements_coon;
  for (int i = 0; i < 2; i++)
  {
    new_elements_coon.emplace_back(NewVertexId[i + 0]);
    new_elements_coon.emplace_back(NewVertexId[i + 1]);
  }

  return new_elements_coon;
}

vector<unsigned int> Mesh::TRIANGLE_refine(unsigned int element_id, unsigned int dim)
{
  vector<unsigned int> conn;
  vector<Point> points;
  unsigned long NewVertexId[6];
  if (dim == 3)
  {
    get_surface_element_connectivity(element_id, conn);
    get_surface_element_coordinates(element_id, points);
  }
  if (dim == 2)
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
      Point p = EdgeMidPoint(points[nl1], points[nl2]);
      EdgeMap[key].divided = true;
      EdgeMap[key].new_node_id = n_nodes;
      node_index.push_back(n_nodes);
      NewVertexId[3 + i] = n_nodes;
      n_nodes++;
      for (int j = 0; j < 3; j++)
      {
        coord.push_back(p.operator()(j));
      }
    }
    else
    {
      NewVertexId[3 + i] = EdgeMap[key].new_node_id;
    }
  }
  
  vector<unsigned int> new_elements_coon;
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      unsigned int idx = RefineTriangle[i][j];
      new_elements_coon.emplace_back(NewVertexId[idx]);
    }
  }

  return new_elements_coon;
}

vector<unsigned int> Mesh::QUADRANGLE_refine(unsigned int element_id, unsigned int dim)
{
  vector<unsigned int> conn;
  vector<Point> points;
  unsigned long NewVertexId[9];

  if (dim == 3)
  {
    get_surface_element_connectivity(element_id, conn);
    get_surface_element_coordinates(element_id, points);
  }
  if (dim == 2)
  {
    get_element_connectivity(element_id, conn);
    get_element_coordinates(element_id, points);
  }

  for (int i = 0; i < 4; i++)
  {
    unsigned int nl1 = Quad4Edge[i][0];
    unsigned int nl2 = Quad4Edge[i][1];

    unsigned int ng1 = conn[nl1];
    unsigned int ng2 = conn[nl2];
    unsigned long key = EdgeKey(ng1, ng2);
    NewVertexId[i] = ng1;
    if (EdgeMap[key].divided == false)
    {
      Point p = EdgeMidPoint(points[nl1], points[nl2]);
      EdgeMap[key].divided = true;
      EdgeMap[key].new_node_id = n_nodes;
      node_index.push_back(n_nodes);
      NewVertexId[4 + i] = n_nodes;
      n_nodes++;
      for (int i = 0; i < 3; i++)
      {
        coord.push_back(p.operator()(i));
      }
    }
    else
    {
      NewVertexId[4 + i] = EdgeMap[key].new_node_id;
    }
  }

  unsigned int ng1 = conn[0];
  unsigned int ng2 = conn[1];
  unsigned int ng3 = conn[2];
  unsigned int ng4 = conn[3];
  unsigned long key = QuadKey(ng1, ng2, ng3, ng4);
  if (cpQuadrangle.find(key) == cpQuadrangle.end())
  {
    Point p = QuadMidPoint(points[0], points[1], points[2], points[3]);
    node_index.push_back(n_nodes);
    cpQuadrangle[key] = n_nodes;
    NewVertexId[8] = n_nodes;
    n_nodes++;
    for (int i = 0; i < 3; i++)
    {
      coord.push_back(p.operator()(i));
    }
  }
  else
  {
    NewVertexId[8] = cpQuadrangle[key];
  }

  vector<unsigned int> new_elements_coon;
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      unsigned int idx = RefineQuadrangle[i][j];
      new_elements_coon.emplace_back(NewVertexId[idx]);
    }
  }

  return new_elements_coon;
}

vector<unsigned int> Mesh::TETRAHEDRON_refine(unsigned int element_id, unsigned int dim)
{
  vector<unsigned int> conn;
  vector<Point> points;
  unsigned long NewVertexId[10];

  
  get_element_connectivity(element_id, conn);
  get_element_coordinates(element_id, points);

  for(int i = 0; i < 6; i++)
  {
    unsigned int nl1 = Tetra6Edge[i][0];
    unsigned int nl2 = Tetra6Edge[i][1];

    unsigned int ng1 = conn[nl1];
    unsigned int ng2 = conn[nl2];
    unsigned long key = EdgeKey(ng1, ng2);
    NewVertexId[i] = ng1;

    if (EdgeMap[key].divided == false)
    {
      Point p = EdgeMidPoint(points[nl1], points[nl2]);
      EdgeMap[key].divided = true;
      EdgeMap[key].new_node_id = n_nodes;
      node_index.push_back(n_nodes);
      NewVertexId[4 + i] = n_nodes;
      n_nodes++;
      for (int i = 0; i < 3; i++)
      {
        coord.push_back(p.operator()(i));
      }
    }
    else
    {
      NewVertexId[4 + i] = EdgeMap[key].new_node_id;
    }
  }

  vector<unsigned int> new_elements_coon;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      unsigned int idx = RefineTetrahedron[i][j];
      new_elements_coon.emplace_back(NewVertexId[idx]);
    }
  }

  return new_elements_coon;
}

void Mesh::report()
{
  vector<unsigned int> conn;
  int nse = get_n_face_elements();
  cout << endl<<"----Relatorio----"<<endl;
  cout<< "---Surface Elements---"<<endl;
  for(int i = 0; i< get_n_face_elements(); i++)
  {
    cout << "Element ID: "     << i << endl
         << "Element Type: " << gmshEleType[i] << endl
         << "Element Tags: " << physical_tag[i] << endl;
    get_surface_element_connectivity(i, conn);
    cout << "Element nodes: ";
    for(int j = 0; j < conn.size(); j++)
    {
      cout << conn[j] << " ";
    }
    cout << endl<<endl;
  }
  cout << endl;

  cout<< "---Elements---"<<endl;
  for(int i = 0; i< get_n_elements(); i++)
  {
    cout << "Element ID: "     << i + nse << endl
         << "Element Type: " << gmshEleType[i + nse] << endl
         << "Element Tags: "  << physical_tag[i] << endl;
    get_element_connectivity(i, conn);
    cout << "Element nodes: ";
    for(int j = 0; j < conn.size(); j++)
    {
      cout << conn[j] << " ";
    }
    cout <<endl<< "Element Edges" <<endl;
    if (gmshEleType[i + nse] == TRIANGLE)
    {
      for (int j = 0; j < 3; j++)
      {
        unsigned int nl1 = Tri3Edge[j][0];
        unsigned int nl2 = Tri3Edge[j][1];

        unsigned int ng1 = conn[nl1];
        unsigned int ng2 = conn[nl2];
        unsigned long key = EdgeKey(ng1,ng2);
        cout << "  Edge " << j 
             << ": "      << EdgeMap[key].v1
             << " -- "    << EdgeMap[key].v2
             << endl;
      }
    }
    cout << endl << endl;
  }

  cout << "gmshEleType = [";
  for (int k = 0; k < gmshEleType.size(); k++)
  { 
    cout << gmshEleType[k]<<" ";
  }
  cout << "]"<<endl;

  cout << "Conn = [";
  for (int k = 0; k < this->conn.size(); k++)
  { 
    cout << this->conn[k]<<" ";
  }
  cout << "]"<<endl;

  cout << "offset = [";
  for (int k = 0; k < offset.size(); k++)
  { 
    cout << offset[k]<<" ";
  }
  cout << "]"<<endl;
}

void Mesh::mshExport(string FileName)
{
  string line;
  ofstream file;
  file.open(FileName, ios::out);
  if (!file.is_open())
  {
    cout << "Unable to open file" << endl;
    exit(1);
  }
  /*---------------Mesh Format---------------*/
  file << "$MeshFormat"    << '\n';
  file << MeshFormat[0]    << " "
       << MeshFormat[1]    << " "
       << MeshFormat[2]    << '\n'
       << "$EndMeshFormat" << '\n';
  
  /*---------------Physical Regions---------------*/
  int phyDimension;
  string phyName;
  file << "$PhysicalNames" << '\n';
  file << numPhyGroups << '\n';
  for(int i = 0; i < numPhyGroups; i++)
  {
    phyDimension = physical_map[phyIds[i]].first;
    phyName = physical_map[phyIds[i]].second;
    file << phyDimension << " "
         << phyIds[i]    << " "
         << phyName      << '\n';
  }
  file << "$EndPhysicalNames" << '\n';

  /*---------------Nodes---------------*/
  file << "$Nodes" << '\n';
  file << n_nodes   << '\n';
  for(int i = 0; i < n_nodes; i++)
  {
    unsigned int nodeId = i ;
    unsigned short nodeOffset = i * 3;
    unsigned short X = (nodeOffset + 0), Y = (nodeOffset + 1), Z = (nodeOffset + 2);
    file << nodeId   << " "
         << coord[X] << " "
         << coord[Y] << " "
         << coord[Z] << '\n';
  }
  file << "$EndNodes" << '\n';

  /*--------------Elements--------------*/
  unsigned int nTotElements = n_face_elements + n_elements;
  vector<unsigned int> elementConn;
  unsigned int conSize;
  file << "$Elements"  << '\n';
  file << nTotElements << '\n';
  unsigned short numTags = 2, tag1, tag2 = 1;
  for(int i = 0; i < nTotElements; i++)
  {
    unsigned int elementID = i +1;
    tag1 = physical_tag[i];
    conSize = offset[i+1] - offset[i];
    file << elementID      << " "
         << gmshEleType[i] << " "
         << numTags        << " "
         << tag1           << " "
         << tag2           << " ";
    for(int j = 0; j < conSize; j++)
    {
      file << (conn[(offset[i] + j)])<< " ";
    }
    file << '\n';    
  }
  file << "$EndElements";
  file.close();

}



int main(int argc, char *argv[])
{
  Mesh mesh("mesh.msh");
  // mesh.read();
  mesh.GetEdges();
  mesh.report();
  cout << "Aplicando refinamento x1..."<<endl;
  mesh.refine(1);
  cout << "Malha refinada!!!"<<endl;
  mesh.GetEdges();
  mesh.report();
  cout << "Exportando Malha refinada..."<<endl;
  mesh.mshExport("RefinedMesh.msh");
  cout << "Malha exportada com sucesso!!"<<endl;

  return 0;
}
