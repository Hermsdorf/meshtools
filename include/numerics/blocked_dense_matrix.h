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
        void print();
        ~BlockDenseMatrix();
        const T* data();
        //friend ostream& operator<<(ostream& os, const BlockDenseMatrix<T>& m);
    private:
        int idx(int i_row, int j_col, int i_block, int j_block);
        int nrows;
        int ncols;
        int block_size;
        T* _data;
};



template <typename T>
BlockDenseMatrix<T>::BlockDenseMatrix(int rows, int cols, int b_size) {
    nrows = rows;
    ncols = cols;
    block_size = b_size;
    unsigned int size = rows*b_size*cols*b_size;
    _data = new T[size];
    //std::fill(data, data + size, 0);
    for(int i = 0; i < size; i++)
        _data[i] = i;
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
    return ;
}

template <typename T>
T& BlockDenseMatrix<T>::operator()(int row, int col, int i_block, int j_block) {
    assert(row >= 0 && row < nrows);
    assert(col >= 0 && col < ncols);
    assert(i_block >= 0 && i_block < block_size);
    assert(j_block >= 0 && j_block < block_size);
    return _data[idx(row, col,i_block,j_block)];
}

template <typename T>
ostream& operator<<(ostream& os, const BlockDenseMatrix<T>& m) {
    for (int i = 0; i < m.nrows; i++) {
        for (int j = 0; j < m.ncols; j++) {
            for(int bi = 0; bi < m.block_size; bi++)
               for(int bj = 0; bj < m.block_size; bj++)
                    os << m._data[m.idx(i, j, bi, bj)] << " ";
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
    delete[] _data;
}

template <typename T>
const T* BlockDenseMatrix<T>::data(){
    return _data;
}


typedef BlockDenseMatrix<double>  BlockDenseMatrixFloat64;
typedef BlockDenseMatrix<float>   BlockDenseMatrixFloat32;
typedef BlockDenseMatrix<int>     BlockDenseMatrixInt32;


#endif /* BLOCKED_DENSE_MATRIX_H */
