#ifndef DENSE_MATRIX_H
#define DENSE_MATRIX_H

#include <iostream>
using namespace std;

#include "numeric_vector.h"

template <typename T>
class DenseMatrix {

    public:
        DenseMatrix(int rows, int cols);
        T& operator()(int row, int col);
        DenseMatrix<T> operator*(DenseMatrix<T> &B);
        DenseMatrix<T> operator*(T scalar);
        DenseMatrix<T> operator*=(T scalar);
        DenseMatrix<T> transpose();
        void print();
        ~DenseMatrix();
        T* get_data();
    private:
        int idx(int i, int j);
        int nrows;
        int ncols;
        T* data;
};

template <typename T>
DenseMatrix<T>::DenseMatrix(int rows, int cols) {
    nrows = rows;
    ncols = cols;
    data = new T[rows*cols];
    std::fill(data, data + rows*cols, 0);
}

template <typename T>
inline int DenseMatrix<T>::idx(int i, int j) {
    return i*ncols + j;
}

template <typename T>
T& DenseMatrix<T>::operator()(int row, int col) {
    return data[idx(row, col)];
}

template <typename T>
ostream& operator<<(ostream& os, const DenseMatrix<T>& m) {
    for (int i = 0; i < m.nrows; i++) {
        for (int j = 0; j < m.ncols; j++) {
            os << m.data[m.idx(i, j)] << " ";
        }
        os << endl;
    }
    return os;
}

template<typename T>
void DenseMatrix<T>::print() {
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            cout << data[idx(i, j)] << " ";
        }
        cout << endl;
    }
}

template <typename T>
DenseMatrix<T>::~DenseMatrix() {
    delete[] data;
}

template <typename T>
T* DenseMatrix<T>::get_data() {
    return data;
}

template <typename T>
DenseMatrix<T> DenseMatrix<T>::operator*(DenseMatrix<T> &B)
{
    DenseMatrix<T> C(nrows, B.ncols);
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < B.ncols; j++) {
            for (int k = 0; k < ncols; k++) {
                C(i, j) += data[idx(i, k)] * B(k, j);
            }
        }
    }
    return C;
}

template <typename T>
DenseMatrix<T> DenseMatrix<T>::operator*(T scalar)
{
    DenseMatrix<T> C(nrows, ncols);
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            C(i, j) = data[idx(i, j)] * scalar;
        }
    }
    return C;
}

template <typename T>
DenseMatrix<T> DenseMatrix<T>::operator*=(T scalar)
{
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            data[idx(i, j)] *= scalar;
        }
    }
    return *this;
}

template <typename T>
DenseMatrix<T> DenseMatrix<T>::transpose()
{
    DenseMatrix<T> C(ncols, nrows);
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            C(j, i) = data[idx(i, j)];
        }
    }
    return C;
}

typedef DenseMatrix<double> RealDenseMatrix;
typedef DenseMatrix<int>     IntDenseMatrix;

#endif /* DENSE_MATRIX_H */
