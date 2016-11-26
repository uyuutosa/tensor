#pragma once
#include "ReferenceTensor.h"
#pragma optimize("", off)

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
		T _iniVal, 
		std::map<std::string, int> _shape,
		std::map<std::string, int> _ud
	):
    _Tensor<T>(_shape, _ud){
    iniVal = new T(_iniVal);
    genReferenceTensor(); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::vector<T*> &_v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _idx, _shape, _ud)
{
    iniVal = new T;
    genReferenceTensor(); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::vector<T*> &_v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _shape, _ud)
{
    iniVal = new T;
    genReferenceTensor(); 
}

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_shape, _ud)
{
    iniVal = new T;
    genReferenceTensor(); 
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator +(T val){

    std::vector<T*>& retV = getV();
    std::vector<T*>& refV = getRefTensor().getV();

	for (int i = 0; i < size(); i++)
		*retV[i] += *getRefTensor().ref(genIndices(i)) + (val);

	return *new ConcreteTensor<T>(*new std::vector<T*>(retV) , shape, ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator -(T val){

    std::vector<T*>& retV = getV();
    std::vector<T*>& refV = getRefTensor().getV();

	for (int i = 0; i < size(); i++)
		*retV[i] -= *getRefTensor().ref(genIndices(i)) - (val);

	return *new ConcreteTensor<T>(*new std::vector<T*>(retV) , shape, ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator *(T val){

    std::vector<T*>& retV = getV();
    std::vector<T*>& refV = getRefTensor().getV();

	for (int i = 0; i < size(); i++){
		*retV[i] =  *getRefTensor().ref(genIndices(i)) * (val);
    }

	return *new ConcreteTensor<T>(*new std::vector<T*>(retV) , shape, ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator /(T val){

    std::vector<T*>& retV = getV();
    std::vector<T*>& refV = getRefTensor().getV();

	for (int i = 0; i < size(); i++)
		*retV[i] /= *getRefTensor().ref(genIndices(i)) / (val);

	return *new ConcreteTensor<T>(*new std::vector<T*>(retV) , shape, ud);
}


template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator +(_Tensor<T>& obj){

//    auto& ret = clone();
	auto& ret = broadcast(obj, true);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = ret.getRefTensor().getV();
    int nSize(noTrimed.size());
	*ret.ref(noTrimed.genIndices(0)) += (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ret.ref(noTrimed.genIndices(i)) += *getRefTensor().ref(noTrimed.genIndices(i)) + (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(ret.getV()), ret.getRefTensor().shape, ret.getRefTensor().ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator -(_Tensor<T>& obj){

//    auto& ret = clone();
	auto& ret = broadcast(obj, true);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = ret.getRefTensor().getV();
    int nSize(noTrimed.size());
	*ret.ref(noTrimed.genIndices(0)) -= (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ret.ref(noTrimed.genIndices(i)) -= *getRefTensor().ref(noTrimed.genIndices(i)) - (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(ret.getV()), ret.getRefTensor().shape, ret.getRefTensor().ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator *(_Tensor<T>& obj){

//    auto& ret = clone();
	auto& ret  = broadcast(obj, true);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = ret.getRefTensor().getV();
    int nSize(noTrimed.size());
	*ret.ref(noTrimed.genIndices(0)) = *ref(noTrimed.genIndices(0)) * (*obj.ref(noTrimed.genIndices(0)));
	*noTrimed.ref(noTrimed.genIndices(0)) = *ret.ref(noTrimed.genIndices(0));
	for (int i = 1; i < nSize; i++){
		*ret.ref(noTrimed.genIndices(i)) = (*ref(noTrimed.genIndices(i)) *  *noTrimed.getRefTensor().ref(noTrimed.genIndices(i))) * (*obj.ref(noTrimed.genIndices(i)));
	    *noTrimed.ref(noTrimed.genIndices(i)) = *ret.ref(noTrimed.genIndices(i));
    }

	return *new ConcreteTensor<T>(*new std::vector<T*>(ret.getV()), ret.getRefTensor().shape, ret.getRefTensor().ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator /(_Tensor<T>& obj){

//    auto& ret = clone();
	auto& ret = broadcast(obj, true);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = ret.getRefTensor().getV();
    int nSize(noTrimed.size());
	*ret.ref(noTrimed.genIndices(0)) /= (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ret.ref(noTrimed.genIndices(i)) /= *getRefTensor().ref(noTrimed.genIndices(i)) / (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(ret.getV()), ret.getRefTensor().shape, ret.getRefTensor().ud);
}


template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator+=(_Tensor<T>& obj)
{
	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = getRefTensor().getV();
    int nSize(noTrimed.size());
	*ref(noTrimed.genIndices(0)) += (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed.genIndices(i)) += *getRefTensor().ref(noTrimed.genIndices(i)) + (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(getV()), refTensor->shape, refTensor->ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator-=(_Tensor<T>& obj)
{
	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = getRefTensor().getV();
    int nSize(noTrimed.size());
	*ref(noTrimed.genIndices(0)) -= (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed.genIndices(i)) -= *getRefTensor().ref(noTrimed.genIndices(i)) - (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(getV()), refTensor->shape, refTensor->ud);
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator*=(_Tensor<T>& obj)
{
	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = getRefTensor().getV();
    int nSize(noTrimed.size());
	*ref(noTrimed.genIndices(0)) *= (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed.genIndices(i)) *= *getRefTensor().ref(noTrimed.genIndices(i)) * (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(getV()), refTensor->shape, refTensor->ud);
}


template<typename T>
_Tensor<T>& ReferenceTensor<T>::operator/=(_Tensor<T>& obj)
{
	_Tensor<T>& noTrimed = broadcast(obj);
    std::vector<T*>  refV = getRefTensor().getV();
    int nSize(noTrimed.size());
	*ref(noTrimed.genIndices(0)) /= (*obj.ref(noTrimed.genIndices(0)));
	for (int i = 1; i < nSize; i++) 
		*ref(noTrimed.genIndices(i)) /= *getRefTensor().ref(noTrimed.genIndices(i)) / (*obj.ref(noTrimed.genIndices(i)));

	return *new ConcreteTensor<T>(*new std::vector<T*>(getV()), refTensor->shape, refTensor->ud);
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

    std::vector<T*>& ref_v = refTensor->getV();

    ref_v[0] = iniVal;

    for(int i=0; i<size()-1; i++)
        ref_v[i+1] = (v)[i];
}


template<typename T>
bool ReferenceTensor<T>::isRefTensor() {
	return refTensor;
}

template<typename T>
_Tensor<T>& ReferenceTensor<T>::clone(){
	auto& ret = *new ReferenceTensor(*this);
    ret.iniVal = new T(*iniVal);
    ret.genReferenceTensor();
    return ret;
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
