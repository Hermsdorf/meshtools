#ifndef DENSE_MATRIX_H
#define DENSE_MATRIX_H

#include <iostream>
using namespace std;

template <typename T>
class DenseMatrix {

    public:
        DenseMatrix(int rows, int cols);
        T& operator()(int row, int col);
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


typedef DenseMatrix<double> RealDenseMatrix;
typedef DenseMatrix<int>     IntDenseMatrix;

#endif /* DENSE_MATRIX_H */
