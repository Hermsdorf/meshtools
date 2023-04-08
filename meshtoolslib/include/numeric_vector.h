#ifndef NUMERIC_VECTOR_H
#define NUMERIC_VECTOR_H

#include <iostream>
using namespace std;

template <typename T>
class NumericVector {

    public:
        NumericVector();
        T&               operator()(int i);
        T                operator*(NumericVector<T> v);
        T                operator*=(NumericVector<T> v);
        NumericVector<T> operator+(NumericVector<T> v);
        NumericVector<T> operator+=(NumericVector<T> v);
        NumericVector<T> operator-(NumericVector<T> v);
        NumericVector<T> operator-=(NumericVector<T> v);
        NumericVector<T> cross_product(NumericVector<T> v);
        T                norm();
        void             unit();
        //friend ostream& operator<<(ostream& os, const NumericVector<T>& v);
        ~NumericVector();
    private:
        T data[3];
};

template <typename T>
NumericVector<T>::NumericVector() {
    data[0] = data[1] = data[2] = 0;
}

template <typename T>
inline T& NumericVector<T>::operator()(int i) {
    return data[i];
}

template <typename T>
ostream& operator<<(ostream& os, const NumericVector<T>& v) {
    for (int i = 0; i < 3; i++) {
        os << v.data[i] << " ";
    }
    return os;
}


template <typename T>
NumericVector<T>::~NumericVector() {

}


template <typename T>
T NumericVector<T>::operator*(NumericVector<T> v) {
    T result = 0;
    for (int i = 0; i < 3; i++) {
        result += data[i] * v.data[i];
    }
    return result;
}

template <typename T>
T NumericVector<T>::operator*=(NumericVector<T> v) {
    T result = 0;
    for (int i = 0; i < 3; i++) {
        result += data[i] * v.data[i];
    }
    return result;
}

template <typename T>
NumericVector<T> NumericVector<T>::operator+(NumericVector<T> v) 
{
    NumericVector<T> result;
    for (int i = 0; i < 3; i++) {
        result.data[i] += data[i] + v.data[i];
    }
    return result;
}

template <typename T>
NumericVector<T> NumericVector<T>::operator+=(NumericVector<T> v)
{
    NumericVector<T> result;
    for (int i = 0; i < 3; i++) {
        result.data[i] += data[i] + v.data[i];
    }
    return result;
}

template <typename T>
NumericVector<T> NumericVector<T>::operator-(NumericVector<T> v) 
{
    NumericVector<T> result;
    for (int i = 0; i < 3; i++) {
        result.data[i] += data[i] - v.data[i];
    }
    return result;
}

template <typename T>
NumericVector<T> NumericVector<T>::operator-=(NumericVector<T> v) 
{
    NumericVector<T> result;
    for (int i = 0; i < 3; i++) {
        result.data[i] += data[i] - v.data[i];
    }
    return result;
}

template <typename T>
T NumericVector<T>::norm()
{
    T result;
    for (int i = 0; i < 3; i++) {
        result += data[i]*data[i];
    }
    return sqrt(result);
}

template <typename T>
void NumericVector<T>::unit()
{
    T result = norm();
    for (int i = 0; i < 3; i++) {
      this->data[i] /= result;
    }
}

template <typename T>
NumericVector<T> NumericVector<T>::cross_product(NumericVector<T> v)
{
    NumericVector<T> result;
    result.data[0] = data[1]*v.data[2] - data[2]*v.data[1];
    result.data[1] = data[2]*v.data[0] - data[0]*v.data[2];
    result.data[2] = data[0]*v.data[1] - data[1]*v.data[0];
    return result;
}



typedef NumericVector<double>    RealVector;
typedef NumericVector<double>    Gradient;
typedef NumericVector<double>    Point;

#endif /* NUMERIC_VECTOR_H */
