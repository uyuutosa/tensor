#pragma once 
#include "ConcreteTensor.h"
//#pragma optimize("", off)


//・Public member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
// Manipulate of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::clone(){
    //auto a(*this);// = gen(shape, ud);
    
	//return std::shared_ptr<_Tensor<T>>(a);
	return std::shared_ptr<_Tensor<T>>(new auto(*this));
}

template<typename T>
std::shared_ptr<_Tensor<T>>	ConcreteTensor<T>::gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){

	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(_v, _idx, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::gen(
                std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){

	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(_v, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::gen(
    std::map<std::string, int> _shape, 
    std::map<std::string, int> _ud){

	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(_shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator+(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    for(auto a: *(ret->getV())){
        *a = *(*v)[0] * 0; 
    }

    int tmp = noTrimed->size();
	for (int i = 0; i < tmp; i++){
		*ret->ref(noTrimed->genIndices(i)) += *ref(noTrimed->genIndices(i)) + *obj->ref(noTrimed->genIndices(i));
    }

	return ret;
}


template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator-(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	for (int i = 0; i < noTrimed->size(); i++)
		*ret->ref(noTrimed->genIndices(i)) += *ref(noTrimed->genIndices(i)) - *obj->ref(noTrimed->genIndices(i));

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator*(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	for (int i = 0; i < noTrimed->size(); i++){
        if(obj->isFunctionTensor()){
		    *ret->ref(noTrimed->genIndices(i)) +=  (*obj->ref(noTrimed->genIndices(i))) * (*ref(noTrimed->genIndices(i)));
        } else {
		    *ret->ref(noTrimed->genIndices(i)) +=  *ref(noTrimed->genIndices(i)) * (*obj->ref(noTrimed->genIndices(i)));
        }
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator/(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

	for (int i = 0; i < noTrimed->size(); i++)
		*ret->ref(noTrimed->genIndices(i)) += *ref(noTrimed->genIndices(i)) / *obj->ref(noTrimed->genIndices(i));

	return ret;
}

//template<typename T>
//std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator^(std::shared_ptr<_Tensor<T>> obj)
//{
//	auto ret      = broadcast(obj, true);
//	auto noTrimed = broadcast(obj);

//    for(auto a: *ret->getV())
//        *a = *(*v)[0] * 0; 

//	for (int i = 0; i < noTrimed->size(); i++){
//        auto tmp = std::shared_ptr<T>(new T);
//        *tmp = 1;
//	    for (int j = 0; j < *(obj->ref(noTrimed->genIndices(i))); j++)
//            *tmp *= *ref(noTrimed->genIndices(i));
//        ret->ref(noTrimed->genIndices(i)) = tmp;
//    }
//            
//	return ret;
//}

template<typename T>
void ConcreteTensor<T>::operator+=(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

    int tmp = noTrimed->size();
	for (int i = 0; i < tmp; i++){
		*ret->ref(noTrimed->genIndices(i)) += *ref(noTrimed->genIndices(i)) + (*obj->ref(noTrimed->genIndices(i)));
    }

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
    N     = ret->size();
}

template<typename T>
void ConcreteTensor<T>::operator-=(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    int tmp = noTrimed->size();
	for (int i = 0; i < tmp; i++){
		*ret->ref(noTrimed->genIndices(i)) += *ref(noTrimed->genIndices(i)) - (*obj->ref(noTrimed->genIndices(i)));
    }

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
    N     = ret->size();
}

template<typename T>
void ConcreteTensor<T>::operator*=(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    int tmp = noTrimed->size();
	for (int i = 0; i < tmp; i++){
        if(obj->isFunctionTensor())
		    *(ret->ref(noTrimed->genIndices(i))) += (*obj->ref(noTrimed->genIndices(i))) * (*ref(noTrimed->genIndices(i)));
        else
		    *(ret->ref(noTrimed->genIndices(i))) += *ref(noTrimed->genIndices(i)) * *(obj->ref(noTrimed->genIndices(i)));
    }

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
    N     = ret->size();
}

template<typename T>
void ConcreteTensor<T>::operator/=(std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true);
	auto noTrimed = broadcast(obj);

    int tmp = noTrimed->size();
	for (int i = 0; i < tmp; i++){
		*(ret->ref(noTrimed->genIndices(i))) += *ref(noTrimed->genIndices(i)) / *(obj->ref(noTrimed->genIndices(i)));
    }

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
    N     = ret->size();
}

//template<typename T>
//void ConcreteTensor<T>::operator^=(std::shared_ptr<_Tensor<T>> obj)
//{
//	auto ret      = broadcast(obj, true);
//	auto noTrimed = broadcast(obj);

//    int tmp = noTrimed->size();
//	for (int i = 0; i < tmp; i++){
//        auto tmp = std::shared_ptr<T>(new T);
//        *tmp = 1;
//	    for (int j = 0; j < *obj->ref(noTrimed->genIndices(i)); j++)
//            *tmp *= *ref(noTrimed->genIndices(i));
//        ret->ref(noTrimed->genIndices(i)) = tmp;
//    }

//    v     = ret->getV();
//    shape = ret->shape;
//    ud    = ret->ud;
//    N     = ret->size();
//}


template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator+(T val){
	
	//auto ret = std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(std::shared_ptr<_Tensor<T>>(this)));
	auto ret =  clone();

    auto retV = ret->getV();

	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] += (val);

	return ret;
}


template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator-(T val){
	//auto ret =  std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
	auto ret =  clone();
	
    auto retV = ret->getV();
	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] -= (val);

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator*(T val){
	//auto ret =  std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
	auto ret =  clone();

    auto retV = ret->getV();
	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] *= (val);

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator/(T val){
	//auto ret =  std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
	auto ret =  clone();

    auto retV = ret->getV();
	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] /= (val);

	return ret;
}

//template<typename T>
//std::shared_ptr<_Tensor<T>> ConcreteTensor<T>::operator^(T val){

//	auto ret = tu::ones<T>(shape, ud).getT();

//    auto retV = ret->getV();
//    auto V = getV();
//	for(int i=0; i<ret->size(); i++)
//        for(int j=0; j<val; j++)
//		    *(*retV)[i] *= *(*V)[i];

//	return ret;
//}

template<typename T>
void ConcreteTensor<T>::operator+=(T val){
	
	auto ret  = std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
    auto retV = ret->getV();

	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] += (val);

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
}

template<typename T>
void ConcreteTensor<T>::operator-=(T val){
	
	auto ret = std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
    auto retV = ret->getV();

	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] -= (val);

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
}

template<typename T>
void ConcreteTensor<T>::operator*=(T val){
	
	auto ret = std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
    auto retV = ret->getV();

	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] *= (val);

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
}

template<typename T>
void ConcreteTensor<T>::operator/=(T val){
	
	auto ret = std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(*this));
    auto retV = ret->getV();

	for (int i = 0; i < ret->size(); i++)
		*(*retV)[i] /= (val);

    v     = ret->getV();
    shape = ret->shape;
    ud    = ret->ud;
}

//template<typename T>
//void ConcreteTensor<T>::operator^=(T val){

//	auto ret = tu::ones<T>(shape, ud).getT();

//    auto retV = ret->getV();
//    auto V    = getV();

//	for (int i = 0;  i<ret->size(); i++)
//        for(int j=0; j<val; j++)
//		    *(*retV)[i] *= *(*V)[i];

//    v     = retV;
//    shape = ret->shape;
//    ud    = ret->ud;
//}


//template<typename T>
//_Tensor<T>& ConcreteTensor<T>::operator =(T& val){

//	_Tensor<T>& ret =  ConcreteTensor<T>(*this);

//    std::vector<T>& retV = ret.getV();
//	for (int i = 0; i < ret.size(); i++)
//		retV[i] = val;
//	return ret;
//}

//template<typename T>
//_Tensor<T>& ConcreteTensor<T>::operator [](_Tensor<T> &obj){
//    std::vector<T>& idxV = getV();
//    std::vector<T>& retV(obj->size());
//    std::vector<T>& objV = obj->getV();

//	for (int i = 0; i < obj->size(); i++)
//        retV[i] = idxV[objV[i]];  

//    return ConcreteTensor<T>(retV, obj->shape, obj->ud);
//}

//template<typename T>
//_Tensor<T&>& ConcreteTensor<T>::operator [] (int _idx){

//    std::map<std::string, int> retShape;
//    for(auto pair: shape)
//        if(idx[0] != pair.first)
//            retShape[pair.first] = pair.second;

//    std::map<std::string, int> retUd;
//    for(auto pair: ud)
//        if(idx[0] != pair.first)
//            retUd[pair.first] = pair.second;

//    std::shared_ptr<_Tensor<T&> > ret = std::shared_ptr<_Tensor<T> >(new ConcreteTensor<T>(retShape, retUd));

//    std::vector<T&>& retV = ret.getV();

//	for (int i = 0; i < ret.size(); i++)
//		retV[i] = ref(ret.genIndices(i));

//    return ret;
//}
