#ifndef TENSOR_H__
#define TENSOR_H__

#include "numeric_vector.h"

template <typename T>
class Tensor {

    public:
        Tensor();
        T&               operator()(int i, int j);
        NumericVector<T> mult(NumericVector<T>& v);
        T contract(Tensor<T> &v);
        ~Tensor(){};
    private:
        T data[3][3];
};

template <typename T>
Tensor<T>::Tensor() {
    data[0][0] = data[0][1] = data[0][2] = 0;
    data[1][0] = data[1][1] = data[1][2] = 0;
    data[2][0] = data[2][1] = data[2][2] = 0;
}

template <typename T>
inline T& Tensor<T>::operator()(int i, int j) {
    return data[i][j];
}

template <typename T>
ostream& operator<<(ostream& os, const Tensor<T>& v) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            os << v.data[i][j] << " ";
    }
    return os;
}

template <typename T>
NumericVector<T> Tensor<T>::mult(NumericVector<T>& v)
{
    NumericVector<T> tmp;
    tmp(0) = data[0][0]*v(0) + data[0][1]*v(1)  + data[0][2]*v(2);
    tmp(1) = data[1][0]*v(0) + data[1][1]*v(1)  + data[1][2]*v(2);
    tmp(2) = data[2][0]*v(0) + data[2][1]*v(1)  + data[2][2]*v(2);
    return tmp;
}


template <typename T>
T Tensor<T>::contract(Tensor<T> &v)
{
    T tmp = 0;
     for (int i = 0; i < 3; i++) 
        for (int j = 0; j < 3; j++)
            tmp += data[i][j]*v.data[i][j];
    return tmp;
}

typedef Tensor<double>    RealTensor;

#endif /* TENSOR_H__ */
