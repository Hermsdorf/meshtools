#ifndef ELEMENT_H
#define ELEMENT_H

#include "mesh.h"
#include "numeric_vector.h"

class Element
{
    friend class Mesh;
    public:
        static unique_ptr<Element> New()
        {
            return std::unique_ptr<Element> (new Element());
        }
        Element(){};
        Element(const Element& el) = delete;

        void reset(unsigned int index, std::vector<Point> &vertices, std::vector<unsigned int> &topo, unsigned short type, unsigned int tag);
        std::vector<unsigned int>&  connectivity() {return _conn; };
        Point&                      node(int i) {return _coords[i]; } ;
        unsigned short&             type() {return _type; } ;
        unsigned int&               region()  {return _tag; } ;
        unsigned int                n_nodes() {return _conn.size(); };
        Point                       calculate_centroid();
        double                      calculate_h();
    private:
        std::vector<unsigned int> _conn;
        std::vector<Point>        _coords;
        unsigned short            _type;
        unsigned int              _tag;
        unsigned                  _id;
        
};

class SurfaceElement: public Element
{
    public:
        static unique_ptr<SurfaceElement> New()
        {
            return std::unique_ptr<SurfaceElement> (new SurfaceElement());
        }

        SurfaceElement(){};
        SurfaceElement(Element& e) { _internal_elem = e; };
        Element&    get_internal_element() {return _internal_elem; };
        void        set_internal_element(Element& e) {_internal_elem = e; };

        RealVector  calculate_normal();
        
    private:
        Element  _internal_elem;
};

#endif /* ELEMENT_H */
