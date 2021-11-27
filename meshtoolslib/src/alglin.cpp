#include <iostream>
#include "alglin.h"

Matrix::Matrix(unsigned int n, unsigned int m, unsigned int z)
{
    nx = n;
    ny = m;
    nz = z;
    _data = new double[n*m*z];
}


Matrix::Matrix(unsigned int n, unsigned int m)
{
    nx = n;
    ny = m;
    nz = m;
    _data = new double[n*m*m];
}

Matrix::~Matrix()
{
    delete [] _data;
}

double& Matrix::operator()(unsigned int i, unsigned int j, unsigned int k)
{
    return _data[i*(ny*nz) + j*nz + k];
}

