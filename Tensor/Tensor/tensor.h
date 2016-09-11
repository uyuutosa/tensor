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

#define NUM_OF_WRAP      5 //! < The number of turn down 
                           //!   for output line.
#define NUM_OF_PRECISION 5 //! < The precision of output value.

//! @class 

template <typename T>
class tensor {
public:

	//! @brief Empty constructor.
    tensor(){}

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @params[in] _v     input value(one dimensional array).
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	tensor(
		std::vector<T> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @params[in] _shape input shape value.
	//! @params[in] _ud    input super(up) or subscript(down) value.
	tensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	//! @brief Constructor with input value and indices.
	//! @params[in] _v      input value.
	//! @params[in] indices index values(ex. "ijk").
	tensor(
		std::vector<std::vector<T> > _v,
   		std::string indices);

//・Operators・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*


	virtual std::shared_ptr<tensor<T> > operator + (std::shared_ptr<tensor<T> > obj);
	virtual std::shared_ptr<tensor<T> > operator - (std::shared_ptr<tensor<T> > obj);
	virtual std::shared_ptr<tensor<T> > operator * (std::shared_ptr<tensor<T> > obj);
	virtual std::shared_ptr<tensor<T> > operator / (std::shared_ptr<tensor<T> > obj);

	virtual std::shared_ptr<tensor<T> > operator + (T val);
	virtual std::shared_ptr<tensor<T> > operator - (T val);
	virtual std::shared_ptr<tensor<T> > operator * (T val);
	virtual std::shared_ptr<tensor<T> > operator / (T val);

	virtual std::shared_ptr<tensor<T> > operator []  (std::shared_ptr<tensor<T> > obj);

	//virtual std::shared_ptr<tensor<T> > operator + (std::shared_ptr<tensor<std::shared_ptr<T>>> obj);

	//virtual std::shared_ptr<tensor<T*> > operator + (T* val);
	//virtual std::shared_ptr<tensor<T*> > operator - (T* val);
	//virtual std::shared_ptr<tensor<T*> > operator * (T* val);
	//virtual std::shared_ptr<tensor<T*> > operator / (T* val);

//    template<typename U>
//	virtual std::shared_ptr<tensor<U> > operator *(U val);
	
	//! @brief Get total size of tensor.
	//! @retvals total size of tensor.
	int size();

	int ndim; //!< The number of dimensions.

	//std::map<std::string, int> getIdx();
	//std::map<std::string, int> getShape();
	std::map<std::string, int> genIndices(int i);
	T& ref(std::map<std::string, int> indices);

    //tensor<T> gft(std::map<std::string, int> shape,  T (*_f)(int, T));

	void view();

//	std::map<std::string, int> genRevIdx();

	void setV(std::vector<T> &obj);
	std::vector<T>& getV();

    std::shared_ptr<tensor<T*>>& getRefTensor();
    //tensor<T&>& getRefTensor();

//	std::map<std::string, int> revIdx; // Vector dimensons and element name(ex. i,j,k...).
	std::vector<std::string> idx;      // Vector dimensons and element name(ex. i,j,k...).
	std::map<std::string, int> shape;  // Shape of tensor.1
	std::map<std::string, int> ud;     //!< Upper or downer script(super or sub script).
	                                   //   Upper = 1, downer = 0.

	std::map<std::string, int> step;



	virtual std::shared_ptr<tensor<T> > broadcast(std::shared_ptr<tensor<T> > obj);

    template<typename U>
    std::shared_ptr<tensor<U> > operator *(U val);

    int iniVal;
	
protected:

	std::vector<T> v;

    std::shared_ptr<tensor<T*>>  refTensor;


	std::map<std::string, int> calcStep();

	bool isExistThisIndex(std::map<std::string, int> indices, std::string i);


	void putLeftBracket(int i);
	void putValue(int i);
	void putRightBracket(int i);


//	std::map<std::string, std::string> D;


	int N;

};

#include "tensor_detail.h"
//#include "tensorUtil.h"