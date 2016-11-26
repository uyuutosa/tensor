#pragma once
#include <memory>
#include "ConcreteTensor.h"




template<typename T>
class ReferenceTensor:
    public _Tensor<T>{
public:
    ReferenceTensor(T _iniVal,
		            std::map<std::string, int> _shape,
					std::map<std::string, int> _ud=std::map<std::string, int>());
//    ReferenceTensor(std::shared_ptr<_Tensor<T> > _iniVal, 
//		            std::map<std::string, int> _shape,
//					std::map<std::string, int> _ud=std::map<std::string, int>());

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @params[in] _v     input value(one dimensional array).
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	ReferenceTensor(
		std::vector<T*> &_v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @params[in] _v     input value(one dimensional array).
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	ReferenceTensor(
		std::vector<T*> &_v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	ReferenceTensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;


    void genReferenceTensor();

    _Tensor<T>& operator +(T val);
    _Tensor<T>& operator -(T val);
    _Tensor<T>& operator *(T val);
    _Tensor<T>& operator /(T val);
	_Tensor<T>& operator + (_Tensor<T>& obj);
	_Tensor<T>& operator - (_Tensor<T>& obj);
	_Tensor<T>& operator * (_Tensor<T>& obj);
	_Tensor<T>& operator / (_Tensor<T>& obj);

    //_Tensor<T>& operator +=  (_Tensor<T>& obj);
    //_Tensor<T>& operator +=  (T val);
    //_Tensor<T>& operator -=  (T val);
    //_Tensor<T>& operator *=  (T val);
    //_Tensor<T>& operator /=  (T val);
    //_Tensor<T>& operator ^=  (T val);
    _Tensor<T>& operator +=  (_Tensor<T> &obj);
    _Tensor<T>& operator -=  (_Tensor<T> &obj);
    _Tensor<T>& operator *=  (_Tensor<T> &obj);
    _Tensor<T>& operator /=  (_Tensor<T> &obj);
//    _Tensor<T>& operator ^=  (_Tensor<T> &obj);

    //virtual _Tensor<T*>& getRefTensor();
    bool isRefTensor();

    //! @brief Deep copy of _Tensor<T>.
    _Tensor<T>& clone();

    //! @brief Gen of new _Tensor<T>.
    virtual _Tensor<T>& gen(std::vector<T*> &_v, 
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

    virtual _Tensor<T>& gen(std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

	virtual _Tensor<T>&	gen(std::vector<T*> &_v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud=std::map<std::string, int>);

private:



};

#include "ReferenceTensor_detail.h"