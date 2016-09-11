#pragma once
#include "tensorUtil.h"

namespace tu {

int cnt(0);
std::map<std::string, int> ret_global;
std::string key;
int val(0);
std::map<std::string, int> s() {
	std::map<std::string, int> ret(ret_global);
	ret_global.clear();
	return ret;
}

template <class Head, class... Tail>
std::map<std::string, int> s(Head head, Tail... tail)
{
	bool tof(cnt++ % 2);
	if(!tof)
		key = (const char*)head;

	if(tof)
		val = (int)head;

	if (key.size() && val != 0) {
		ret_global[key] = val;
		key = "";
		val = 0;
		cnt = 0;
	}
	// パラメータパックtailをさらにheadとtailに分割する
	//print(std::move(tail)...);
	return s(tail...);
}

template<typename T>
tensor<T> zeros(std::map<std::string, int> shape) {
	int N(1);
	for (auto pair : shape)
		N *= pair.second;

	std::vector<T> v(N, 0);
	return tensor<T>(v, shape);

}

template<typename T>
tensor<T> ones(std::map<std::string, int> shape) {
	int N(1);
	for (auto pair : shape)
		N *= pair.second;

	std::vector<T> v(N, 1);
	return tensor<T>(v, shape);

}

template<typename T>
tensor<T> anyVals(std::map<std::string, int> shape, T val) {

	int N(1);
	for (auto pair : shape)
		N *= pair.second;

	std::vector<T> v(N, val);
	return tensor<T>(v, shape);

}
}
