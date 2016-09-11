#pragma once
#include "tensor.h"

template<typename T>
std::shared_ptr<tensor<T> > operator +(
	std::shared_ptr<tensor<T> > obj1,
	std::shared_ptr<tensor<T> > obj2) {

	return *obj1 + obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator -(
	std::shared_ptr<tensor<T> > obj1,
	std::shared_ptr<tensor<T> > obj2) {

	return *obj1 - obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator *(
	std::shared_ptr<tensor<T> > obj1,
	std::shared_ptr<tensor<T> > obj2) {

	return *obj1 * obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator /(
	std::shared_ptr<tensor<T> > obj1,
	std::shared_ptr<tensor<T> > obj2) {

	return *obj1 / obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator +(
	std::shared_ptr<tensor<T> > obj1,
	T obj2) {

	return *obj1 + obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator -(
	std::shared_ptr<tensor<T> > obj1,
	T obj2) {

	return *obj1 - obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator *(
	std::shared_ptr<tensor<T> > obj1,
	T obj2) {

	return *obj1 * obj2;
}

template<typename T>
std::shared_ptr<tensor<T> > operator /(
	std::shared_ptr<tensor<T> > obj1,
	T obj2) {

	return *obj1 / obj2;
}
