#pragma once
#include "Tensor.h"

template<typename T, typename U>
Tensor<bool> operator <(
    U val,
    Tensor<T>& obj){
	return obj > val;
}

template<typename T, typename U>
Tensor<bool> operator >(
    U val,
    Tensor<T>& obj){
	return obj < val;
}

template<typename T>
Tensor<bool> operator <(
    T val,
    Tensor<T>& obj){
	return obj > val;
}

template<typename T>
Tensor<bool> operator >(
    T val,
    Tensor<T>& obj){
	return obj < val;
}

template<typename T>
Tensor<T> operator +(
    T val,
    Tensor<T>& obj){
	return obj + val;
}

template<typename T>
Tensor<T> operator -(
    T val,
    Tensor<T>& obj){
	return obj - val;
}

template<typename T>
Tensor<T> operator *(
    T val,
    Tensor<T>& obj){
	return obj * val;
}

template<typename T>
Tensor<T> operator /(
    T val,
    Tensor<T>& obj){
	return obj / val;
}

template<typename T>
Tensor<T> operator ^(
    T val,
    Tensor<T>& obj){
	return obj ^ val;
}

template<typename T>
Tensor<T> operator -(
	Tensor<T> obj1,
	Tensor<T> obj2) {

	return obj1 - obj2;
}

template<typename T>
Tensor<T> operator *(
	Tensor<T> obj1,
	Tensor<T> obj2) {

	return obj1 * obj2;
}

template<typename T>
Tensor<T> operator /(
	Tensor<T> obj1,
	Tensor<T> obj2) {

	return obj1 / obj2;
}

template<typename T>
Tensor<T> operator +(
	Tensor<T> obj1,
	T obj2) {

	return obj1 + obj2;
}

template<typename T>
Tensor<T> operator -(
	Tensor<T> obj1,
	T obj2) {

	return obj1 - obj2;
}

template<typename T>
Tensor<T> operator *(
	Tensor<T> obj1,
	T obj2) {

	return obj1 * obj2;
}

template<typename T>
Tensor<T> operator /(
	Tensor<T> obj1,
	T obj2) {

	return obj1 / obj2;
}
