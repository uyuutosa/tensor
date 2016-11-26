#pragma once
#include "Tensor.h"
//#pragma optimize ("", off)
template<typename T>
Tensor<T>::Tensor(
	std::vector<T*> &_v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud):
    _t(new ConcreteTensor<T>(_v, _idx, _shape, _ud)), refCnt(new int(1)),
    shape(_shape), ud(_ud)
{
    shape = _t.shape;
}

template<typename T>
Tensor<T>::Tensor(
	std::vector<T*> &_v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _t(new ConcreteTensor<T>(_v, _shape, _ud)), refCnt(new int(1)),
    shape(_shape), ud(_ud)
{
}

template<typename T>
Tensor<T>::Tensor(
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
    _t(new ConcreteTensor<T>(_shape, _ud)), refCnt(new int(1)),
    shape(_shape), ud(_ud)
{ 
}

template<typename T>
Tensor<T>::Tensor(
	std::vector<std::vector<T*> > &_v,
	std::string indices):
    _t(new ConcreteTensor<T>(_v, _indices)), refCnt(new int(1)),
    shape(_shape), ud(_ud)
{
}


template<typename T>
int Tensor<T>::size()
{
	return _t->size();
}

template<typename T>
void Tensor<T>::view(std::ostream &os){
    _t->view(os);
}

template<typename T>
Tensor<T> Tensor<T>::slice(std::map<std::string, int> minIndices, std::map<std::string, int> maxIndices){
    return Tensor<T>(_t->slice(minIndices, maxIndices));
}

template<typename T>
template<typename U, typename W>
Tensor<U> Tensor<T>::convertTo(){
    return Tensor<U>(_t->convertTo<U, W>());
}

template<typename T>
Tensor<T> Tensor<T>::inv(std::string indices){
    return Tensor<T>(_t->inv(indices));
}

template<typename T>
Tensor<T> Tensor<T>::sum(std::string indices){
    return Tensor<T>(_t->sum(indices));
}

template<typename T>
Tensor<T> Tensor<T>::mean(std::string indices){
    return Tensor<T>(_t->mean(indices));
}

template<typename T>
Tensor<T> Tensor<T>::sign(){
    return Tensor<T>(_t->sign());
}

template<typename T>
Tensor<T> Tensor<T>::norm(std::string indices){
    return Tensor<T>(_t->norm(indices));
}

template<typename T>
Tensor<T> Tensor<T>::putBack(std::string indices){
    return Tensor<T>(_t->putBack(indices));
}


template<typename T>
Tensor<T>& Tensor<T>::cidx(std::string first, ...){
    va_list args;
    va_start(args, first);

    std::map<std::string, std::string> ret;

    int cnt(1);
    std::string name(first);
    while (true) {
        if(!(cnt % 2))
            name = std::string(va_arg(args, char*));
        if (!name.size()){
            va_end(args);
            return *new Tensor<T>(_t.cidx(ret));
        }
        if(cnt % 2)
            ret[name] = va_arg(args, char*);
        cnt++;
    } 
}

template<typename T>
Tensor<T>& Tensor<T>::cud(std::string first, ...){
    va_list args;
    va_start(args, first);

    std::map<std::string, int> ret;

    int cnt(1);
    std::string name(first);
    while (true) {
        if(!(cnt % 2))
            name = std::string(va_arg(args, char*));
        if (!name.size()){
            va_end(args);
            return *new Tensor<T>(_t.cud(ret));
        }
        if(cnt % 2)
            ret[name] = va_arg(args, int);
        cnt++;
    } 
}

template<typename T>
Tensor<T> Tensor<T>::grad(Tensor<T>& obj, std::map<std::string, int> indices, double delta){
    return Tensor<T>(_t->grad(*obj._t, indices, delta));
}

template<typename T>
Tensor<T> Tensor<T>::substitute(Tensor<T> &obj, std::map<std::string, int> _indices){
    return Tensor<T>(_t->substitute(*obj._t, _indices));
}

template<typename T>
template<typename U>
Tensor<U>& Tensor<T>::merge(){
    _Tensor<U>& ret_t = _t->merge<U>();
    Tensor<U>&  ret   = *new Tensor<U>(ret_t);
    ret.shape         = std::map<std::string, int>(ret_t.shape);
    ret.ud            = std::map<std::string, int>(ret_t.ud);
    return ret;
}

template<typename T>
Tensor<T> Tensor<T>::concat(Tensor<T>& obj, std::string thisIdx, std::string objIdx, std::string concaIdx, int udVal){
    return Tensor<T>(_t->concat(*obj.getT(), thisIdx, objIdx, concaIdx, udVal));
}

template<typename T>
_Tensor<T>* Tensor<T>::getT(){
    return _t;
}

template<typename T>
std::vector<T*>& Tensor<T>::getV(){
    return _t->getV();
}

template<typename T>
std::map<std::string, int>& Tensor<T>::getShape(){
    return _t->shape;
}

template<typename T>
std::map<std::string, int>& Tensor<T>::getUd(){
    return _t->ud;
}

template<typename T>
Tensor<T> Tensor<T>::clone(){
    return Tensor<T>(_t->clone());
}


template<typename T>
void Tensor<T>::substituteFunc(void* _f, std::map<std::string, int> indices){
   _t->substituteFunc(_f, indices);
}

template<typename T>
template<typename U>
Tensor<U> Tensor<T>::operator *(U& val) {
	return Tensor<U>( *_t * val);
}

template<typename T>
Tensor<T> Tensor<T>::operator+(Tensor<T>& obj)
{ 
	return Tensor<T>( *_t + *obj._t);
}


template<typename T>
Tensor<T> Tensor<T>::operator-(Tensor<T>& obj)
{
	return Tensor<T>( *_t - *obj._t);
}

template<typename T>
Tensor<T> Tensor<T>::operator*(Tensor<T>& obj)
{
    if(obj.getT()->isFunctionTensor())
    	return Tensor<T>( (*obj._t) * (*_t));
    else
	    return Tensor<T>( *_t * (*obj._t));
}

template<typename T>
Tensor<T> Tensor<T>::operator/(Tensor<T>& obj)
{
	return Tensor<T>( *_t / (*obj._t));
}

template<typename T>
Tensor<T>& Tensor<T>::operator =(T val)
{
    *_t =  val;
    //*_t =  val;
    return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator=(Tensor<T>& obj)
{
    //if(!--(*refCnt))
    //    delete _t;
    *_t     = *obj._t;
    refCnt = obj.refCnt;
    ++(*refCnt);
    return obj;
}


template<typename T>
Tensor<T> Tensor<T>::operator+(T val){
	return Tensor<T>(*_t + val);
}


template<typename T>
Tensor<T> Tensor<T>::operator-(T val){
	return Tensor<T>(*_t - val);
}

template<typename T>
Tensor<T> Tensor<T>::operator*(T val){
	return Tensor<T>(*_t * val);
}

template<typename T>
Tensor<T> Tensor<T>::operator/(T val){
	return Tensor<T>(*_t / val);
}

template<typename T>
Tensor<T> Tensor<T>::operator^(T val){
	return Tensor<T>(*_t ^ val);
}

template<typename T>
Tensor<T>& Tensor<T>::operator+=(Tensor<T> &obj)
{
    *_t += *obj._t;
	return *this;
}


template<typename T>
Tensor<T>& Tensor<T>::operator-=(Tensor<T> &obj)
{
    *_t -= *obj._t;
	return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator*=(Tensor<T> &obj)
{
    if(obj.getT()->isFunctionTensor()){
        *_t = *obj._t * *_t;
    } else {
        *_t *= *obj._t;
    }
	return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator/=(Tensor<T> &obj)
{
    *_t /= *obj._t;
	return *this;
}


template<typename T>
Tensor<T>& Tensor<T>::operator+=(T val)
{
    *_t += val;
	return *this;
}


template<typename T>
Tensor<T>& Tensor<T>::operator-=(T val)
{
    *_t -= val;
	return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator*=(T val)
{
    *_t *= val;
	return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator/=(T val)
{
    *_t /= val;
	return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator^=(T val)
{
    *_t ^= val;
	return *this;
}

template<typename T>
Tensor<T> Tensor<T>::operator [] (std::map<std::string, int> &_indices)
{
	return Tensor<T>((*_t)[_indices]);
}

template<typename T>
template<typename U>
Tensor<T> Tensor<T>::operator []  (Tensor<U> &obj){
	return Tensor<T>((*_t)[*obj.getT()]);
}

template<typename T>
Tensor<T>& Tensor<T>::operator <<(T val){
    *_t << val;
	return *this;
}

template<typename T>
Tensor<T>& Tensor<T>::operator ,(T val){
    *_t , val;
	return *this;
}

template<typename T>
T*& Tensor<T>::ref(std::map<std::string, int> indices){
    return _t->ref(indices);
}


template<typename T>
bool Tensor<T>::operator>(Tensor<T> &obj)
{
    return *_t > (*obj._t);
}

template<typename T>
bool Tensor<T>::operator<(Tensor<T> &obj)
{
    return *_t < (*obj._t);
}

template<typename T>
template<typename U>
bool Tensor<T>::operator<(U val)
{
    return *_t < val;
}

template<typename T>
template<typename U>
bool Tensor<T>::operator>(U val)
{
    return *_t > val;
}
//template<typename T>
//Tensor<T>& Tensor<T>::operator []  (Tensor<T> obj){
//    return Tensor<T>(_t[obj->_t]);
//}

//template<typename T>
//Tensor<T&>& Tensor<T>::operator []  (int _idx){
//    return Tensor<T&>(_t[idx]);
//}

//template<typename T>
//Tensor<T>& Tensor<T>::operator =  (T& val){
//	return Tensor<T>(_t = val);
//}
