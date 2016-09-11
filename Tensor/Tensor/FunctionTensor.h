#pragma once
#include <memory>
#include "tensor.h"
#include "FunctionElement.h"
#include "ConcreteTensor.h"


#pragma optimize("", off)

template<typename T>
class FunctionTensor:
    public tensor<T>{
public:
	FunctionTensor(
		void* _f,
		std::map<std::string, int> _shape,
		std::map<std::string, int> _map=std::map<std::string, int>());


    template<typename U>
    std::shared_ptr<tensor<U> > operator *(U val);
    std::shared_ptr<tensor<T> > operator *(T val);
	std::shared_ptr<tensor<T> > operator *(std::shared_ptr<tensor<T> > obj);

private:
    tensor<std::shared_ptr<FunctionElement<T> > > funcTensor;

    std::shared_ptr<FunctionElement<T>> genFunctionElementOnAnyRank(
        std::map<std::string, int> indices, 
        void* _f);

    void genFunctionTensor(
        std::map<std::string, int> _shape,
    	void* _f);


};

#include "FunctionTensor_detail.h"
