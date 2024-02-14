
#include "qgauss.h"


QGauss::QGauss():order(1),npoints(1)
{
    
}

void QGauss::reset(Element& elem)
{
    switch (elem.type())
    {
    case EDGE2:
        this->npoints = 1;
        _gauss_p.resize(1);
        _gauss_w.resize(1);
        _gauss_p[0](0) = 0.5; // 0
        _gauss_w[0]    = 1.0; // 2?
        break;
    case TRI3:
        this->npoints = 1;
        _gauss_p.resize(1);
        _gauss_w.resize(1);
        _gauss_p[0](0) = 1.0/3.0;
        _gauss_p[0](1) = 1.0/3.0;
        _gauss_p[0](2) = 0.0;
        _gauss_w[0] = 0.5;
        break;
    case QUAD4:
        this->npoints = 4;
        _gauss_p.resize(4);
        _gauss_w.resize(4);
        _gauss_p[0](0) =  -0.57735026919;
        _gauss_p[0](1) =  -0.57735026919;
        _gauss_p[1](0) =   0.57735026919;
        _gauss_p[1](1) =  -0.57735026919;
        _gauss_p[2](0) =   0.57735026919;
        _gauss_p[2](1) =   0.57735026919;
        _gauss_p[3](0) =  -0.57735026919;
        _gauss_p[3](1) =   0.57735026919;
        _gauss_w[0] = _gauss_w[1] = _gauss_w[2] = _gauss_w[3] = 1.0;
        break;
    case TET4:
        this->npoints = 1;
        _gauss_p.resize(1);
        _gauss_w.resize(1);
        _gauss_p[0](0) = 0.25;
        _gauss_p[0](1) = 0.25;
        _gauss_p[0](2) = 0.25;
        _gauss_w[0]   = 1.0/6.0;
    default:
        break;
    }
}

int QGauss::n_points()
{
    return this->npoints;
}

double QGauss::weight(int i)
{
    return _gauss_w[i];
}

Point QGauss::point(int i)
{
    return _gauss_p[i];
}

QGaussData QGauss::get(int i)
{
    QGaussData p;
    p.first  = _gauss_p[i];
    p.second = _gauss_w[i];
    return p;
}


