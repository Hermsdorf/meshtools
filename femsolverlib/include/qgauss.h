#ifndef QGAUSS_H__
#define QGAUSS_H__
#include <map>
#include "mesh.h"

typedef std::pair<Point,double> QGaussData;

class QGauss
{
    public:
        QGauss();
        void   reset(Element& elem);
        double weight(int i);
        Point  point(int i);
        int    n_points(); 
        QGaussData get(int i);

    private:
        std::vector<Point>  _gauss_p;
        std::vector<double> _gauss_w;
        unsigned int order;
        unsigned int npoints;

};


#endif /* QGAUSS_H__ */
