#ifndef QGAUSS_H__
#define QGAUSS_H__
#include <map>
#include "mesh.h"
#include "element.h"

// Respectively: gauss point, gauss weight
typedef std::pair<Point,double> QGaussData;

class QGauss
{
    public:
        static std::unique_ptr<QGauss> New()
        {
            return std::make_unique<QGauss>();
        }
        QGauss();
        void   reset(Element& elem);
        double weight(int qp);
        Point  point(int qp);
        int    n_points(); 
        QGaussData get(int qp);
        QGaussData operator[](int qp);

    private:
        std::vector<Point>  _gauss_p;
        std::vector<double> _gauss_w;
        unsigned int order;
        unsigned int npoints;

};


#endif /* QGAUSS_H__ */
