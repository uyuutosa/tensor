#pragma once
#include <memory>
#include "Tensor.h"
#include "FunctionElement.h"
#include "ConcreteTensor.h"


////#pragma optimize("", off)

template<typename T>
class FunctionTensor:
    public _Tensor<T>{
public:
	FunctionTensor(
		void* _f,
		std::map<std::string, int> _shape,
		std::map<std::string, int> _ud=std::map<std::string, int>());

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @param[in] _v     input value(one dimensional array).
	//! @param[in] _shape input shape value.
	//! @param[in] _ud    input super(up) or subscript(down) value.
	FunctionTensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @param[in] _v     input value(one dimensional array).
	//! @param[in] _shape input shape value.
	//! @param[in] _ud    input super(up) or subscript(down) value.
	FunctionTensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @param[in] _shape input shape value.
	//! @param[in] _ud    input super(up) or subscript(down) value.
	FunctionTensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;


    template<typename U>
    std::shared_ptr<_Tensor<U>> operator *(U val);
    std::shared_ptr<_Tensor<T>> operator *(T val);
	std::shared_ptr<_Tensor<T>> operator *(std::shared_ptr<_Tensor<T>> obj);

    std::shared_ptr<_Tensor<T>> grad(std::shared_ptr<_Tensor<T>> obj, std::map<std::string, int> indices, double delta);

    //! @brief Deep copy of _Tensor<T>.
    std::shared_ptr<_Tensor<T>> clone();

    //! @brief Gen of new _Tensor<T>.
    virtual std::shared_ptr<_Tensor<T>> gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

    virtual std::shared_ptr<_Tensor<T>> gen(
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

	virtual std::shared_ptr<_Tensor<T>>	gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud=std::map<std::string, int>);

    //! @brief Substitute a function to a element of tensor.
    void substituteFunc(void* _f, std::map<std::string, int> indices);

    virtual bool isFunctionTensor();

private:

    std::shared_ptr<_Tensor<FunctionElement<T>>> funcTensor;

    std::shared_ptr<FunctionElement<T>> genFunctionElementOnAnyRank(
        std::map<std::string, int> indices, 
        void* _f);

    void genFunctionTensor(
        std::map<std::string, int> _shape,
    	void* _f);


};

#include "FunctionTensor_detail.h"
