#pragma once
#include "tensor.h"


//

template<typename T>
class ConcreteTensor:
    public tensor<T>{
public:

	ConcreteTensor(
		std::vector<T> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>()):
            tensor<T>(_v, _shape, _ud){}

	ConcreteTensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>()):
            tensor<T>(_shape, _ud){}

	ConcreteTensor(
		std::vector<std::vector<T> > _v,
   		std::string indices):
            tensor<T>(_v, indicies){}


    std::shared_ptr<tensor<T> > broadcast(std::shared_ptr<tensor<T> > obj);
//・Operators・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*


	std::shared_ptr<tensor<T> > operator + (std::shared_ptr<tensor<T> > obj);
	std::shared_ptr<tensor<T> > operator - (std::shared_ptr<tensor<T> > obj);
	std::shared_ptr<tensor<T> > operator * (std::shared_ptr<tensor<T> > obj);
	std::shared_ptr<tensor<T> > operator / (std::shared_ptr<tensor<T> > obj);

	std::shared_ptr<tensor<T> > operator + (T val);
	std::shared_ptr<tensor<T> > operator - (T val);
	std::shared_ptr<tensor<T> > operator * (T val);
	std::shared_ptr<tensor<T> > operator / (T val);

	std::shared_ptr<tensor<T> > operator [](std::shared_ptr<tensor<T> > obj);
};

#include "ConcreteTensor_detail.h"