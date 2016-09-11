#pragma once

#include <iostream>
#include <map>
#include "tensor.h"

// tensor util
namespace tu {
	template <class Head, class... Tail>
	std::map<std::string, int> s(Head head, Tail... tail);

// generating funcitons.

	//! @brief Generating and filling tensor by zero.
	//! @param[in] shape shape of tensor.
	template<typename T>
	tensor<T> zeros(std::map<std::string, int> shape);

	//! @brief Generating and filling tensor by zero.
	//! @param[in] shape shape of tensor.
	template<typename T>
	tensor<T> ones(std::map<std::string, int> shape);

	//! @brief filling tensor by any value.
	//! @param[in] shape shape of tensor.
	//! @param[in] val   Any value.
	template<typename T>
	tensor<T> anyVals(std::map<std::string, int> shape, T val);
}

#include "tensorUtil_detail.h"
