#pragma once
#include <memory>
#include "ConcreteTensor.h"




template<typename T>
class ReferenceTensor:
    public tensor<T>{
public:
    ReferenceTensor(T _iniVal,
		            std::map<std::string, int> _shape,
					std::map<std::string, int> _ud=std::map<std::string, int>());
//    ReferenceTensor(std::shared_ptr<tensor<T> > _iniVal, 
//		            std::map<std::string, int> _shape,
//					std::map<std::string, int> _ud=std::map<std::string, int>());


    void genReferenceTensor(T iniVal);

    std::shared_ptr<tensor<T> > operator *(T val);
	std::shared_ptr<tensor<T> > operator * (std::shared_ptr<tensor<T> > obj);
private:


};

#include "ReferenceTensor_detail.h"