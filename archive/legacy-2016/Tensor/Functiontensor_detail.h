#pragma once
#include "FunctionTensor.h"
////#pragma optimize ("", off)

template<typename T>
FunctionTensor<T>::FunctionTensor(
	void* _f,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud):
	_Tensor<T>(_shape, _ud){
    genFunctionTensor(shape, _f);
}

template<typename T>
FunctionTensor<T>::FunctionTensor(
	std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _idx, _shape, _ud)
{
}

template<typename T>
FunctionTensor<T>::FunctionTensor(
	std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _shape, _ud)
{
}

template<typename T>
FunctionTensor<T>::FunctionTensor(
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_shape, _ud)
{
}

template<typename T>
void FunctionTensor<T>::substituteFunc(void* _f, std::map<std::string, int> indices){
    auto flst = funcTensor->getV();
    funcTensor->ref(indices) = genFunctionElementOnAnyRank(indices, _f);
}

template<typename T>
bool FunctionTensor<T>::isFunctionTensor(){
    return true;
}

template<typename T>
std::shared_ptr<FunctionElement<T>> FunctionTensor<T>::genFunctionElementOnAnyRank(
    std::map<std::string, int> indices, 
    void* _f){
    
    auto ret = std::shared_ptr<FunctionElement<T>>(new FunctionElement<T>());
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

    auto flst =
        std::shared_ptr<std::vector<std::shared_ptr<FunctionElement<T>>>>(new std::vector<std::shared_ptr<FunctionElement<T>>>(total));

    funcTensor = std::shared_ptr<_Tensor<FunctionElement<T>>>(new _Tensor<FunctionElement<T>>(flst, _shape));

    for(int i=0; i<flst->size(); i++)        
        (*flst)[i] = genFunctionElementOnAnyRank(funcTensor->genIndices(i), _f);

    funcTensor->setV(flst);
}

template<typename T>
std::shared_ptr<_Tensor<T>> FunctionTensor<T>::operator *(T val){
    auto retV = std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>(size()));
    auto funcV = funcTensor->getV();

	for (int i = 0; i < size(); i++)
		*(*retV)[i] = *(*funcV)[i] * val;

	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(retV, shape, ud));
}

template<typename T>
template<typename U>
std::shared_ptr<_Tensor<U>> FunctionTensor<T>::operator*(U val)
{
    auto retV  = std::shared_ptr<std::vector<U*>>(new  std::vector<U*>(size()));
    auto funcV = funcTensor->getV();

	for (int i=0; i<size(); i++)
		*(*retV)[i] = *(*funcV)[i] * val;

	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(retV, shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> FunctionTensor<T>::operator *(std::shared_ptr<_Tensor<T>> obj) {

	auto ret = std::shared_ptr<_Tensor<T>>();
	auto noTrimed = broadcast(obj);
    this;
	if (obj->isRefTensor()) {
		ret = std::shared_ptr<_Tensor<T>>(new ReferenceTensor<T>(*obj->iniVal, broadcast(obj,true)->shape));
		auto retV = ret->getV();
		for (int i = 0; i < noTrimed->size(); i++)
			*ret->ref(noTrimed->genIndices(i)) = *funcTensor->ref(noTrimed->genIndices(i)) * (*ret->getRefTensor()->ref(noTrimed->genIndices(i)));
	} else {
		ret = broadcast(obj, true);
		auto retV = ret->getV();
		for (int i = 0; i < noTrimed->size(); i++){
			auto kkk  = *funcTensor->ref(noTrimed->genIndices(i)) * *obj->ref(noTrimed->genIndices(i));
			*ret->ref(noTrimed->genIndices(i))  = *funcTensor->ref(noTrimed->genIndices(i)) * *obj->ref(noTrimed->genIndices(i));
        }
	}
	return std::shared_ptr<_Tensor<T>>(new ConcreteTensor<T>(ret->getV(), ret->shape, ret->ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> FunctionTensor<T>::grad(std::shared_ptr<_Tensor<T>> obj, std::map<std::string, int> indices, double delta){
    auto obj2 = gen(obj->shape, obj->ud);

    for (int i=0; i < N; i++){
        *(*obj2->getV())[i] = (*(obj->getV()))[i]->clone();
        *(*obj2->getV())[i] = (*(obj2->getV()))[i]->substitute((*(*(obj2->getV()))[i])[indices] + delta, indices);
    }

    auto ret1 = (*this * obj2);
    auto ret2 = (*this * obj); 

    for (int i=0; i < N; i++)
        *(*ret1->getV())[i] = (*(*ret1->getV())[i] - *(*ret2->getV())[i]) / delta;
    return ret1;
}

//template<typename T>
//std::shared_ptr<Tensor<T> > FunctionTensor<T>::operator+(std::shared_ptr<Tensor<T> > obj)
//{
//	return std::shared_ptr<Tensor<T> >();
//}

template<typename T>
std::shared_ptr<_Tensor<T>> FunctionTensor<T>::clone(){
	return std::shared_ptr<_Tensor<T>>(new auto(*this));
}

template<typename T>
std::shared_ptr<_Tensor<T>>	FunctionTensor<T>::gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new FunctionTensor<T>(_v, _idx, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> FunctionTensor<T>::gen(
                std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new FunctionTensor<T>(_v, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> FunctionTensor<T>::gen(
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new FunctionTensor<T>(_shape, _ud));
}
