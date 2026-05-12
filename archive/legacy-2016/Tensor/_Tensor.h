//!
//! @file   _Tensor.h
//! @brief  This tensor class can versatile calculation 
//!         for multidimensional array.
//! @author Yu Sato

#pragma once
#include <vector>
#include <iostream>
#include <memory>
#include <map>
#include <iomanip>
#include <string>
#include <opencv2/opencv.hpp>

#define NUM_OF_WRAP      5 //! < The number of turn down 
                           //!   for output line.
#define NUM_OF_PRECISION 5 //! < The precision of output value.



//! @brief   Abstract class of tensor.
//! @details This tensor class enable to perform
//!          versatile Tensor calculation followed by 
//!          an calculation rule.
//!        
template <typename T>
class _Tensor{
public:

// Constructors ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
	//! @brief Empty constructor.
    _Tensor(): 
        iniCnt(0) 
       {N=0;}

	//! @brief Constructor that handle only one value,
    //!        which is used when tensor of tensor have no value.
    _Tensor(T val): 
        iniCnt(0) 
       {N=0; 
        v->push_back(std::shared_ptr<T>(new T(val)));}

	//! @brief      Copy constructor.
    //! @param[in] obj  A Tensor<T>.
    _Tensor(const _Tensor<T> &obj): 
        iniCnt(0)
       {
        //auto a = (obj.getV());

        // Initialize values of _Tensor<T>.
        auto objV = obj.getV();
        v = std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>(objV->size(), 0));
        for(int i=0; i<v->size(); i++)
            (*v)[i] = std::shared_ptr<T>(new T(*(*objV)[i]));

