#pragma once
#include "FunctionTensor.h"
//#pragma optimize("", off)

template<typename T>
FunctionTensor<T>::FunctionTensor(
	void* _f,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _map):
	tensor<T>(_shape, _map){
    genFunctionTensor(shape, _f);
}

template<typename T>
std::shared_ptr<FunctionElement<T>> FunctionTensor<T>::genFunctionElementOnAnyRank(
    std::map<std::string, int> indices, 
    void* _f){
    
    auto ret = std::shared_ptr<FunctionElement<T>>();
    switch (indices.size())
    {
    case 1:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank1<T>(indices, (T (*)(int, T))_f));
        break;
    case 2:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank2<T>(indices, (T (*)(int, int, T))_f));
        break;
    case 3:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank3<T>(indices, (T (*)(int, int, int, T))_f));
        break;
    case 4:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank4<T>(indices, (T (*)(int, int, int, int, T))_f));
        break;
    case 5:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank5<T>(indices, (T (*)(int, int, int, int, int, T))_f));
        break;
    case 6:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank6<T>(indices, (T (*)(int, int, int, int, int, int, T))_f));
        break;
    case 7:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank7<T>(indices, (T (*)(int, int, int, int, int, int, int, T))_f));
        break;
    case 8:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank8<T>(indices, (T (*)(int, int, int, int, int, int, int, int, T))_f));
        break;
    case 9:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank9<T>(indices, (T (*)(int, int, int, int, int, int, int, int, int, T))_f));
        break;
    case 10:
        ret = std::shared_ptr<FunctionElement<T>>(new FunctionElementRank10<T>(indices, (T (*)(int, int, int, int, int, int, int, int, int, int, T))_f));
        break;
    }
    return ret;
}

template<typename T>
void FunctionTensor<T>::genFunctionTensor(
    std::map<std::string, int> _shape,
    void* _f){

    int total(1);
    for(auto pair: _shape)
        total *= pair.second;

    std::vector<std::shared_ptr<FunctionElement<T> > > flst(total);
    funcTensor = tensor<std::shared_ptr<FunctionElement<T> > >(flst, _shape);

    for(int i=0; i<flst.size(); i++)        
        flst[i] = genFunctionElementOnAnyRank(funcTensor.genIndices(i), _f);

    funcTensor.setV(flst);
}

template<typename T>
template<typename U>
std::shared_ptr<tensor<U> > FunctionTensor<T>::operator *(U val){
    std::vector<U> retV(size());
    auto funcV = funcTensor.getV();


	for (int i = 0; i < size(); i++)
		retV[i] = *funcV[i] * val;

	return std::shared_ptr<tensor<U> >(new ConcreteTensor<U>(retV, shape));
}

template<typename T>
std::shared_ptr<tensor<T> > FunctionTensor<T>::operator*(T val)
{
    std::vector<T> retV(size());
    auto funcV = funcTensor.getV();


	for (int i = 0; i < size(); i++)
		retV[i] = *funcV[i] * val;

	return std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(retV, shape));
}

template<typename T>
std::shared_ptr<tensor<T> > FunctionTensor<T>::operator *(std::shared_ptr<tensor<T> > obj) {
	std::shared_ptr<tensor<T> > ret;

	if (obj->getRefTensor()) {
		ret = std::shared_ptr<tensor<T> >(new ReferenceTensor<T>(obj->iniVal, broadcast(obj)->shape));
		std::vector<T>& retV = ret->getV();
		for (int i = 0; i < ret->size(); i++)
			retV[i] = *funcTensor.ref(ret->genIndices(i)) * (*ret->getRefTensor()->ref(ret->genIndices(i)));
	} else {
		ret = broadcast(obj);
		std::vector<T>& retV = ret->getV();
		for (int i = 0; i < ret->size(); i++)
			retV[i] = *funcTensor.ref(ret->genIndices(i)) * obj->ref(ret->genIndices(i));
	}
	return ret;
}

//template<typename T>
//std::shared_ptr<tensor<T> > FunctionTensor<T>::operator+(std::shared_ptr<tensor<T> > obj)
//{
//	return std::shared_ptr<tensor<T> >();
//}

