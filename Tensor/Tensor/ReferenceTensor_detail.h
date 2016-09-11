#pragma once
#include "ReferenceTensor.h"

template<typename T>
ReferenceTensor<T>::ReferenceTensor(
		T _iniVal, 
		std::map<std::string, int> _shape,
		std::map<std::string, int> _ud
	):
    tensor<T>(_shape, _ud){
    iniVal = _iniVal;
    genReferenceTensor(iniVal); 
}

//template<typename T>
//ReferenceTensor<T>::ReferenceTensor(std::shared_ptr<tensor<T> > _iniVal, std::map<std::string, int> _shape):
//    tensor<T>(_shape){
//    iniVal = _iniVal;
//    genReferenceTensor(iniVal); 

//}


template<typename T>
std::shared_ptr<tensor<T> > ReferenceTensor<T>::operator *(T val){

    std::vector<T>& retV = getV();
    std::vector<T*>& refV = getRefTensor()->getV();

	for (int i = 0; i < size(); i++)
		retV[i] = *getRefTensor()->ref(genIndices(i)) * val;
   		//retV[i] = (*(refV[i])) * val;

	return std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(std::vector<T>(retV) , shape));
}

template<typename T>
std::shared_ptr<tensor<T> > ReferenceTensor<T>::operator *(std::shared_ptr<tensor<T> > obj){

	auto refT = std::shared_ptr<tensor<T> >(new ReferenceTensor<T>(iniVal, broadcast(obj)->shape));
    std::vector<T>&  retV = refT->getV();

	for (int i = 0; i < refT->size(); i++) 
		retV[i] = *refT->getRefTensor()->ref(refT->genIndices(i)) * obj->ref(refT->genIndices(i));

	return std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(std::vector<T>(retV) , refT->shape));
}

//template<typename T>
//std::shared_ptr<tensor<T> > ReferenceTensor<T>::operator *(std::shared_ptr<tensor<T> > obj){

//    std::vector<T>& retV = getV();
//    std::vector<T*>& refV = getRefTensor()->getV();

//	for (int i = 0; i < size(); i++)
//		retV[i] = *getRefTensor()->ref(genIndices(i)) * val;
//   		//retV[i] = (*(refV[i])) * val;

//	return std::shared_ptr<tensor<T> >(new ConcreteTensor<T>(std::vector<T>(retV) , shape));
//}

template<typename T>
void ReferenceTensor<T>::genReferenceTensor(T iniVal){

    refTensor = std::shared_ptr<tensor<T*> >(new tensor<T*>(shape));

    std::vector<T*>& ref_v = refTensor->getV();

    ref_v[0] = new T(iniVal);

    for(int i=0; i<size()-1; i++)
        ref_v[i+1] = &(v)[i];
}