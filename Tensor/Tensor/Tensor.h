
//!
//! @file   Tensor.h
//! @brief  The class can versatile calculation 
//!         for multidimensional array.
//! @author Yu Sato

#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <iomanip>
#include <string>
#include <cstdarg>
#include "_Tensor.h"
//#pragma optimize("", off)

template <typename T>
class Tensor {
public:

	//! @brief Empty constructor.
    Tensor():
        _t(new ConcreteTensor<T>()), 
        refCnt(new int(1)),
        shape(*new std::map<std::string, int>()), ud(*new std::map<std::string, int>())
        {}


    Tensor(_Tensor<T>& __t):
        refCnt(new int(1)),
        shape(__t.shape), 
        ud(__t.ud)
        {_t = &__t;}

    Tensor(const Tensor &obj):
        refCnt(obj.refCnt),
        shape(obj._t->shape), ud(obj._t->ud)
    {
        ++(*refCnt);
        _t = &obj._t->clone();
    }

	Tensor(
		std::vector<T*> &_v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @params[in] _v     input value(one dimensional array).
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	Tensor(
		std::vector<T*> &_v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	Tensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	//! @brief Constructor with input value and indices.
	//! @params[in] _v      input value.
	//! @params[in] indices index values(ex. "ijk").
	Tensor(
		std::vector<std::vector<T*> > &_v,
   		std::string indices);

    Tensor<T> slice(std::map<std::string, int> minIdx, std::map<std::string, int> maxIdx);


    template<typename U,typename W>
    Tensor<U> convertTo();

    Tensor<T> inv(std::string indices);

    Tensor<T> sum(std::string indices);

    Tensor<T> mean(std::string indices);

    Tensor<T> sign();

    Tensor<T> norm(std::string indices);

    Tensor<T> putBack(std::string indices);

	Tensor<T>& cud(std::string  first, ...);

	Tensor<T>& cidx(std::string first, ...);

    Tensor<T>& t(
             std::map<std::string, int> shape, 
             std::map<std::string, int> ud, 
             ...);


    Tensor<T> grad(Tensor<T>& obj, std::map<std::string, int> indices, double delta);

    Tensor<T> substitute(Tensor<T> &obj, std::map<std::string, int> _indices);

    template<typename U>
    Tensor<U>& merge();

//    Tensor<T> concat(Tensor<T>& obj, std::string i);
    Tensor<T> concat(Tensor<T>& obj, 
                      std::string thisIdx, 
                      std::string objIdx, 
                      std::string concaIdx, 
                      int udVal);

	_Tensor<T>* getT();

	std::vector<T*>& getV();

    std::map<std::string, int>& getShape();
    std::map<std::string, int>& getUd();


    Tensor<T> clone();

    //! @brief Substitute a function to a element of tensor.
    void substituteFunc(void* _f, std::map<std::string, int> indices);

//・Operators・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*


	virtual Tensor<T> operator + (Tensor<T>& obj);
	virtual Tensor<T> operator - (Tensor<T>& obj);
	virtual Tensor<T> operator * (Tensor<T>& obj);
	virtual Tensor<T> operator / (Tensor<T>& obj);

	virtual Tensor<T> operator + (T val);
	virtual Tensor<T> operator - (T val);
	virtual Tensor<T> operator * (T val);
	virtual Tensor<T> operator / (T val);
	virtual Tensor<T> operator ^ (T val);

    virtual Tensor<T>& operator << (T val);
    virtual Tensor<T>& operator ,  (T val);

	virtual Tensor<T>& operator = (T val);
	virtual Tensor<T>& operator = (Tensor<T>& obj);

    virtual Tensor<T>& operator +=  (T val);
    virtual Tensor<T>& operator -=  (T val);
    virtual Tensor<T>& operator *=  (T val);
    virtual Tensor<T>& operator /=  (T val);
	virtual Tensor<T>& operator ^= (T val);
    virtual Tensor<T>& operator +=  (Tensor<T> &obj);
    virtual Tensor<T>& operator -=  (Tensor<T> &obj);
    virtual Tensor<T>& operator *=  (Tensor<T> &obj);
    virtual Tensor<T>& operator /=  (Tensor<T> &obj);

    bool operator <  (Tensor<T> &obj);
    bool operator >  (Tensor<T> &obj);

    template<typename U>
    bool operator <  (U val);
    template<typename U>
    bool operator >  (U val);

    Tensor<T> operator [] (std::map<std::string, int> &_indices);

    template<typename U>
	Tensor<T>  operator []  (Tensor<U> &obj);

	T*& ref(std::map<std::string, int> indices);
//	virtual Tensor<T>& operator =  (T& val);

	int size();

	void view(std::ostream &os=std::cout);

    template<typename U>
    friend std::ostream& operator <<(std::ostream &os, Tensor<U>& obj);


    template<typename U>
    Tensor<U> operator *(U& val);

    ~Tensor(){
        //if(!--(*refCnt)){
        //    if(_t)
        //        delete _t;
        //    delete refCnt;
        //}
    }

    std::map<std::string, int> shape, ud;
protected:
    int* refCnt;

    //_Tensor<T>  _t;
    _Tensor<T>*  _t;
};


template<typename U>
std::ostream& operator <<(std::ostream &os, Tensor<U>& obj){
    os << std::endl;
    obj.view(os);
    return os;    
}

#include "Tensor_detail.h"
//#include "TensorUtil.h"

#include "ConcreteTensor.h"
#include "FunctionTensor.h"
#include "ReferenceTensor.h"
#include "Tensor_operators.h"
#include "TensorUtil.h"
