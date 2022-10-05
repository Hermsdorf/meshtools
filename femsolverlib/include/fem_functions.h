#ifndef FEM_FUNCTIONS_H
#define FEM_FUNCTIONS_H

#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h" 



void FEMGetQGauss(MeshElementType elem_type, std::vector<RealVector> &points, std::vector<double> &qw);


void FEMComputeFunctions(MeshElementType ele_type, RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);

void FEMStab(MeshElementType elem_type, RealVector qp, std::vector<Point> &coords, RealVector &g,  RealTensor &G);

/*
class QGauss
{
    public:
        QGauss(int order = 1);
        void SetPoints(int element_type);
        double wheigh(int i);
        Point  point(int i);

    private:
        std::vector<Point>  _gauss_p;
        std::vector<double> _gauss_w;
        unsigned int order;
        unsigned int npoints;

}

class FEFunctions
{
    public:
        FEFunctions();
        ComputeFunctions(Element& elem);

        std::vector<double>  &  get_phi();
        std::vector<Gradient>&  get_dphi();
        std::vector<Point>   &  get_xyz();

    private:
        std::vector<double> >  _phi ;
        std::vector<Gradient>  _dphi;
        std::vector<Point>     _xyz ;

        // cache values 

        

}
*/

#endif /* FEM_FUNCTIONS_H */


