#pragma once
#include "FunctionTensor.h"

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
	std::vector<T*> &_v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _Tensor<T>(_v, _idx, _shape, _ud)
{
}

template<typename T>
FunctionTensor<T>::FunctionTensor(
	std::vector<T*> &_v,
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
    auto flst = funcTensor.getV();
    funcTensor.ref(indices) = genFunctionElementOnAnyRank(indices, _f);
}

template<typename T>
bool FunctionTensor<T>::isFunctionTensor(){
    return true;
}

template<typename T>
FunctionElement<T>* FunctionTensor<T>::genFunctionElementOnAnyRank(
    std::map<std::string, int> indices, 
    void* _f){
    
    auto ret = new FunctionElement<T>();
    switch (indices.size())
    {
    case 1:
        ret = new FunctionElementRank1<T>(indices, (T& (*)(int, T&))_f);
        break;
    case 2:
        ret = new FunctionElementRank2<T>(indices, (T& (*)(int, int, T&))_f);
        break;
    case 3:
        ret = new FunctionElementRank3<T>(indices, (T& (*)(int, int, int, T&))_f);
        break;
    case 4:
        ret = new FunctionElementRank4<T>(indices, (T& (*)(int, int, int, int, T&))_f);
        break;
    case 5:
        ret = new FunctionElementRank5<T>(indices, (T& (*)(int, int, int, int, int, T&))_f);
        break;
    case 6:
        ret = new FunctionElementRank6<T>(indices, (T& (*)(int, int, int, int, int, int, T&))_f);
        break;
    case 7:
        ret = new FunctionElementRank7<T>(indices, (T& (*)(int, int, int, int, int, int, int, T&))_f);
        break;
    case 8:
        ret = new FunctionElementRank8<T>(indices, (T& (*)(int, int, int, int, int, int, int, int, T&))_f);
        break;
    case 9:
        ret = new FunctionElementRank9<T>(indices, (T& (*)(int, int, int, int, int, int, int, int, int, T&))_f);
        break;
    case 10:
        ret = new FunctionElementRank10<T>(indices, (T& (*)(int, int, int, int, int, int, int, int, int, int, T&))_f);
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

    std::vector<FunctionElement<T>* > &flst =
        * new std::vector<FunctionElement<T>* > (total);
    funcTensor = * new _Tensor<FunctionElement<T>>(flst, _shape);

    for(int i=0; i<flst.size(); i++)        
        flst[i] = genFunctionElementOnAnyRank(funcTensor.genIndices(i), _f);

    funcTensor.setV(flst);
}

template<typename T>
_Tensor<T>& FunctionTensor<T>::operator *(T val){
    std::vector<T*> retV(size());
    auto funcV = funcTensor.getV();

	for (int i = 0; i < size(); i++)
		(*retV[i]) = *(funcV[i]) * val;

	return _Tensor<T>(*new ConcreteTensor<T>(retV, shape));
}

template<typename T>
template<typename U>
_Tensor<U>& FunctionTensor<T>::operator*(U val)
{
    std::vector<U*> retV(size());
    std::vector<FunctionElement<T>*>& funcV = funcTensor.getV();

	for (int i=0; i<size(); i++)
		(*retV[i]) = *funcV[i] * val;

	return *new ConcreteTensor<U>(retV, shape, ud);
}

template<typename T>
_Tensor<T>& FunctionTensor<T>::operator *(_Tensor<T>& obj) {

	_Tensor<T>& ret = *new _Tensor<T>();
	_Tensor<T>& noTrimed = broadcast(obj);

	if (obj.isRefTensor()) {
		ret = *new ReferenceTensor<T>(obj.iniVal, broadcast(obj,true).shape);
		std::vector<T*>& retV = ret.getV();
		for (int i = 0; i < noTrimed.size(); i++)
			ret.ref(noTrimed.genIndices(i)) = new T(*funcTensor.ref(noTrimed.genIndices(i)) * (**ret.getRefTensor().ref(noTrimed.genIndices(i))));
	} else {
		ret = broadcast(obj, true);
		std::vector<T*>& retV = ret.getV();
		for (int i = 0; i < noTrimed.size(); i++){
			ret.ref(noTrimed.genIndices(i))  = new T(*funcTensor.ref(noTrimed.genIndices(i)) * *obj.ref(noTrimed.genIndices(i)));
        }
	}
	return *new ConcreteTensor<T>(ret.getV(), ret.shape, ret.ud);
}

//template<typename T>
//_Tensor<T>& FunctionTensor<T>::grad(_Tensor<T>& obj, std::map<std::string, int> indices, double delta){
//    _Tensor<T>& obj2 = _Tensor<T>(obj.shape, obj.ud);

//    for (int i=0; i < N; i++){
//        obj2.getV()[i] = obj.getV()[i].clone();
//        obj2.getV()[i] = obj2.getV()[i].substitute(obj2.getV()[i][indices] + delta, indices);
//       // obj.getV()[i].view();
//       // obj2.getV()[i].view();
//    }
//    
//    

//    _Tensor<T>& ret1 = (*this * obj2);
//    _Tensor<T>& ret2 = (*this * obj); 
//    //ret1.view();
//    //ret2.view();

//    for (int i=0; i < N; i++)
//        ret1.getV()[i] = (ret1.getV()[i] - ret2.getV()[i]) / delta;
//    return ret1;
//}

//template<typename T>
//std::shared_ptr<Tensor<T> > FunctionTensor<T>::operator+(std::shared_ptr<Tensor<T> > obj)
//{
//	return std::shared_ptr<Tensor<T> >();
//}

template<typename T>
_Tensor<T>& FunctionTensor<T>::clone(){
	return *new auto(*this);
}

template<typename T>
_Tensor<T>&	FunctionTensor<T>::gen(
                    std::vector<T*> &_v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){
	return *new FunctionTensor<T>(_v, _idx, _shape, _ud);
}

template<typename T>
_Tensor<T>& FunctionTensor<T>::gen(
                std::vector<T*> &_v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){
	return *new FunctionTensor<T>(_v, _shape, _ud);
}

template<typename T>
_Tensor<T>& FunctionTensor<T>::gen(
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud){
	return *new FunctionTensor<T>(_shape, _ud);
}
