#include <iostream>
#include "alglin.h"

Matrix::Matrix(int n, int m, int z)
{
    nx = n;
    ny = m;
    nz = z;
    _data = new double [n*m*z];
}


Matrix::Matrix(int n, int m)
{
    nx = n;
    ny = m;
    nz = m;
    _data = new double [n*m*m];
}

double& Matrix::operator()(int  i, int j, int k)
{
    return _data[i*(nz*nx) + j*nz + k];
}
