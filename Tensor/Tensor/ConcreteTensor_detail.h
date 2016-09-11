#pragma once 
#include "ConcreteTensor.h"

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::broadcast(std::shared_ptr<tensor<T> > obj) {
	int maxDim(idx.size());
	auto newIdx   = idx;
	auto newShape = shape;
	auto newUd    = ud;
	int newSize(1);

	// making corresnpondance between index names and Dimansion names.
	std::string dupUd("");
	for (auto a : obj->ud)
		if (shape.count(a.first))
			dupUd.push_back(*a.first.c_str());

	for (auto a : dupUd) {
		std::string str(1, a);
		newShape.erase(str);
	}

	for (int i=0; i<obj->idx.size(); i++)
		if(!shape.count(obj->idx[i]) && 
			dupUd.find(obj->idx[i]) == std::string::npos)
			newIdx.push_back(obj->idx[i]); // imcrement one more than maximum Dimumtsion.

	for (auto a : obj->shape)
		newShape[a.first] = a.second; // imcrement one more than maximum Dimumtsion.

	for (int i = 0; i < newIdx.size(); i++)
		newSize *= newShape[newIdx[i]];

	return std::shared_ptr<tensor<T> >(
        new ConcreteTensor<T>(
		std::vector<T>(newSize, 0),
		newShape));
}

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator+(std::shared_ptr<tensor<T> > obj)
{
	std::shared_ptr<tensor<T> > ret = broadcast(obj);

    std::vector<T>& retV = ret->getV();
	
	for (int i = 0; i < ret->size(); i++)
		retV[i] = ref(ret->genIndices(i)) + obj->ref(ret->genIndices(i));

	return ret;
}


template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator-(std::shared_ptr<tensor<T> > obj)
{
	std::shared_ptr<tensor<T> > ret = broadcast(obj);

    std::vector<T>& retV = ret->getV();
	
	for (int i = 0; i < ret->size(); i++)
		retV[i] = ref(ret->genIndices(i)) - obj->ref(ret->genIndices(i));

	return ret;
}

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator*(std::shared_ptr<tensor<T> > obj)
{
	std::shared_ptr<tensor<T> > ret = broadcast(obj);

    std::vector<T>& retV = ret->getV();
	
	for (int i = 0; i < ret->size(); i++)
		retV[i] = ref(ret->genIndices(i)) * obj->ref(ret->genIndices(i));

	return ret;
}

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator/(std::shared_ptr<tensor<T> > obj)
{
	std::shared_ptr<tensor<T> > ret = broadcast(obj);

    std::vector<T>& retV = ret->getV();
	
	for (int i = 0; i < ret->size(); i++)
		retV[i] = ref(ret->genIndices(i)) / obj->ref(ret->genIndices(i));

	return ret;
}


template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator+(T val){
	
	std::shared_ptr<tensor<T> > ret = std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(*this));

    std::vector<T>& retV = ret->getV();

	for (int i = 0; i < ret->size(); i++)
		retV[i] += val;

	return ret;
}


template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator-(T val){

	std::shared_ptr<tensor<T> > ret = std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(*this));
	
    std::vector<T>& retV = ret->getV();
	for (int i = 0; i < ret->size(); i++)
		retV[i] -= val;

	return ret;
}

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator*(T val){

	std::shared_ptr<tensor<T> > ret = std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(*this));

    std::vector<T>& retV = ret->getV();
	for (int i = 0; i < ret->size(); i++)
		retV[i] *= val;
	return ret;
}

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator/(T val){

	std::shared_ptr<tensor<T> > ret = std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(*this));

    std::vector<T>& retV = ret->getV();
	for (int i = 0; i < ret->size(); i++)
		retV[i] /= val;
	return ret;
}

template<typename T>
std::shared_ptr<tensor<T> > ConcreteTensor<T>::operator [](std::shared_ptr<tensor<T> > obj){
    std::vector<T>& idxV = getV();
    std::vector<T>  retV(obj->size());
    std::vector<T>& objV = obj->getV();

	for (int i = 0; i < obj->size(); i++)
        retV[i] = idxV[objV[i]];  

    return std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(retV, obj->shape));
}
