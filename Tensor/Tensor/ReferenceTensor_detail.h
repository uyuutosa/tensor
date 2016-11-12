#pragma once
#include "ReferenceTensor.h"

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
		T _iniVal, 
		std::map<std::string, int> _shape,
		std::map<std::string, int> _ud
	):
    _Tensor<T>(_shape, _ud){
    iniVal = _iniVal;
    genReferenceTensor(iniVal); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::vector<T*> &_v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _idx, _shape, _ud)
{
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::vector<T*> &_v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _shape, _ud)
{
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_shape, _ud)
{
}


template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator *(T val){

    std::vector<T*>& retV = getV();
    std::vector<T**>& refV = getRefTensor().getV();

	for (int i = 0; i < size(); i++)
		retV[i] = new T(**getRefTensor().ref(genIndices(i)) * (val));
   		//retV[i] = (*(refV[i])) * val;

	return *new ConcreteTensor<T>(*new std::vector<T*>(retV) , shape, ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator *(_Tensor<T>& obj){

	auto refT = std::shared_ptr<_Tensor<T> >(new ReferenceTensor<T>(iniVal, broadcast(obj).shape));
	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>&  retV = refT->getV();

	for (int i = 0; i < noTrimed.size(); i++) 
		refT->ref(noTrimed.genIndices(i)) = new T(**refT->getRefTensor().ref(noTrimed.genIndices(i)) * (*obj.ref(noTrimed.genIndices(i))));

	return *new ConcreteTensor<T>(*new std::vector<T*>(retV) , refT->shape, refT->ud);
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
void ReferenceTensor<T>::genReferenceTensor(T iniVal){

    refTensor = std::shared_ptr<_Tensor<T*> >(new _Tensor<T*>(shape));

    std::vector<T**>& ref_v = refTensor->getV();

    *ref_v[0] = new T(iniVal);

    for(int i=0; i<size()-1; i++)
        ref_v[i+1] = &(v)[i];
}


template<typename T>
bool ReferenceTensor<T>::isRefTensor() {
	return refTensor;
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::clone(){
	return *new auto(*this);
}

template<typename T>
_Tensor<T>&	ReferenceTensor<T>::gen(
                    std::vector<T*> &_v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){
	return *new ReferenceTensor<T>(_v, _idx, _shape, _ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::gen(
                std::vector<T*> &_v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){
	return *new ReferenceTensor<T>(_v, _shape, _ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::gen(
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud){
	return *new ReferenceTensor<T>(_shape, _ud);
}