        // Deep copy of any member variables.
        N = v->size();
        shape = obj.shape;
        ud    = obj.ud;
        idx    = obj.idx;
        step = obj.step;
        viewStep = obj.viewStep;
        viewIdx = obj.viewIdx;
        ndim = obj.ndim;
        }

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @param[in] _v     Input value(one dimensional array).
	//! @param[in] _shape Input shape value.
	//! @param[in] _ud    Input super(up) or subscript(down) value.
	_Tensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::vector<std::string> _idx,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor with input value, shape and sup. or subscript infomations.
	//! @param[in] _v     Input value(one dimensional array).
	//! @param[in] _shape Input shape value.
	//! @param[in] _ud    Input super(up) or subscript(down) value.
	_Tensor(
		std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor shape and sup. or subscript infomations.
	//! @param[in] _shape Input shape value.
	//! @param[in] _ud    Input super(up) or subscript(down) value.
	_Tensor(
   		std::map<std::string, int> _shape,
   		std::map<std::string, int> _ud = std::map<std::string, int>())
        ;

	//! @brief Constructor with input value and indices.
	//! @param[in] _v      Input value.
	//! @param[in] indices Index values(ex. "ijk").
	_Tensor(
		std::shared_ptr<std::vector<std::vector<std::shared_ptr<T>>>> _v,
   		std::string indices)
        ;

//・Public member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

    // Manipulate of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

    //! @brief Slice of _Tensor<T>.
    //! @param[in] minIdx Minimum index number.
    //! @param[in] maxIdx Maximum index number.
    std::shared_ptr<_Tensor<T>> slice(std::map<std::string, int> minIdx, 
                                      std::map<std::string, int> maxIdx);

    //! @brief Convert type of _Tensor<T>, such as 
    //!        ConcreteTensor<T>, ReferenceTensor<T>, FunctionTensor<T>.
    //! @param[in] U Change type.
    //! @param[in] W Change _Tensor<T> type.
    template<typename U, typename W>
    std::shared_ptr<_Tensor<U>> convertTo();

    //! @brief Substritute of _Tensor<T>(obsolute).
    //! @param[in] obj      _Tensor<T> for substitution.
    //! @param[in] _indices Substituton indices of _Tensor<T>.
    std::shared_ptr<_Tensor<T>> substitute(std::shared_ptr<_Tensor<T>> obj, std::map<std::string, int> _indices);

    //! @brief   Merge nested tensor.
    //! @details This member function will be only used for _Tensor<Tensor<T>>.
    template<typename U>
	std::shared_ptr<_Tensor<U>> merge();

    //! @brief      Put back selected axis end of 'idx'
    //! @params[in] indices Putting back indices.
    std::shared_ptr<_Tensor<T>> putBack(std::string indices);


    //! @brief Deep copy of _Tensor<T>.
    virtual std::shared_ptr<_Tensor<T>> clone();

    //! @brief     Generate of new _Tensor<T>.
    //! @details   This function is used for getting a new subclass _Tensor<T>,
    //!            such as ConcreteTensor<T>, ReferenceTensor<T>, FunctionTensor<T>.
    //! @param[in] _v     values.
    //! @param[in] _shape Shape of _Tensor<T>.
    //! @param[in] _ud    Up-down assignemt as 1 or 0.
    virtual std::shared_ptr<_Tensor<T>> gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

    //! @brief     Generate of new _Tensor<T>.
    //! @details   This function is used for getting a new subclass _Tensor<T>,
    //!            such as ConcreteTensor<T>, ReferenceTensor<T>, FunctionTensor<T>.
    //! @param[in] _shape Shape of _Tensor<T>.
    //! @param[in] _ud    Up-down assignemt as 1 or 0.
    virtual std::shared_ptr<_Tensor<T>> gen(std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud=std::map<std::string, int>);

    //! @brief     Generate of new _Tensor<T>.
    //! @details   This function is used for getting a new subclass _Tensor<T>,
    //!            such as ConcreteTensor<T>, ReferenceTensor<T>, FunctionTensor<T>.
    //! @param[in] _v     values.
    //! @param[in] idx    Index of assignement.
    //! @param[in] _shape Shape of _Tensor<T>.
    //! @param[in] _ud    Up-down assignemt as 1 or 0.
	virtual std::shared_ptr<_Tensor<T>>	gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud=std::map<std::string, int>);


    //! @brief     Concatenate two tensors.
    //! @details   Concatenate two tensors, 'this' and 'obj'.
    //!            Then, need to select concatenate axis,
    //!            'thisIdx' and 'objIdx' from 'this' and 'obj'. 
    //!            Concatenated axis is named as 'concaIdx'.
    //! @param[in] thisIdx  This   indEx for concatenation. 
    //! @param[in] objIdx   Object index for concatenation. 
    //! @param[in] concaIdx Index after concatenation(same on thisIdx or obj Idx are OK).
    //! @param[in] udVal    Up-down assigment after concatenation.
    std::shared_ptr<_Tensor<T>> concat(
                       std::shared_ptr<_Tensor<T>> obj, 
                       std::string thisIdx, 
                       std::string objIdx, 
                       std::string concaIdx, 
                       int udVal);

    //! @brief         Change indices at super or substript to indices at sub or superscript.
    //! @param[in] map a std::map<std::string, int> contains 
    //!                the pairs of eixist label and changing label.
	std::shared_ptr<_Tensor<T>> cud(
                                    std::map<std::string, int> map);

    //! @brief         Change indices label of _Tensor<T>.
    //! @param[in] map a std::map<std::string, std::string> contains 
    //!                the pairs of eixist label and changing label.
	std::shared_ptr<_Tensor<T>> cidx(
        std::map<std::string, std::string> map);

    //! @brief  Reshape of _Tensor<T>.
	std::shared_ptr<_Tensor<T>> reshape(
                        std::map<std::string, int> _shape, 
                        std::map<std::string, int> _ud );

    //! @brief Set object.
	void setV(std::shared_ptr<std::vector<std::shared_ptr<T>>> obj);

    //! @brief Get std::vector<T> contained data.
	std::shared_ptr<std::vector<std::shared_ptr<T>>> getV() const;

    // Get infomation of Tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

    //! @brief Generate indices of elementdata from 
    //!        the index of when grance of data as 
    //!        one dimensional array. 
	std::map<std::string, int> genIndices(int i);

	//! @brief Get total size of _Tensor.
	//! @retval total size of _Tensor.
	int size();

    //! @brief     Get reference of element of data.
    //! @param[in] indices indices of element of data.
	std::shared_ptr<T>& ref(std::map<std::string, int> indices);

    //! @brief View infomation of tensor, shape, ud and data.
	void view(std::ostream& os=std::cout);


    virtual std::shared_ptr<_Tensor<T>> getRefTensor();

    //! @brief      broadcast tensor using this and obj.
    //! @details    Using two tensor, 
    //!             shape informations included this and obj,
    //!             get a broadcasted tensor which contains zeros.
    //! @param[in] obj    a Tensor<T>.
    //! @param[in] trimUd select whether trim 'ud' in case of odd ud but same idx.
    //!                    This flag used for inner product.
	virtual std::shared_ptr<_Tensor<T>> broadcast(std::shared_ptr<_Tensor<T>> obj,
                                                  bool trimUd=false);

    //! @brief  Whether or not a _Tensor<T> has reference Tensor.
    //! @retval bool if 'this' has reference tensor 
    //!         then return true, else return false.
    bool isRefTensor();

    //! @brief  Whether or not a _Tensor<T> is FunctionTensor<T>.
    //! @retval bool if 'this' has reference tensor 
    //!         then return true, else return false.
    virtual bool isFunctionTensor();

    // Math・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

    //! @brief      Calculate invert matrix for seleting two axes.
    //! @params[in] indices Two indices of indices. Need to enter two characters.
    std::shared_ptr<_Tensor<T>> inv(std::string indices);

    std::shared_ptr<_Tensor<T>> sum(std::string indices);

    std::shared_ptr<_Tensor<T>> mean(std::string indices);

    std::shared_ptr<_Tensor<T>> max(std::string indices);

    std::shared_ptr<_Tensor<T>> min(std::string indices);

    std::shared_ptr<_Tensor<T>> sign();

    std::shared_ptr<_Tensor<T>> norm(std::string indices);
    

    //! @brief Calculate gradient of 'obj'.
    virtual std::shared_ptr<_Tensor<T>> grad (
                                              std::shared_ptr<_Tensor<T>> obj, 
                                              std::map<std::string, int> indices, 
                                              double delta);

    //! @brief Substitute a function to a element of tensor.
    virtual void substituteFunc(void* _f, std::map<std::string, int> indices);

