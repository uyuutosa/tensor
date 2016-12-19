
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
////#pragma optimize("", off)

template <typename T>
class Tensor {
public:

	//! @brief Empty constructor.
    Tensor():
        _t(new ConcreteTensor<T>()) 
        {}


    Tensor(std::shared_ptr<_Tensor<T>> __t):
        refCnt(new int(1)),
        shape(__t->shape), 
        ud(__t->ud),
        _t(__t)
        {}

	//! @brief     Copy constractor.
    //! @param[in] obj a Tensor<T>.
    Tensor(const Tensor &obj):
        shape(obj._t->shape), ud(obj._t->ud)
    {
        _t = obj._t->clone();
    }

	//! @brief     Constructor with input value, Indices, shape and sup. or subscript infomations.
	//! @param[in] _v     Input value(one dimensional array).
    //! @param[in] _idx   Indices correspondance between _idx and _shape.
	//! @param[in] _shape Input shape value.
	//! @param[in] _ud    Input super(up) or subscript(down) value.
	Tensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief     Constructor with input value, shape and sup. or subscript infomations.
	//! @param[in] _v     Input value(one dimensional array).
	//! @param[in] _shape Input shape value.
	//! @param[in] _ud    Input super(up) or subscript(down) value.
	Tensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @param[in] _shape input shape value.
	//! @param[in] _ud    input super(up) or subscript(down) value.
	Tensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	//! @brief Constructor with input value and indices.
	//! @param[in] _v      input value.
	//! @param[in] indices index values(ex. "ijk").
	Tensor(
		std::shared_ptr<std::vector<std::vector<std::shared_ptr<T>>>> _v,
   		std::string indices);

	//! @brief     Slice value.
	//! @param[in] minIdx Min indices of the Tensor<T>. 
	//! @param[in] maxIdx Max indices of the Tensor<T>.
    Tensor<T> slice(std::map<std::string, int> minIdx, std::map<std::string, int> maxIdx);

	//! @brief     Convert type of element of Tensor<T>, and type of Tensor<T> itself.
	//! @param[in] U Type of element of Tensor<T>
	//! @param[in] W Type of Tensor<T> itself.
    template<typename U,typename W>
    Tensor<U> convertTo();

    //! @brief     Calc inverse matrix for two axis of Tensor<T>.
    //! @param[in] indices indices contains name of the axis of a Tensor<T>.
    Tensor<T> inv(std::string indices);

    //! @brief     Sum of Tensor<T> for 'indices'.
    //! @param[in] indices indices contains name of the axis of a Tensor<T>.
    Tensor<T> sum(std::string indices);

    //! @brief     Mean of Tensor<T> for 'indices'.
    //! @param[in] indices indices contains name of the axis of a Tensor<T>.
    Tensor<T> mean(std::string indices);

    //! @brief     Minimum value of Tensor<T> for 'indices'.
    //! @param[in] indices 'indices' contains name of the axis of a Tensor<T>.
    Tensor<T> min(std::string indices);

    //! @brief     Maximum value of Tensor<T> for 'indices'.
    Tensor<T> max(std::string indices);

    //! @brief Signeture of the Tensor<T>.
    Tensor<T> sign();

    //! @brief norm of the Tensor<T> for 'indices'.
    //! @param[in] indices 'indices' contains name of the axis of a Tensor<T>.
    Tensor<T> norm(std::string indices);

    //! @brief transpose' contains shape, ud, which is used 
    Tensor<T> putBack(std::string indices);

	Tensor<T> cud(std::string  first, ...);

	Tensor<T> cidx(std::string first, ...);

    //! @brief  Reshape of _Tensor<T>.
	Tensor<T> reshape(std::map<std::string, int> _shape, std::map<std::string, int> _ud );

    ////! @brief     
	////! @param[in] shape Input shape value.
	////! @param[in] ud    Input super(up) or subscript(down) value.
    Tensor<T>& t(
             std::map<std::string, int> shape, 
             std::map<std::string, int> ud, 
             ...);


    //! @brief     Gradient of Tensor<T>.
    //! @param[in] obj  
    Tensor<T> grad(Tensor<T>& obj, std::map<std::string, int> indices, double delta);

    Tensor<T> substitute(Tensor<T> &obj, std::map<std::string, int> _indices);

    template<typename U>
    Tensor<U> merge();

//    Tensor<T> concat(Tensor<T>& obj, std::string i);
    Tensor<T> concat(Tensor<T>& obj, 
                      std::string thisIdx, 
                      std::string objIdx, 
                      std::string concaIdx, 
                      int udVal);

	std::shared_ptr<_Tensor<T>> getT();

	std::shared_ptr<std::vector<std::shared_ptr<T>>> getV();

    std::map<std::string, int> getShape();
    std::map<std::string, int> getUd();


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
//	virtual Tensor<T> operator ^ (T val);

    virtual Tensor<T>& operator << (T val);
    virtual Tensor<T>& operator ,  (T val);

	virtual void operator = (T val);
	virtual void operator = (Tensor<T>& obj);

    virtual Tensor<T>& operator +=  (T val);
    virtual Tensor<T>& operator -=  (T val);
    virtual Tensor<T>& operator *=  (T val);
    virtual Tensor<T>& operator /=  (T val);
//	virtual Tensor<T>& operator ^= (T val);
    virtual Tensor<T>& operator +=  (Tensor<T> &obj);
    virtual Tensor<T>& operator -=  (Tensor<T> &obj);
    virtual Tensor<T>& operator *=  (Tensor<T> &obj);
    virtual Tensor<T>& operator /=  (Tensor<T> &obj);

    Tensor<bool> operator == (T val);
    Tensor<bool> operator >  (T val);
    Tensor<bool> operator <  (T val);
    Tensor<bool> operator >= (T val);
    Tensor<bool> operator <= (T val);
    Tensor<bool> operator == (Tensor<T> &obj);
    Tensor<bool> operator >  (Tensor<T> &obj);
    Tensor<bool> operator <  (Tensor<T> &obj);
    Tensor<bool> operator >= (Tensor<T> &obj);
    Tensor<bool> operator <= (Tensor<T> &obj);

    //bool operator <  (Tensor<T> &obj);
    //bool operator >  (Tensor<T> &obj);

    //template<typename U>
    //bool operator <  (U val);
    //template<typename U>
    //bool operator >  (U val);


    template<typename U>
	Tensor<T> suffix (Tensor<U> &obj, std::string sfx);

    Tensor<T> operator [] (std::map<std::string, int> &_indices);

    template<typename U>
	Tensor<T>  operator []  (Tensor<U> &obj);

	std::shared_ptr<T>& ref(std::map<std::string, int> indices);
//	virtual Tensor<T>& operator =  (T& val);

	int size();

	void view(std::ostream &os=std::cout);

    template<typename U>
    friend std::ostream& operator <<(std::ostream &os, Tensor<U>& obj);


    template<typename U>
    Tensor<U> operator *(U& val);

    ~Tensor(){
       // if(!--(*refCnt)){
       //     if(_t)
       //         delete _t;
       //     delete refCnt;
       // }
    }

    std::map<std::string, int> shape, ud;
protected:
    int* refCnt;

    //_Tensor<T>  _t;
    std::shared_ptr<_Tensor<T>>  _t;
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
