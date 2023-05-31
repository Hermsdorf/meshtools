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
        NumericVector<T> operator+(NumericVector<T> v);
        void             operator+=(NumericVector<T> v);
        NumericVector<T> operator-(NumericVector<T> v);
        void             operator-=(NumericVector<T> v);
        NumericVector<T> operator*(T v);
        void             operator*=(T v);
        NumericVector<T> operator/(T v);
        void             operator/=(T v);
        NumericVector<T> cross_product(NumericVector<T> v);
        T                dot_product(NumericVector<T> v);
        T                norm();
        void             unit();
        void             scale(T s);
        void             zero();
        void             add_scaled(const T s, const NumericVector<T> &v);
        void             add_scaled_vector(const NumericVector<T> &s, const NumericVector<T> &v);
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
NumericVector<T>::~NumericVector() {

}

template <typename T>
inline T& NumericVector<T>::operator()(int i) {
    return data[i];
}

template <typename T>
ostream& operator<<(ostream& os, NumericVector<T>& v) {
    for (int i = 0; i < 3; i++) {
        os << v(i) << " ";
    }
    return os;
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
NumericVector<T> NumericVector<T>::operator*(T v) {
    NumericVector result;
    for (int i = 0; i < 3; i++) {
        result(i) = v*data[i];
    }
    return result;
}

template <typename T>
void NumericVector<T>::operator*=(T v) {
    for (int i = 0; i < 3; i++) {
        data[i] = v*data[i];
    }
}

template <typename T>
NumericVector<T> NumericVector<T>::operator+(NumericVector<T> v) 
{
    NumericVector<T> result;
    for (int i = 0; i < 3; i++) {
        result.data[i] = data[i] + v.data[i];
    }
    return result;
}

template <typename T>
void NumericVector<T>::operator+=(NumericVector<T> v)
{
    for (int i = 0; i < 3; i++) {
        data[i] += v.data[i];
    }
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
void NumericVector<T>::operator-=(NumericVector<T> v) 
{
    for (int i = 0; i < 3; i++) {
        data[i] -= v.data[i];
    }
}

template <typename T>
NumericVector<T> NumericVector<T>::operator/(T v)
{
    NumericVector<T> result;
    for (int i = 0; i < 3; i++) {
        result.data[i] = data[i]/v;
    }
    return result;
}

template <typename T>
void NumericVector<T>::operator/=(T v)
{
    for (int i = 0; i < 3; i++) {
        data[i] /= v;
    }
}

template <typename T>
T NumericVector<T>::norm()
{
    T result = 0;
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

template <typename T>
T NumericVector<T>::dot_product(NumericVector<T> v)
{
    T result = 0;
    for (int i = 0; i < 3; i++) {
        result += data[i]*v.data[i];
    }
    return result;
}

template<typename T>
void NumericVector<T>::scale(T s)
{
    for (int i = 0; i < 3; i++) {
        data[i] *= s;
    }
}

template<typename T>
void NumericVector<T>::zero()
{
    for (int i = 0; i < 3; i++) {
        data[i] = 0;
    }
}

template<typename T>
void NumericVector<T>::add_scaled(const T s, const NumericVector<T> &v)
{
    for (int i = 0; i < 3; i++) {
        data[i] += s*v.data[i];
    }
}

template<typename T>
void NumericVector<T>::add_scaled_vector(const NumericVector<T> &s, const NumericVector<T> &v)
{
    for (int i = 0; i < 3; i++) {
        data[i] += s.data[i]*v.data[i];
    }
}


typedef NumericVector<double>    RealVector;
typedef NumericVector<double>    Gradient;
typedef NumericVector<double>    Point;

#endif /* NUMERIC_VECTOR_H */
