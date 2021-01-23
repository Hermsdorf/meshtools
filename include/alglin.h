#include <iostream>

#ifndef ALGLIN_H__
#define ALGLIN_H__
class Matrix
{
    public:
        Matrix(unsigned int n, unsigned int m);
        Matrix(unsigned int n, unsigned int m, unsigned int z);
        ~Matrix();
        double& operator()(unsigned int i, unsigned int j, unsigned int z);

    private:
        double *_data;
        unsigned int nx, ny, nz;
};
#endif //ALGLIN_H__

