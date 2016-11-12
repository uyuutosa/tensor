#pragma once
#include <memory>
#include "Tensor.h"
#include "FunctionElement.h"
#include "ConcreteTensor.h"


//#pragma optimize("", off)

template<typename T>
class FunctionTensor:
    public _Tensor<T>{
public:
	FunctionTensor(
		void* _f,
		std::map<std::string, int> _shape,
		std::map<std::string, int> _ud=std::map<std::string, int>());

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @params[in] _v     input value(one dimensional array).
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	FunctionTensor(
		std::vector<T*> &_v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @params[in] _v     input value(one dimensional array).
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	FunctionTensor(
		std::vector<T*> &_v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	FunctionTensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;


    template<typename U>
    _Tensor<U>& operator *(U val);
    _Tensor<T>& operator *(T val);
	_Tensor<T>& operator *(_Tensor<T> &obj);

//    _Tensor<T>& grad(_Tensor<T>& obj, std::map<std::string, int> indices, double delta);

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

    //! @brief Substitute a function to a element of tensor.
    void substituteFunc(void* _f, std::map<std::string, int> indices);

    virtual bool isFunctionTensor();

private:

    _Tensor<FunctionElement<T>> funcTensor;

    FunctionElement<T>* genFunctionElementOnAnyRank(
        std::map<std::string, int> indices, 
        void* _f);

    void genFunctionTensor(
        std::map<std::string, int> _shape,
    	void* _f);


};

#include "FunctionTensor_detail.h"
