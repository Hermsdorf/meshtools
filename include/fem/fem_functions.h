#ifndef FEM_FUNCTIONS_H__
#define FEM_FUNCTIONS_H__

#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h" 
#include "qgauss.h"


class FEMFunction
{
    public:
        FEMFunction(ParallelMesh& mesh);
        void ComputeFunction(Element& elem, QGaussData qp);

        std::vector<double>  &  get_phi() { return _phi; } ;
        std::vector<Gradient>&  get_dphi(){ return _dphi; } ;
        Point                &  get_xyz() { return _xyz; } ;
        double               &  get_JxW() { return _JxW; } ;
        RealVector           &  get_g() { return _g; } ;
        RealTensor           &  get_G() { return _G; } ;
        RealVector           &  get_dxi() { return _dxi; } ;
        RealVector           &  get_deta() { return _deta; } ;
        RealVector           &  get_dzeta() { return _dzeta; } ;
            

    private:
        std::vector<double>    _phi ; // shape functions in reference coordinate system
        std::vector<Gradient>  _dphi; // gradient of the shape functions in reference coordinate system
        Point                  _xyz ; // coordinates of gaussian point in physical coordinate system
        double                 _JxW ; // Jacobian determinant times weight

        
        // cache values
        RealTensor             _G;
        RealVector             _g;
        RealVector             _dxi;
        RealVector             _deta;
        RealVector             _dzeta;

        //
        void TRI3Function(Element& elem, QGaussData qp);
        void QUAD4Function(Element& elem, QGaussData qp);
        void TET4Function(Element& elem, QGaussData qp);
        
        ParallelMesh& _mesh;
};


#endif /* FEM_FUNCTIONS_H__ */


