#pragma once 
#include "ConcreteTensor.h"


//・Public member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
// Manipulate of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
template<typename T>
_Tensor<T>& ConcreteTensor<T>::clone(){
	return *new auto(*this);
}

template<typename T>
_Tensor<T>&	ConcreteTensor<T>::gen(std::vector<T*> &_v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){

	return *new ConcreteTensor<T>(_v, _idx, _shape, _ud);
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::gen(std::vector<T*> &_v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){

	return *new ConcreteTensor<T>(_v, _shape, _ud);
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::gen(
    std::map<std::string, int> _shape, 
    std::map<std::string, int> _ud){

	return *new ConcreteTensor<T>(_shape, _ud);
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator+(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

    int tmp = noTrimed.size();
	for (int i = 0; i < tmp; i++){
		*ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) + *obj.ref(noTrimed.genIndices(i));
    }

	return ret;
}


template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator-(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	for (int i = 0; i < noTrimed.size(); i++)
		*ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) - *obj.ref(noTrimed.genIndices(i));

	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator*(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	for (int i = 0; i < noTrimed.size(); i++){
        if(obj.isFunctionTensor()){
		    *ret.ref(noTrimed.genIndices(i)) +=  (*obj.ref(noTrimed.genIndices(i))) * (*ref(noTrimed.genIndices(i)));
        } else {
		    *ret.ref(noTrimed.genIndices(i)) +=  *ref(noTrimed.genIndices(i)) * (*obj.ref(noTrimed.genIndices(i)));
        }
    }

	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator/(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	for (int i = 0; i < noTrimed.size(); i++)
		*ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) / *obj.ref(noTrimed.genIndices(i));

	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator^(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

	for (int i = 0; i < noTrimed.size(); i++){
        T&tmp = *new T;
        tmp = 1;
	    for (int j = 0; j < *obj.ref(noTrimed.genIndices(i)); j++)
            tmp *= *ref(noTrimed.genIndices(i));
        *ret.ref(noTrimed.genIndices(i)) = tmp;
    }
            
		    //ret.ref(noTrimed.genIndices(i)) += ref(noTrimed.genIndices(i)) ^ obj.ref(noTrimed.genIndices(i));

	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator+=(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    for(auto a: ret.getV())
        *a = *v[0] * 0; 

    int tmp = noTrimed.size();
	for (int i = 0; i < tmp; i++){
		*ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) + (*obj.ref(noTrimed.genIndices(i)));
    }

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
    N     = ret.size();
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator-=(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    int tmp = noTrimed.size();
	for (int i = 0; i < tmp; i++){
		*ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) - (*obj.ref(noTrimed.genIndices(i)));
    }

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
    N     = ret.size();
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator*=(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    int tmp = noTrimed.size();
	for (int i = 0; i < tmp; i++){
        if(obj.isFunctionTensor())
		    *ret.ref(noTrimed.genIndices(i)) += (*obj.ref(noTrimed.genIndices(i))) * (*ref(noTrimed.genIndices(i)));
        else
		    *ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) * (*obj.ref(noTrimed.genIndices(i)));
    }

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
    N     = ret.size();
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator/=(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    int tmp = noTrimed.size();
	for (int i = 0; i < tmp; i++){
		*ret.ref(noTrimed.genIndices(i)) += *ref(noTrimed.genIndices(i)) / *obj.ref(noTrimed.genIndices(i));
    }

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
    N     = ret.size();
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator^=(_Tensor<T>& obj)
{
	_Tensor<T>& ret      = broadcast(obj, true);
	_Tensor<T>& noTrimed = broadcast(obj);

    int tmp = noTrimed.size();
	for (int i = 0; i < tmp; i++){
        T&tmp = *new T;
        tmp = 1;
	    for (int j = 0; j < *obj.ref(noTrimed.genIndices(i)); j++)
            tmp *= *ref(noTrimed.genIndices(i));
        *ret.ref(noTrimed.genIndices(i)) = tmp;
    }

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
    N     = ret.size();
	return *this;
}


template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator+(T val){
	
	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();

	for (int i = 0; i < ret.size(); i++)
		*(retV[i]) += (val);

	return ret;
}


template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator-(T val){

	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);
	
    std::vector<T*>& retV = ret.getV();
	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) -= (val);

	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator*(T val){

	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();
	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) *= (val);

	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator/(T val){

	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();
	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) /= (val);
	return ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator^(T val){

//	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);
	_Tensor<T>* ret = tu::ones<T>(shape, ud).getT();

    std::vector<T*>& retV = ret->getV();
    std::vector<T*>& V = getV();
	for (int i = 0; i < ret->size(); i++)
        for(int j=0; j<val; j++)
		    (*retV[i]) *= (*V[i]);

	return *ret;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator+=(T val){
	
	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();

	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) += (val);

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator-=(T val){
	
	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();

	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) -= (val);


    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator*=(T val){
	
	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();

	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) *= (val);

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator/=(T val){
	
	_Tensor<T>& ret = *new ConcreteTensor<T>(*this);

    std::vector<T*>& retV = ret.getV();

	for (int i = 0; i < ret.size(); i++)
		(*retV[i]) /= (val);

    v     = ret.getV();
    shape = ret.shape;
    ud    = ret.ud;
	return *this;
}

template<typename T>
_Tensor<T>& ConcreteTensor<T>::operator^=(T val){

	_Tensor<T>& ret = *tu::ones<T>(shape, ud).getT();

    std::vector<T*>& retV = ret.getV();
    std::vector<T*>& V = getV();
	for (int i = 0; i < ret.size(); i++)
        for(int j=0; j<val; j++)
		    (*retV[i]) *= (*V[i]);

    v     = retV;
    shape = ret.shape;
    ud    = ret.ud;
	return *this;
}


template<typename T>
bool ConcreteTensor<T>::operator<(_Tensor<T> &obj){

    std::vector<T*>& objV = obj.getV();
    std::vector<T*>& V = getV();
	for (int i = 0; i < obj.size(); i++)
        if((*V[i]) < (*objV[i]))
            break;

	return true;
}

template<typename T>
bool ConcreteTensor<T>::operator>(_Tensor<T> &obj){

    std::vector<T*>& objV = obj.getV();
    std::vector<T*>& V = getV();
	for (int i = 0; i < obj.size(); i++)
        if((*V[i]) > (*objV[i]))
            break;

	return true;
}
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
//    std::vector<T>& retV(obj.size());
//    std::vector<T>& objV = obj.getV();

//	for (int i = 0; i < obj.size(); i++)
//        retV[i] = idxV[objV[i]];  

//    return ConcreteTensor<T>(retV, obj.shape, obj.ud);
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
