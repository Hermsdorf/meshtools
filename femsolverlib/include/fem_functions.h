#ifndef FEM_FUNCTIONS_H
#define FEM_FUNCTIONS_H

#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h" 
#include "qgauss.h"


void FEMGetQGauss(MeshElementType elem_type, std::vector<RealVector> &points, std::vector<double> &qw);


void FEMComputeFunctions(MeshElementType ele_type, RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);

void FEMStab(MeshElementType elem_type, RealVector qp, std::vector<Point> &coords, RealVector &g,  RealTensor &G);



class FEMFunction
{
    public:
        FEMFunction();
        void ComputeFunction(Element& elem, QGaussData qp);

        std::vector<double>  &  get_phi() { return _phi; } ;
        std::vector<Gradient>&  get_dphi(){ return _dphi; } ;
        Point                &  get_xyz() { return _xyz; } ;
        double               &  get_JxW() { return _JxW; } ;
        RealVector           &  get_g() { return _g; } ;
        RealTensor           &  get_G() { return _G; } ;
            

    private:
        std::vector<double>    _phi ;
        std::vector<Gradient>  _dphi;
        Point                  _xyz ;
        double                 _JxW ;

        
        // cache values
        RealTensor             _G;
        RealVector             _g;

        //
        void TRI3Function(Element& elem, QGaussData qp);
        void QUAD4Function(Element& elem, QGaussData qp);
        void EDGEFaceFunction(Element& elem, QGaussData qp);
        void TET4Function(Element& elem, QGaussData qp);
        
};


#endif /* FEM_FUNCTIONS_H */


