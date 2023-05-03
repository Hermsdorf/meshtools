#ifndef FACE_FEM_FUNCTIONS_H__
#define FACE_FEM_FUNCTIONS_H__

#include <iostream>
#include <vector>

#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h" 
#include "qgauss.h"
#include "element.h"


class BoundaryFEMFunction
{
    public:
        BoundaryFEMFunction();
        void ComputeFunction(SurfaceElement& elem, QGaussData qp);
        
        std::vector<double>  &  get_phi() { return _phi; } ;
        std::vector<Gradient>&  get_dphi(){ return _dphi; } ;
        Point                &  get_xyz() { return _xyz; } ;
        double               &  get_JxW() { return _JxW; } ;
            

    private:
        std::vector<double>    _phi ;
        std::vector<Gradient>  _dphi;
        Point                  _xyz ;
        double                 _JxW ;
        RealVector             _normal;
        std::vector<RealVector>  _tangents;
    
        void EDGEFaceFunction(SurfaceElement& elem, QGaussData qp);
        void TRI3FaceFunction(SurfaceElement& elem, QGaussData qp);
        void QUAD4FaceFunction(SurfaceElement& elem, QGaussData qp);
        
};


#endif /* FACE_FEM_FUNCTIONS_H__ */
