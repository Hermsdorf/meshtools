
#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h"


#include "fem_functions.h"


void TRI3DefaultQGauss(std::vector<RealVector> &points, std::vector<double> &qw);

void TRI3ComputeFunctions( RealVector q_point, double qw, std::vector<Point> coords, RealVector &p_gauss, 
                           std::vector<double>   &phi, 
                           std::vector<Gradient> &dphi ,
                           double &JxW);

void TRI3Stab(RealVector q_point, std::vector<Point> coords, RealVector &g,  RealTensor &G);

void QUAD4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw);

void QUAD4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);

void TET4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw);

void TET4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);
 

void FEMGetQGauss(MeshElementType elem_type, std::vector<RealVector> &points, std::vector<double> &qw)
{
    switch (elem_type)
    {
    case TRI3:
        TRI3DefaultQGauss(points,qw);
        break;
    case QUAD4:
        QUAD4DefaultQGauss(points,qw);
        break;
    case TET4:
        TET4DefaultQGauss(points,qw);
        break;
    default:
        throw std::runtime_error("Element type not supported");
        break;
    }
}


void FEMComputeFunctions(MeshElementType elem_type, RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW)
{
    switch (elem_type)
    {
    case TRI3:
        TRI3ComputeFunctions(qp, qw,coords, point, phi,dphi,JxW);
        break;
    case QUAD4:
        QUAD4ComputeFunctions(qp, qw,coords, point, phi,dphi,JxW);
        break;
    case TET4:
        TET4ComputeFunctions(qp, qw,coords, point, phi,dphi,JxW);
        break;
    default:
         std::cout <<" Elemento: " << elem_type << std::endl;
         throw std::runtime_error("Element type not supported");
        break;
    }
}


void FEMStab(MeshElementType elem_type, RealVector qp, std::vector<Point> &coords, RealVector &g,  RealTensor &G)
{
    switch (elem_type)
    {
    case TRI3:
        TRI3Stab(qp, coords,g,G);
        break;
    default:
         std::cout <<" Elemento: " << elem_type << std::endl;
         throw std::runtime_error("Element type not supported");
        break;
    }
}