//・Operators ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
    //! @brief same Operators.
    template<typename U>
    std::shared_ptr<_Tensor<U>> operator *(U val);

    // On _Tensor<T>.
	virtual std::shared_ptr<_Tensor<T>> operator + (std::shared_ptr<_Tensor<T>> obj);
	virtual std::shared_ptr<_Tensor<T>> operator - (std::shared_ptr<_Tensor<T>> obj);
	virtual std::shared_ptr<_Tensor<T>> operator * (std::shared_ptr<_Tensor<T>> obj);
	virtual std::shared_ptr<_Tensor<T>> operator / (std::shared_ptr<_Tensor<T>> obj);
//	virtual std::shared_ptr<_Tensor<T>> operator ^ (std::shared_ptr<_Tensor<T>> obj);
    
    // On T.
	virtual std::shared_ptr<_Tensor<T>> operator + (T val);
	virtual std::shared_ptr<_Tensor<T>> operator - (T val);
	virtual std::shared_ptr<_Tensor<T>> operator * (T val);
	virtual std::shared_ptr<_Tensor<T>> operator / (T val);
//	virtual std::shared_ptr<_Tensor<T>> operator ^ (T val);

    // For initialize.
    virtual void operator << (T val);
    virtual void operator ,  (T val);

    // 
    virtual void operator +=  (T val);
    virtual void operator -=  (T val);
    virtual void operator *=  (T val);
    virtual void operator /=  (T val);
    virtual void operator ^=  (T val);
    virtual void operator +=  (std::shared_ptr<_Tensor<T>> obj);
    virtual void operator -=  (std::shared_ptr<_Tensor<T>> obj);
    virtual void operator *=  (std::shared_ptr<_Tensor<T>> obj);
    virtual void operator /=  (std::shared_ptr<_Tensor<T>> obj);
