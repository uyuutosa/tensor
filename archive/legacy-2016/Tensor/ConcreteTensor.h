#pragma once
#include "_Tensor.h"
#include "TensorUtil.h"
#include <opencv2/opencv.hpp>
////#pragma optimize("", off)


template<typename T>
class ConcreteTensor:
    public _Tensor<T>{
public:
// Constructors ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
    ConcreteTensor():_Tensor<T>(){}

	ConcreteTensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>()):
            _Tensor<T>(_v, _idx, _shape, _ud){}

	ConcreteTensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>()):
            _Tensor<T>(_v, _shape, _ud){}

	ConcreteTensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>()):
            _Tensor<T>(_shape, _ud){}

	ConcreteTensor(
		std::shared_ptr<std::vector<std::vector<T>>> _v,
   		std::string indices):
            _Tensor<T>(_v, indicies){}

//・Public member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
    //// Manipulate of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

    //! @brief Deep copy of _Tensor<T>.
    std::shared_ptr<_Tensor<T>> clone();

    //! @brief Gen of new _Tensor<T>.
    virtual std::shared_ptr<_Tensor<T>> gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

    virtual std::shared_ptr<_Tensor<T>> gen(std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

	virtual std::shared_ptr<_Tensor<T>>	gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud=std::map<std::string, int>);


//・Operators・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

	std::shared_ptr<_Tensor<T>> operator + (std::shared_ptr<_Tensor<T>> obj);
	std::shared_ptr<_Tensor<T>> operator - (std::shared_ptr<_Tensor<T>> obj);
	std::shared_ptr<_Tensor<T>> operator * (std::shared_ptr<_Tensor<T>> obj);
	std::shared_ptr<_Tensor<T>> operator / (std::shared_ptr<_Tensor<T>> obj);
//	std::shared_ptr<_Tensor<T>> operator ^ (std::shared_ptr<_Tensor<T>> obj);

	std::shared_ptr<_Tensor<T>> operator + (T val);
	std::shared_ptr<_Tensor<T>> operator - (T val);
	std::shared_ptr<_Tensor<T>> operator * (T val);
	std::shared_ptr<_Tensor<T>> operator / (T val);
//	std::shared_ptr<_Tensor<T>> operator ^ (T val);

    void operator +=  (T val);
    void operator -=  (T val);
    void operator *=  (T val);
    void operator /=  (T val);
//    void operator ^=  (T val);
    void operator += (std::shared_ptr<_Tensor<T>> obj);
    void operator -=  (std::shared_ptr<_Tensor<T>> obj);
    void operator *=  (std::shared_ptr<_Tensor<T>> obj);
    void operator /=  (std::shared_ptr<_Tensor<T>> obj);
//    void operator ^=  (std::shared_ptr<_Tensor<T>> obj);

    bool operator < (std::shared_ptr<_Tensor<T>> obj);
    bool operator > (std::shared_ptr<_Tensor<T>> obj);
};

#include "ConcreteTensor_detail.h"