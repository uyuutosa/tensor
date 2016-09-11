#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <iomanip>
#include <string>
//#include "FunctionTensor.h"

#define NUM_OF_WRAP      5
#define NUM_OF_PRECISION 5


template <typename T>
class tensor {
public:
    tensor(){}

	tensor(
		std::vector<T> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

	tensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>());

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