#pragma once
#include "tensor.h"

template<typename T>
tensor<T>::tensor(
	std::vector<T> _v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
	v(_v),
	shape(_shape),
	ud(_ud)
{
	for (auto pair : shape)
		idx.push_back(pair.first);

	N      = v.size();
	ndim   = idx.size();
	revIdx = genRevIdx();
	step   = calcStep();
	if (!ud.size())
		for (auto pair : shape)
			ud[pair.first] = 0;
}

template<typename T>
tensor<T>::tensor(
	std::vector<std::vector<T> > _v,
	std::string indices)
	{



	idx.push_back(indices.substr(0,1));
	idx.push_back(indices.substr(1,1));
	shape[idx[0]] = _v.size();
	shape[idx[1]] = _v[0].size();
	N = 1;

	for (auto pair : shape) {
		N *= pair.second;
		ud[pair.first] = 1;
	}

	ndim   = idx.size();
	revIdx = genRevIdx();
	step   = calcStep();

	v = std::vector<T>(N);
	int cols(shape[idx[1]]);
	for (int i = 0; i < N; i++)
		v[i] = _v[i/cols][i%cols];
	
}


template<typename T>
int tensor<T>::size()
{
	return N;
}


template<typename T>
std::map<std::string, int> tensor<T>::genIndices(int i) {
	std::map<std::string, int> ret;

	for (int j = 0; j < ndim; j++) {
		ret[idx[j]] = i/step[idx[j]];
		i %= step[idx[j]];
	}


	return ret;
}

template<typename T>
T tensor<T>::ref(std::map<std::string, int> indices){
	std::map<std::string, int> ret;

	int sumIdx(0);
	for (int i = 0; i < idx.size(); i++)
		if(isExistThisIndex(indices, idx[i]))
			sumIdx += indices[idx[i]] * step[idx[i]];

	return v[sumIdx];
}


template<typename T>
void tensor<T>::view(){
	// Tensor attributes are described below
	std::cout << "-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
	std::cout << "Total size  : " << N << std::endl;
	std::cout << "Shape       : (";

	for (auto pair: shape)
		std::cout << pair.first << ": " << pair.second << ", ";
	std::cout << ")" << std::endl;


	std::cout << "Num. of Dim.: " << ndim << std::endl;
	std::cout << std::endl;

	std::cout << "-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
	std::cout << "[";
	for (int i = 0; i < N; i++) {
		putLeftBracket(i);
		putValue(i);
		putRightBracket(i);
	}
	std::cout << "]" << std::endl;
	std::cout << std::endl;
}



template<typename T>
std::map<std::string, int> tensor<T>::genRevIdx() {
	std::map<std::string, int> ret;

	for (int i = 0; i < idx.size(); i++) {
		ret[idx[i]] = i;
	}

	return ret;
}

template<typename T>
std::vector<T> getV() {
	return v;
}


template<typename T>
std::map<std::string, int> tensor<T>::calcStep(){
	int mul(1);
	std::map<std::string, int> ret;
	ret[idx[idx.size()-1]] = mul;
	if(idx.size() > 1)
		for (int i = idx.size()-2; i >= 0; i--) {
			ret[idx[i]] = mul = mul * shape[idx[i+1]];
		}
	return ret;
}

template<typename T>
bool tensor<T>::isExistThisIndex(std::map<std::string, int> indices, std::string i) {
	return indices.count(i);
}

template<typename T>
tensor<T> tensor<T>::broadcast(tensor &obj) {
	int maxDim(idx.size());
	auto newIdx   = idx;
	auto newShape = shape;
	int newSize(1);

	// making corresnpondance between index names and Dimansion names.

	for (int i=0; i<obj.idx.size(); i++)
		if(!shape.count(obj.idx[i]))
			newIdx.push_back(obj.idx[i]); // imcrement one more than maximum Dimumtsion.

	for (auto a : obj.shape)
		newShape[a.first] = a.second; // imcrement one more than maximum Dimumtsion.

	for (int i = 0; i < newIdx.size(); i++)
		newSize *= newShape[newIdx[i]];

	return tensor(
		std::vector<T>(newSize, 0),
		newShape);
}


template<typename T>
void tensor<T>::putLeftBracket(int i) {

	for (int j = 0; j < ndim-1; j++) {
		if (i % step[idx[j]]) {
			std::cout << " ";
		} //else if (step[idx[j]] == 1)
		//	std::cout << " ";
	}
	if (i) 
		std::cout << " ";

	for (int j = 0; j < ndim-1; j++) {
		if (!(i % step[idx[j]])) {
			std::cout << "[";
		}
	}

}

template<typename T>
void tensor<T>::putValue(int i) {

	std::cout 
		<< std::setw(NUM_OF_PRECISION+2)
		<< std::setprecision(NUM_OF_PRECISION) 
		<< v[i] 
		<< ", ";

	if (i != 0 && 
		(i % shape[idx[ndim-1]]) &&
		!((i % shape[idx[ndim-1]]) % NUM_OF_WRAP) &&
		((i+1) % shape[idx[ndim-1]])
	   ) {
		std::cout << std::endl;
		std::cout
			<< std::setw(NUM_OF_PRECISION-4)
			<< std::setfill(' ') << " ";
	}


}

template<typename T>
void tensor<T>::putRightBracket(int i) {

	int cnt(0);
	for (int j = ndim-2; j >= 0; j--) {
		bool tof(true);
		if (!((i + 1) % step[idx[j]])) {
			std::cout << "]";

			if (j - 1 >= 0) {
				if ((i + 1) % step[idx[j - 1]])
					std::cout << std::endl;
			}else{
				if (i != N-1)
					std::cout << std::endl;
			}
		}
	}
}

template<typename T>
tensor<T> tensor<T>::operator+(tensor &obj)
{
	tensor<T> ret = broadcast(obj);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] = ref(ret.genIndices(i)) + obj.ref(ret.genIndices(i));

	return ret;
}


template<typename T>
tensor<T> tensor<T>::operator-(tensor &obj)
{
	tensor<T> ret = broadcast(obj);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] = ref(ret.genIndices(i)) - obj.ref(ret.genIndices(i));

	return ret;
}

template<typename T>
tensor<T> tensor<T>::operator*(tensor &obj)
{
	tensor<T> ret = broadcast(obj);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] += ref(ret.genIndices(i)) * obj.ref(ret.genIndices(i));

	return ret;
}

template<typename T>
tensor<T> tensor<T>::operator/(tensor &obj)
{
	tensor<T> ret = broadcast(obj);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] = ref(ret.genIndices(i)) / obj.ref(ret.genIndices(i));

	return ret;
}


template<typename T>
tensor<T> tensor<T>::operator+(T val){
	
	tensor<T> ret(this);

	for (int i = 0; i < ret.N; i++)
		ret.v[i] += val;

	return ret;
}

template<typename T>
tensor<T> tensor<T>::operator-(T val){

	tensor<T> ret(this);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] -= val;

	return ret;
}

template<typename T>
tensor<T> tensor<T>::operator*(T val){

	tensor<T> ret(this);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] *= val;

	return ret;
}

template<typename T>
tensor<T> tensor<T>::operator/(T val){

	tensor<T> ret(*this);
	
	for (int i = 0; i < ret.N; i++)
		ret.v[i] /= val;

	return ret;
}
