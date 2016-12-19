#pragma once
#include "ReferenceTensor.h"
////#pragma optimize("", off)

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
		T _iniVal, 
		std::map<std::string, int> _shape,
		std::map<std::string, int> _ud
	):
    _Tensor<T>(_shape, _ud){
    iniVal = std::shared_ptr<T>(new T(_iniVal));
    genReferenceTensor(); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _idx, _shape, _ud)
{
    iniVal = std::shared_ptr<T>(new T);
    genReferenceTensor(); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _shape, _ud)
{
    iniVal = std::shared_ptr<T>(new T);
    genReferenceTensor(); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_shape, _ud)
{
    iniVal = std::shared_ptr<T>(new T);
    genReferenceTensor(); 
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator +(T val){

    auto retV = getV();
    auto refV = getRefTensor()->getV();

	for (int i = 0; i < size(); i++)
		*(*retV)[i] += *getRefTensor()->ref(genIndices(i)) + (val);

	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(retV, shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator -(T val){

    auto retV = getV();
    auto refV = getRefTensor()->getV();

	for (int i = 0; i < size(); i++)
		*(*retV)[i] -= *getRefTensor()->ref(genIndices(i)) - (val);

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(retV)),
    //                   shape, 
    //                   ud));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(retV, shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator *(T val){

    auto retV = getV();
    auto refV = getRefTensor()->getV();

	for (int i = 0; i < size(); i++){
		*(*retV)[i] =  *getRefTensor()->ref(genIndices(i)) * (val);
    }

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(retV)),
    //                   shape, 
    //                   ud));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(retV, shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator /(T val){

    auto retV = getV();
    auto refV = getRefTensor()->getV();

	for (int i = 0; i < size(); i++)
		*(*retV)[i] /= *getRefTensor()->ref(genIndices(i)) / (val);

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(retV)),
    //                   shape, 
    //                   ud));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(retV, shape, ud));
}


