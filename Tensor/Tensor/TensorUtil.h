#pragma once

#include <iostream>
#include <string>
#include <map>
#include <cstdarg>
#include <random>
#include "Tensor.h"
#include <cmath>


// Tensor util
namespace tu {
	//template <class Head, class... Tail>
	//std::map<std::string, int> s(Head head, Tail... tail);

// generating funcitons.

	//! @brief Generating and filling Tensor by zero.
	//! @param[in] shape shape of Tensor.
	template<typename T>
	Tensor<T> zeros(std::map<std::string, int> shape, std::map<std::string, int> ud=std::map<std::string, int>());

	//! @brief Generating and filling Tensor by zero.
	//! @param[in] shape shape of Tensor.
	template<typename T>
	Tensor<T> ones(std::map<std::string, int> shape, std::map<std::string, int> ud=std::map<std::string, int>());

	//! @brief Generating and filling Tensor by zero.
	//! @param[in] shape shape of Tensor.
	template<typename T>
	Tensor<T> sqrt_t(Tensor<T> &obj);

	//! @brief Calculate exponential of 'obj'.
	//! @param[in] shape shape of Tensor.
	template<typename T>
	Tensor<T> exp_t(Tensor<T> &obj);

	//! @brief Calculate exponential of 'obj'.
	//! @param[in] shape shape of Tensor.
	template<typename T>
	Tensor<T> sin_t(Tensor<T> &obj);

	//! @brief Calculate exponential of 'obj'.
	//! @param[in] shape shape of Tensor.
	template<typename T>
	Tensor<T> cos_t(Tensor<T> &obj);

    template<typename T>
    Tensor<T> Mat2Tensor();

    template<typename T>
    cv::Mat Tensor2Mat(Tensor<T> &obj, int type, std::string _indices);

	template<typename T>
	Tensor<T> eye(int _N, std::map<std::string, int> ud=std::map<std::string, int>());

	template<typename T>
	Tensor<T> normal(double mean, double sigma, std::map<std::string, int> shape, std::map<std::string, int> ud=std::map<std::string, int>());

    template<typename T>
    std::vector<T*>& range(T st, T end, T step);

    template<typename T>
    Tensor<T> rangeTensor(T st, T end, T step, std::string i, int ud);

    //! @brief 
    std::map<std::string, int> s(std::string first, ...);

    //! @brief 
    template<typename T>
    Tensor<T> t(
             std::map<std::string, int> shape, 
             std::map<std::string, int> ud, 
             ...);

    //! @brief 
    template<typename T>
    std::vector<T> l(T list[]);

	//! @brief filling Tensor by any value.
	//! @param[in] shape shape of Tensor.
	//! @param[in] val   Any value.
	template<typename T>
	Tensor<T> anyVals(std::map<std::string, int> shape, T val, std::map<std::string, int> ud=std::map<std::string, int>());
}

#include "TensorUtil_detail.h"
