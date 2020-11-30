#include <iostream>

class Matrix
{
    public:
        Matrix(int n, int m);
        Matrix(int n, int m, int z);
        double& operator()(int i, int j, int z);

    private:
        double *_data;
        unsigned int nx, ny, nz;
};