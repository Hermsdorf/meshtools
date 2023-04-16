#ifndef ELEMENT_H__
#define ELEMENT_H__

#include "mesh.h"

class Element
{
    friend class Mesh;
    public:
        Element(){};
        std::vector<unsigned int>&  connectivity() {return _conn; };
        Point&                      node(int i) {return _coords[i]; } ;
        unsigned short&             type() {return _type; } ;
        unsigned int&               region()  {return _tag; } ;
        unsigned int                n_nodes() {return _conn.size(); };
        Point                       calculate_centroid();
    private:
        std::vector<unsigned int> _conn;
        std::vector<Point>        _coords;
        unsigned short            _type;
        unsigned int              _tag;
        
};

class SurfaceElement: public Element
{
    public:
        SurfaceElement(){};
        SurfaceElement(Element& e) { _internal_elem = e; };
        Element&    get_internal_element() {return _internal_elem; };
        void        set_internal_element(Element& e) {_internal_elem = e; };

        RealVector  calculate_normal();
        
    private:
        Element _internal_elem;
};

#endif // ELEMENT_H__
