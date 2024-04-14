#ifndef EDGE_H__
#define EDGE_H__

class Edge
{
public:
    Edge(unsigned int vert1, unsigned int vert2){v1 = vert1; v2 = vert2; divided = false;};
    Edge(){};
    ~Edge(){};
    unsigned int id;
    unsigned int v1, v2;
    bool divided;
};

#endif // EDGE_H__