//    virtual void operator ^=  (std::shared_ptr<_Tensor<T>> obj);

    template<typename U>
    bool operator <  (std::shared_ptr<U> val);
    template<typename U>
    bool operator >  (std::shared_ptr<U> val);

    std::shared_ptr<_Tensor<bool>> operator == (T val);
    std::shared_ptr<_Tensor<bool>> operator >  (T val);
    std::shared_ptr<_Tensor<bool>> operator <  (T val);
    std::shared_ptr<_Tensor<bool>> operator >= (T val);
    std::shared_ptr<_Tensor<bool>> operator <= (T val);
    std::shared_ptr<_Tensor<bool>> operator == (std::shared_ptr<_Tensor<T>> obj);
    std::shared_ptr<_Tensor<bool>> operator >  (std::shared_ptr<_Tensor<T>> obj);
    std::shared_ptr<_Tensor<bool>> operator <  (std::shared_ptr<_Tensor<T>> obj);
    std::shared_ptr<_Tensor<bool>> operator >= (std::shared_ptr<_Tensor<T>> obj);
    std::shared_ptr<_Tensor<bool>> operator <= (std::shared_ptr<_Tensor<T>> obj);

    //! @brief      Substitution a value.
    //! @params[in] val Input value.
	virtual void operator = (T val);


    //! @brief      Substitution a value.
    //! @param[in] val Input value.
	virtual void operator = (std::shared_ptr<_Tensor<T>> obj){

	    auto noTrimed = broadcast(obj);

        if(N == obj->size())
            for(int i=0; i<N; i++)
                *(*v)[i] = *(*obj->v)[i];
        else
            v = obj->v;

        idx      = obj->idx;
        shape    = obj->shape;
        ud       = obj->ud;
        step     = obj->step;
        viewIdx  = obj->viewIdx;
        viewStep = obj->viewStep;
        N = obj->N;
        ndim = obj->ndim;
        trimedSize = obj->trimedSize;
        //return std::shared_ptr<_Tensor<T>>(this);
        //return obj;
        //return *new _Tensor<T>(obj);
    }

    template<typename U>
	std::shared_ptr<_Tensor<T>> suffix (std::shared_ptr<_Tensor<U>> obj, std::string sfx);



    //! @brief     Get trimed Tensor for _indices.
    //! @param[in] _indices trimming indices.
    //! @retval    trimed tensor.
    virtual std::shared_ptr<_Tensor<T>> operator [] (std::map<std::string, int> &_indices);

    //! @brief     Get trimed Tensor for _indices.
    //! @param[in] _indices trimming indices.
    //! @retval    trimed tensor.
    template<typename U>
    std::shared_ptr<_Tensor<T>> operator [] (std::shared_ptr<_Tensor<U>> obj);

//・Public member variables ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
	
	int ndim; //!< The number of dimensions.
	std::vector<std::string> idx,viewIdx; //!< Vector dimensons and element name(ex. i,j,k...).
	std::map<std::string, int> shape;     //!< Shape of _Tensor.1
	std::map<std::string, int> ud;        //!< Upper or downer script(super or sub script).
	                                      //   Upper = 1, downer = 0.

	std::map<std::string, int> step;      //!< Steps of each axes.
	std::map<std::string, int> viewStep;  //!< Steps of each axes for view.

    int trimedSize;                       //!< trimed size used by _Tensor::inv().

    std::shared_ptr<T> iniVal;                             //!< initial value for reference tensor.

    int iniCnt;

    ~_Tensor<T>(){
        //if(refTensor)
        //    delete refTensor->getV()[0];
        //else
        //    for(int i=0; i<v.size(); i++)
        //	    if(v[i] != nullptr){
        //	        delete v[i];
        //	        v[i] = nullptr;
        //	        if(v[i] == nullptr);
        //	            std::cout<<"h"<<std::endl;
        //	}
    }

    virtual void genReferenceTensor();
	
protected:

//・protected member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

	std::map<std::string, int> calcStep(); 

    // For view・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
	void putLeftBracket(int i, std::ostream &os);
	void putValue(int i, std::ostream &os);
	void putRightBracket(int i, std::ostream &os);

    //! @brief  Return flag of whether a 'indices' contains index 'i'. 
	bool isExistThisIndex(std::map<std::string, int> indices, std::string i);

//・protected member variables ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

	std::shared_ptr<std::vector<std::shared_ptr<T>>> v;                      //!< a vector contains data.

    std::shared_ptr<_Tensor<T>>  refTensor;


	int N; //!< The number of data, which refered from _Tensor<T>::size();
};

#include "_Tensor_detail.h"