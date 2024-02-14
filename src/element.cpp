#include "element.h"

Point Element::calculate_centroid()
{
    Point centroid;
    unsigned int n_nodes = this->n_nodes();
    float tmp = 1.0/n_nodes;

    for (int i = 0; i < n_nodes;  i++) {
        centroid(0) += this->node(i)(0)*tmp;
        centroid(1) += this->node(i)(1)*tmp;
        centroid(2) += this->node(i)(2)*tmp;
    }

    return centroid;
}

double _calculate_h_tet4(Element &elem)
{   
    // Vectors A, B and D are respectively the
    // vectors from node 0 to nodes 1, 2 and 3.
    RealVector A(
        elem.node(1)(0) - elem.node(0)(0),
        elem.node(1)(1) - elem.node(0)(1),
        elem.node(1)(2) - elem.node(0)(2)
    );
    
    RealVector B(
        elem.node(2)(0) - elem.node(0)(0),
        elem.node(2)(1) - elem.node(0)(1),
        elem.node(2)(2) - elem.node(0)(2)
    );

    RealVector D(
        elem.node(3)(0) - elem.node(0)(0),
        elem.node(3)(1) - elem.node(0)(1),
        elem.node(3)(2) - elem.node(0)(2)
    );

    RealVector AxB = A.cross_product(B);
    double AxBD = AxB.dot_product(D);
    double volume = AxBD*0.166666667; // AxBD/6
    double sphere_radius = std::cbrt(0.75*volume*0.318309886); // 0.318309886 = 1/pi
    double h = 2*sphere_radius;

    return h;
}

double _calculate_h_quad4(Element &elem)
{
    // Characteristic length for QUAD4 elements
    // is the length of the diagonal of the element
    RealVector A(
        elem.node(2)(0) - elem.node(0)(0),
        elem.node(2)(1) - elem.node(0)(1),
        elem.node(2)(2) - elem.node(0)(2)
    );

    return A.norm();
}

double _calculate_h_tri3(Element &elem)
{
    // Characteristic length for TRI3 elements
    // is the length of the longest edge of the element
    RealVector A(
        elem.node(1)(0) - elem.node(0)(0),
        elem.node(1)(1) - elem.node(0)(1),
        elem.node(1)(2) - elem.node(0)(2)
    );

    RealVector B(
        elem.node(2)(0) - elem.node(0)(0),
        elem.node(2)(1) - elem.node(0)(1),
        elem.node(2)(2) - elem.node(0)(2)
    ); 

    RealVector C(
        elem.node(2)(0) - elem.node(1)(0),
        elem.node(2)(1) - elem.node(1)(1),
        elem.node(2)(2) - elem.node(1)(2)
    );  

    double   h = A.norm();
    h = std::max(h, B.norm());
    h = std::max(h, C.norm());
    return h;

}

double Element::calculate_h()
{
    double h = 0.0;
    switch (this->type())
    {
        case TRI3:
            h = _calculate_h_tri3(*this);
            break;
        case QUAD4:
            h = _calculate_h_quad4(*this);
            break;
        case TET4:
            h = _calculate_h_tet4(*this);
            break;
        default:
            std::cout << "[element.cpp] h characteristic for element type " << this->type() << " not implemented yet" << std::endl;
            break;
    }

    assert(h > 0.0);

    return h;
}

void Get2DElementNormal(SurfaceElement& elem, RealVector& normal_vector)
{
    RealVector vec_a, vec_b, vec_c;
    
    unsigned int n_nodes = elem.n_nodes();
    vec_a = elem.node(1) - elem.node(0);
    vec_b = elem.node(2) - elem.node(0);

    normal_vector = vec_a.cross_product(vec_b);

    Element& internal_elem  = elem.get_internal_element();
    Point internal_centroid = internal_elem.calculate_centroid();
    vec_c = internal_centroid - elem.node(0);
    if (normal_vector.dot_product(vec_c) > 0)
    {
        normal_vector(0) = -normal_vector(0);
        normal_vector(1) = -normal_vector(1);
        normal_vector(2) = -normal_vector(2);
    }

    normal_vector.unit();
}

void Get1DElementNormal(SurfaceElement& elem, RealVector& normal_vector)
{
    RealVector vec_a, vec_c;
    vec_a = elem.node(1) - elem.node(0);
    
    // Rotacionando vec_a em 90 graus:
    normal_vector(0) = -vec_a(1);
    normal_vector(1) = vec_a(0);

    Element& internal_elem  = elem.get_internal_element();
    Point internal_centroid = internal_elem.calculate_centroid();
    vec_c = internal_centroid - elem.node(0);
    //std::cout << "vec_c: " << vec_c << std::endl;
    if (normal_vector.dot_product(vec_c) > 0)
        normal_vector.scale(-1);

    normal_vector.unit();
}

RealVector SurfaceElement::calculate_normal()
{
    RealVector normal;

    switch (this->type())
    {
        case EDGE2:
            Get1DElementNormal(*this, normal);
            break;
        case TRI3:
            Get2DElementNormal(*this, normal);
            break;
        case QUAD4:
            Get2DElementNormal(*this, normal);
            break;
        default:
            break;
    }

    return normal;
}