//////!
//////! @file   _Tensor_pointer.h
//////! @brief  The class can versatile calculation 
//////!         for multidimensional array.
//////! @author Yu Sato

//#pragma once
//#include <vector>
//#include <iostream>
//#include <memory>
//#include <map>
//#include <iomanip>
//#include <string>
//#include <opencv2/opencv.hpp>

//#define NUM_OF_WRAP      5 //! < The number of turn down 
//                           //!   for output line.
//#define NUM_OF_PRECISION 5 //! < The precision of output value.



////! @brief   Abstract class of tensor.
////! @details This tensor class enable to perform
////!          versatile Tensor calculation followed by 
////!          an calculation rule.
////!        
//template <typename T>
//class _Tensor<T*> {
//public:

//// Constructors ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
//	//! @brief Empty constructor.
//    _Tensor(): v(std::vector<T>())
//       {N=0;}

//	//! @brief Empty constructor.
//    _Tensor(const _Tensor<T> &obj): v(std::vector<T>())
//       {N=0;
//        std::vector<T> o = obj.getV();
//        v = *new std::vector<T>(o);
//        //v = *new std::vector<T>(obj.getV());
//        shape = obj.shape;
//        ud    = obj.ud;
//        idx    = obj.idx;
//        }

//	//! @brief Constructor with input value, shape and sup. or subscript infomations.
//	//! @params[in] _v     input value(one dimensional array).
//	//! @params[in] _shape input shape value.
//	//! @params[in] _ud    input super(up) or subscript(down) value.
//	_Tensor(
//		std::vector<T> _v,
//   		std::vector<std::string> _idx,
//   		std::map<std::string, int> _shape,
//   		std::map<std::string, int> _ud = std::map<std::string, int>())
//        ;

//	//! @brief Constructor with input value, shape and sup. or subscript infomations.
//	//! @params[in] _v     input value(one dimensional array).
//	//! @params[in] _shape input shape value.
//	//! @params[in] _ud    input super(up) or subscript(down) value.
//	_Tensor(
//		std::vector<T> _v,
//   		std::map<std::string, int> _shape,
//   		std::map<std::string, int> _ud = std::map<std::string, int>())
//        ;

//	//! @brief Constructor shape and sup. or subscript infomations.
//	//! @params[in] _shape input shape value.
//	//! @params[in] _ud    input super(up) or subscript(down) value.
//	_Tensor(
//   		std::map<std::string, int> _shape,
//   		std::map<std::string, int> _ud = std::map<std::string, int>())
//        ;

//	//! @brief Constructor with input value and indices.
//	//! @params[in] _v      input value.
//	//! @params[in] indices index values(ex. "ijk").
//	_Tensor(
//		std::vector<std::vector<T> > _v,
//   		std::string indices)
//        ;

////・Public member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//    // Manipulate of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//    //! @brief Merge nested tensor.
//    template<typename U>
//	_Tensor<U>& merge();

//    //! @brief Put  back selected axis end of 'idx'
//    //! @params[in] indices Putting back indices.
//    _Tensor<T>& putBack(std::string indices);


//    //! @brief Deep copy of _Tensor<T>.
//    virtual _Tensor<T>& clone();

//    //! @brief Gen of new _Tensor<T>.
//    virtual _Tensor<T>& gen(std::vector<T> &_v, 
//                    std::map<std::string, int> _shape, 
//                    std::map<std::string, int> _ud=std::map<std::string, int>);

//    virtual _Tensor<T>& gen(std::map<std::string, int> _shape, 
//                    std::map<std::string, int> _ud=std::map<std::string, int>);

//	virtual _Tensor<T>&	gen(std::vector<T> &_v,
//   		            std::vector<std::string> _idx,
//   		            std::map<std::string, int> _shape,
//                    std::map<std::string, int> _ud=std::map<std::string, int>);


//    //! @brief Concatenate two tensors.
//    //! @details Concatenate two tensors, 'this' and 'obj'.
//    //!          Then, need to select concatenate axis,
//    //!          'thisIdx' and 'objIdx' from 'this' and 'obj'. 
//    //!          Concatenated axis is named as 'concaIdx'.
//    _Tensor<T>& concat(_Tensor<T>& obj, 
//                       std::string thisIdx, 
//                       std::string objIdx, 
//                       std::string concaIdx, 
//                       int udVal);

//    //! @brief      change indices at super or substript to indices at sub or superscript.
//    //! @params[in] map a std::map<std::string, int> contains 
//    //!                 the pairs of eixist label and changing label.
//	_Tensor<T>& cud(std::map<std::string, int> map);

//    //! @brief      change indices label of _Tensor<T>.
//    //! @params[in] map a std::map<std::string, std::string> contains 
//    //!                 the pairs of eixist label and changing label.
//	_Tensor<T>& cidx(std::map<std::string, std::string> map);

//    //! @brief set object.
//	void setV(std::vector<T> &obj);

//    //! @brief get std::vector<T> contained data.
//	std::vector<T>& getV() const;

