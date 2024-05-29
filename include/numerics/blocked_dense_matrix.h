#ifndef BLOCKED_DENSE_MATRIX_H
#define BLOCKED_DENSE_MATRIX_H

#include <iostream>
using namespace std;

#include "numeric_vector.h"

template <typename T>
class BlockDenseMatrix {

    public:
        BlockDenseMatrix(int rows, int cols, int b_size);
        T& operator()(int row, int col, int b_i, int b_j);
        // BlockDenseMatrix<T> operator*(BlockDenseMatrix<T> &B);
        // BlockDenseMatrix<T> operator*(T scalar);
        // BlockDenseMatrix<T> operator*=(T scalar);
        // BlockDenseMatrix<T> transpose();
        void print();
        ~BlockDenseMatrix();
        T* data();
    private:
        int idx(int i_row, int j_col, i_block, int j_block);
        int nrows;
        int ncols;
        int block_size;
        T* data;
};



template <typename T>
BlockDenseMatrix<T>::BlockDenseMatrix(int rows, int cols, int b_size) {
    nrows = rows;
    ncols = cols;
    block_size = b_size;
    unsigned int size = rows*b_size*cols*b_size;
    data = new T[size];
    std::fill(data, data + size, 0);
}

/*
    no1            no2
    a0   a1   a2  a3  a4   a5
no1 a6   a7   a8  a9  a10 a11
    a12  a13  a14 a15 a16 a17
    a18  a19  a20 a21 a22 a23
no2 a24  a25  a26 a27 a28 a29
    a30  a31  a32 a33 a34 a35
*/

template <typename T>
inline int BlockDenseMatrix<T>::idx(int i_row, int j_jow, int i_block, int j_block) {
    return i_row*(ncol*b_size*b_size) + j_row*n_col*b_size + i_block*b_size + j_block;
}

template <typename T>
T& BlockDenseMatrix<T>::operator()(int row, int col, int i_block, int j_block) {
    return data[idx(row, col,i_block,j_block)];
}

template <typename T>
ostream& operator<<(ostream& os, const BlockDenseMatrix<T>& m) {
    for (int i = 0; i < m.nrows; i++) {
        for (int j = 0; j < m.ncols; j++) {
            for(int bi = 0; bi < b_size; bi++)
               for(int bj = 0; bj < b_size; bj++)
                    os << m.data[m.idx(i, j, bi, bj)] << " ";
        }
        os << endl;
    }
    return os;
}

template<typename T>
void BlockDenseMatrix<T>::print() {
    std::cout << *this ;
}

template <typename T>
BlockDenseMatrix<T>::~BlockDenseMatrix() {
    delete[] data;
}

template <typename T>
T* BlockDenseMatrix<T>::data() {
    return data;
}

// template <typename T>
// DenseMatrix<T> DenseMatrix<T>::operator*(DenseMatrix<T> &B)
// {
//     DenseMatrix<T> C(nrows, B.ncols);
//     for (int i = 0; i < nrows; i++) {
//         for (int j = 0; j < B.ncols; j++) {
//             for (int k = 0; k < ncols; k++) {
//                 C(i, j) += data[idx(i, k)] * B(k, j);
//             }
//         }
//     }
//     return C;
// }

// template <typename T>
// DenseMatrix<T> DenseMatrix<T>::operator*(T scalar)
// {
//     DenseMatrix<T> C(nrows, ncols);
//     for (int i = 0; i < nrows; i++) {
//         for (int j = 0; j < ncols; j++) {
//             C(i, j) = data[idx(i, j)] * scalar;
//         }
//     }
//     return C;
// }

// template <typename T>
// DenseMatrix<T> DenseMatrix<T>::operator*=(T scalar)
// {
//     for (int i = 0; i < nrows; i++) {
//         for (int j = 0; j < ncols; j++) {
//             data[idx(i, j)] *= scalar;
//         }
//     }
//     return *this;
// }

// template <typename T>
// DenseMatrix<T> DenseMatrix<T>::transpose()
// {
//     DenseMatrix<T> C(ncols, nrows);
//     for (int i = 0; i < nrows; i++) {
//         for (int j = 0; j < ncols; j++) {
//             C(j, i) = data[idx(i, j)];
//         }
//     }
//     return C;
// }

typedef DenseMatrix<double> RealDenseMatrix;
typedef DenseMatrix<int>     IntDenseMatrix;



#endif /* BLOCKED_DENSE_MATRIX_H */
