#include <iostream>

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