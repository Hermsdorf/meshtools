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

double Element::calculate_h(double JxW)
{
    double h = 0.0;
    switch (this->type())
    {
        case TRI3:
            h = std::sqrt(2.0*JxW);
            break;
        case QUAD4:
            std::cout << "[element.cpp] QUAD4 h characteristic not implemented yet" << std::endl;
            break;
        case TET4:
            std:cout << "[element.cpp]  TET4 h characteristic not implemented yet" << std::endl;
            break;
        default:
            break;
    }

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