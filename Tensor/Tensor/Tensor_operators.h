#pragma once
#include "Tensor.h"

template<typename T, typename U>
bool operator <(
    U val,
    Tensor<T>& obj){
	return obj > val;
}

template<typename T, typename U>
bool operator >(
    U val,
    Tensor<T>& obj){
	return obj < val;
}

template<typename T>
bool operator <(
    T val,
    Tensor<T>& obj){
	return obj > val;
}

template<typename T>
bool operator >(
    T val,
    Tensor<T>& obj){
	return obj < val;
}

template<typename T>
Tensor<T>& operator +(
    T val,
    Tensor<T>& obj){
	return obj + val;
}

template<typename T>
Tensor<T>& operator -(
    T val,
    Tensor<T>& obj){
	return obj - val;
}

template<typename T>
Tensor<T>& operator *(
    T val,
    Tensor<T>& obj){
	return obj * val;
}

template<typename T>
Tensor<T>& operator /(
    T val,
    Tensor<T>& obj){
	return obj / val;
}

template<typename T>
Tensor<T>& operator ^(
    T val,
    Tensor<T>& obj){
	return obj ^ val;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator -(
	std::shared_ptr<Tensor<T> > obj1,
	std::shared_ptr<Tensor<T> > obj2) {

	return *obj1 - obj2;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator *(
	std::shared_ptr<Tensor<T> > obj1,
	std::shared_ptr<Tensor<T> > obj2) {

	return *obj1 * obj2;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator /(
	std::shared_ptr<Tensor<T> > obj1,
	std::shared_ptr<Tensor<T> > obj2) {

	return *obj1 / obj2;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator +(
	std::shared_ptr<Tensor<T> > obj1,
	T obj2) {

	return *obj1 + obj2;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator -(
	std::shared_ptr<Tensor<T> > obj1,
	T obj2) {

	return *obj1 - obj2;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator *(
	std::shared_ptr<Tensor<T> > obj1,
	T obj2) {

	return *obj1 * obj2;
}

template<typename T>
std::shared_ptr<Tensor<T> > operator /(
	std::shared_ptr<Tensor<T> > obj1,
	T obj2) {

	return *obj1 / obj2;
}