//    // Get infomation of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//    //! @brief generate indices of elementdata from 
//    //!        the index of when grance of data as one dimensional array. 
//	std::map<std::string, int> genIndices(int i);

//	//! @brief Get total size of _Tensor.
//	//! @retvals total size of _Tensor.
//	int size();

//    //! @brief      get reference of element of data.
//    //! @params[in] indices indices of element of data.
//	T& ref(std::map<std::string, int> indices);

//    //! @brief View infomation of tensor, shape, ud and data.
//	void view();

//    //! @brief  Whether or not has reference Tensor.
//    //! @retval bool if 'this' has reference tensor 
//    //!         then return true, else return false.
//    bool isRefTensor();

//    virtual _Tensor<T*>& getRefTensor();

//    //! @brief      broadcast tensor using this and obj.
//    //! @details    Using two tensor, 
//    //!             shape informations included this and obj,
//    //!             get a broadcasted tensor which contains zeros.
//    //! @params[in] obj    a Tensor<T>.
//    //! @params[in] trimUd select whether trim 'ud' in case of odd ud but same idx.
//    //!                    This flag used for inner product.
//	virtual _Tensor<T>& broadcast(_Tensor<T>& obj,
//								 bool trimUd=false);

//    // Math・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//    //! @brief      Calculate invert matrix for seleting two axes.
//    //! @params[in] indices Two indices of indices. Need to enter two characters.
//    _Tensor<T>& inv(std::string indices);

//    //! @brief Calculate gradient of 'obj'
//    virtual _Tensor<T>& grad ( _Tensor<T>& obj, std::map<std::string, int> indices, double delta);

////・Operators ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
//    //! @brief same Operators.
//    template<typename U>
//    _Tensor<U>& operator *(U val);

//    // On _Tensor<T>.
//	virtual _Tensor<T>& operator + (_Tensor<T> &obj);
//	virtual _Tensor<T>& operator - (_Tensor<T> &obj);
//	virtual _Tensor<T>& operator * (_Tensor<T> &obj);
//	virtual _Tensor<T>& operator / (_Tensor<T> &obj);

//    // On T.
//	virtual _Tensor<T>& operator + (T val);
//	virtual _Tensor<T>& operator - (T val);
//	virtual _Tensor<T>& operator * (T val);
//	virtual _Tensor<T>& operator / (T val);

//    // For initialize.
//    virtual _Tensor<T>& operator << (T val);
//    virtual _Tensor<T>& operator ,  (T val);

//    //
//    virtual _Tensor<T>& operator +=  (T val);
//    virtual _Tensor<T>& operator -=  (T val);
//    virtual _Tensor<T>& operator *=  (T val);
//    virtual _Tensor<T>& operator /=  (T val);
//    virtual _Tensor<T>& operator +=  (_Tensor<T> &obj);
//    virtual _Tensor<T>& operator -=  (_Tensor<T> &obj);
//    virtual _Tensor<T>& operator *=  (_Tensor<T> &obj);
//    virtual _Tensor<T>& operator /=  (_Tensor<T> &obj);

//    //! @brief      Substitution a value.
//    //! @params[in] val Input value.
//	virtual _Tensor<T>& operator = (T val);


//    //! @brief      Substitution a value.
//    //! @params[in] val Input value.
//	virtual _Tensor<T>& operator = (_Tensor<T> &obj){
//        return *new _Tensor<T>(obj);
//    }

//    //! @brief     Get trimed Tensor for _indices.
//    //! @param[in] _indices trimming indices.
//    //! @retval    trimed tensor.
//    virtual _Tensor<T>& operator [] (std::map<std::string, int> &_indices);

////・Public member variables ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
//	
//	int ndim; //!< The number of dimensions.
//	std::vector<std::string> idx,viewIdx; //!< Vector dimensons and element name(ex. i,j,k...).
//	std::map<std::string, int> shape;     //!< Shape of _Tensor.1
//	std::map<std::string, int> ud;        //!< Upper or downer script(super or sub script).
//	                                      //   Upper = 1, downer = 0.

//	std::map<std::string, int> step;      //!< Steps of each axes.
//	std::map<std::string, int> viewStep;  //!< Steps of each axes for view.

//    int trimedSize;                       //!< trimed size used by _Tensor::inv().

//    T iniVal;                             //!< initial value for reference tensor.
//	
//protected:

////・protected member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//	std::map<std::string, int> calcStep(); 

//    // For view・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
//	void putLeftBracket(int i);
//	void putValue(int i);
//	void putRightBracket(int i);

//    //! @brief  Return flag of whether a 'indices' contains index 'i'. 
//	bool isExistThisIndex(std::map<std::string, int> indices, std::string i);

////・protected member variables ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//	std::vector<T>& v;                      //!< a vector contains data.

//    std::shared_ptr<_Tensor<T*>>  refTensor;


//	int N; //!< The number of data, which refered from _Tensor<T>::size();
//};

//#include "_Tensor_detail.h"
//#include "_Tensor_pointer.h"