template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator +(std::shared_ptr<_Tensor<T>> obj){

//    auto& ret = clone();
	auto ret = broadcast(obj, true);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	auto noTrimed = broadcast(obj);
    auto refV = ret->getRefTensor()->getV();
    int  nSize(noTrimed->size());
	*ret->ref(noTrimed->genIndices(0)) += (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ret->ref(noTrimed->genIndices(i)) += *getRefTensor()->ref(noTrimed->genIndices(i)) + (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(ret->getV(), shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator -(std::shared_ptr<_Tensor<T>> obj){

//    auto& ret = clone();
	auto ret = broadcast(obj, true);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	auto noTrimed = broadcast(obj);
    auto refV = ret->getRefTensor()->getV();
    int  nSize(noTrimed->size());
	*ret->ref(noTrimed->genIndices(0)) -= (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ret->ref(noTrimed->genIndices(i)) -= *getRefTensor()->ref(noTrimed->genIndices(i)) - (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(ret->getV(), shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator *(std::shared_ptr<_Tensor<T>> obj){

//    auto& ret = clone();
	auto ret  = broadcast(obj, true);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	auto noTrimed = broadcast(obj);
    auto refV = ret->getRefTensor()->getV();
    int  nSize(noTrimed->size());
	*(ret->ref(noTrimed->genIndices(0))) = *ref(noTrimed->genIndices(0)) * (*obj->ref(noTrimed->genIndices(0)));
	*noTrimed->ref(noTrimed->genIndices(0)) = *ret->ref(noTrimed->genIndices(0));
	for (int i = 1; i < nSize; i++){
		*(ret->ref(noTrimed->genIndices(i))) = (*ref(noTrimed->genIndices(i)) *  *(noTrimed->getRefTensor()->ref(noTrimed->genIndices(i)))) * *(obj->ref(noTrimed->genIndices(i)));
	    *noTrimed->ref(noTrimed->genIndices(i)) = *ret->ref(noTrimed->genIndices(i));
    }

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(ret->getV(), shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::operator /(std::shared_ptr<_Tensor<T>> obj){

//    auto& ret = clone();
	auto ret = broadcast(obj, true);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	auto noTrimed = broadcast(obj);
    auto  refV = ret->getRefTensor()->getV();
    int nSize(noTrimed->size());
	*ret->ref(noTrimed->genIndices(0)) /= (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ret->ref(noTrimed->genIndices(i)) /= *getRefTensor()->ref(noTrimed->genIndices(i)) / (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std:shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(ret->getV(), shape, ud));
}


template<typename T>
void ReferenceTensor<T>::operator+=(std::shared_ptr<_Tensor<T>> obj)
{
	auto noTrimed = broadcast(obj);
    auto  refV = getRefTensor()->getV();
    int nSize(noTrimed->size());
	*ref(noTrimed->genIndices(0)) += (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed->genIndices(i)) += *getRefTensor()->ref(noTrimed->genIndices(i)) + (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
}

template<typename T>
void ReferenceTensor<T>::operator-=(std::shared_ptr<_Tensor<T>> obj)
{
	auto noTrimed = broadcast(obj);
    auto refV = getRefTensor()->getV();
    int  nSize(noTrimed->size());
	*ref(noTrimed->genIndices(0)) -= (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed->genIndices(i)) -= *getRefTensor()->ref(noTrimed->genIndices(i)) - (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std::shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
}

template<typename T>
void ReferenceTensor<T>::operator*=(std::shared_ptr<_Tensor<T>> obj)
{
	auto noTrimed = broadcast(obj);
    auto refV = getRefTensor()->getV();
    int nSize(noTrimed->size());
	*ref(noTrimed->genIndices(0)) *= (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed->genIndices(i)) *= *getRefTensor()->ref(noTrimed->genIndices(i)) * (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std:shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
}


template<typename T>
void ReferenceTensor<T>::operator/=(std::shared_ptr<_Tensor<T>> obj)
{
	auto noTrimed = broadcast(obj);
    auto refV     = getRefTensor()->getV();
    int  nSize(noTrimed->size());
	*ref(noTrimed->genIndices(0)) /= (*obj->ref(noTrimed->genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed->genIndices(i)) /= *getRefTensor()->ref(noTrimed->genIndices(i)) / (*obj->ref(noTrimed->genIndices(i)));

	//return std::shared_ptr<_Tensor<T>>(
    //           new ConcreteTensor<T>(
    //               std::shared_ptr<std::vector<std:shared_ptr<T>>>(
    //                   new std::vector<std::shared_ptr<T>>(
    //                       ret->getV()), 
    //                   ret->getRefTensor()->shape, 
    //                   ret->getRefTensor()->ud)));
}

//template<typename T>
//std::shared_ptr<_Tensor<T> > ReferenceTensor<T>::operator *(std::shared_ptr<_Tensor<T> > obj){

//    std::vector<T>& retV = getV();
//    std::vector<T*>& refV = getRef_Tensor()->getV();

//	for (int i = 0; i < size(); i++)
//		retV[i] = *getRef_Tensor()->ref(genIndices(i)) * val;
//   		//retV[i] = (*(refV[i])) * val;

//	return std::shared_ptr<_Tensor<T> >(new ConcreteTensor<T>(std::vector<T>(retV) , shape));
//}

template<typename T>
void ReferenceTensor<T>::genReferenceTensor(){

    refTensor = std::shared_ptr<_Tensor<T> >(new _Tensor<T>(shape));

    auto ref_v = refTensor->getV();

    (*ref_v)[0] = iniVal;

    for(int i=0; i<size()-1; i++)
        (*ref_v)[i+1] = (*v)[i];
}


template<typename T>
bool ReferenceTensor<T>::isRefTensor() {
	return refTensor;
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::clone(){
	auto ret    = std::shared_ptr<_Tensor<T>>(new auto(*this));
    ret->iniVal = std::shared_ptr<T>(new T(*iniVal));
    ret->genReferenceTensor();
    return std::shared_ptr<_Tensor<T>>(ret);
}

template<typename T>
std::shared_ptr<_Tensor<T>>	ReferenceTensor<T>::gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new ReferenceTensor<T>(_v, _idx, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::gen(
                std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new ReferenceTensor<T>(_v, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ReferenceTensor<T>::gen(
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new ReferenceTensor<T>(_shape, _ud));
}
