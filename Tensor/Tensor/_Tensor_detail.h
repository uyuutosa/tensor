#pragma once
#include "_Tensor.h"

template<typename T>
_Tensor<T>::_Tensor(
	std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
	std::vector<std::string> _idx,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
	v(_v),
    idx(_idx),
	shape(_shape),
	ud(_ud),
    iniCnt(0)
{
    auto tmp = shape; 
    for(auto pair: shape)
        if(pair.second == 0)
            tmp.erase(pair.first);
    shape = tmp;

    if(v->size())
        if((*v)[0].get() == nullptr)
    	    for(auto a: *v)
    		    a = std::shared_ptr<T>(new T);

    viewIdx = idx;
	N      = v->size();
	ndim   = idx.size();
	step   = calcStep();
    viewStep = step;
	if (!ud.size())
		for (auto pair : shape)
			ud[pair.first] = 0;
}

template<typename T>
_Tensor<T>::_Tensor(
	std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
	v(_v),
	shape(_shape),
	ud(_ud),
    iniCnt(0)
{

    auto tmp = shape; 
    for(auto pair: shape)
        if(pair.second == 0)
            tmp.erase(pair.first);
    shape = tmp;

    if(v->size())
        if((*v)[0].get() == nullptr)
    	    for(auto a: *v)
    		    a = std::shared_ptr<T>(new T);

	for (auto pair : shape)
		idx.push_back(pair.first);

    viewIdx = idx;

	N      = v->size();
	ndim   = idx.size();
//	revIdx = genRevIdx();
	step   = calcStep();
    viewStep = step;
	if (!ud.size())
		for (auto pair : shape)
			ud[pair.first] = 0;
}

template<typename T>
_Tensor<T>::_Tensor(
	std::map<std::string, int> _shape,
	std::map<std::string, int> _ud) :
	shape(_shape),
	ud(_ud),
    v(std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>())),
    iniCnt(0)
{
    auto tmp = shape; 
    for(auto pair: shape)
        if(pair.second == 0)
            tmp.erase(pair.first);
    shape = tmp;

    N = 1;
	for (auto pair : shape){
		idx.push_back(pair.first);
        N *= pair.second;
    }

    viewIdx = idx;
    v = std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>(N, NULL));

    if(v->size())
        for(auto& a: *v)
    	    a = std::shared_ptr<T>(new T());
    //getV() = *new std::vector<T>(N);

	ndim   = idx.size();
	step   = calcStep();
    viewStep = step;
	if (!ud.size())
		for (auto pair : shape)
			ud[pair.first] = 0;
}

template<typename T>
_Tensor<T>::_Tensor(
	std::shared_ptr<std::vector<std::vector<std::shared_ptr<T>>>> _v,
	std::string indices):
    v(_v),
    iniCnt(0)
	{

    if(v->size())
        for(auto a: *v)
    	    a = std::shared_ptr<T>(new T(0));


	idx.push_back(indices.substr(0,1));
	idx.push_back(indices.substr(1,1));
	shape[idx[0]] = _v->size();
	shape[idx[1]] = (*_v)[0]->size();
	N = 1;

	for (auto pair : shape) {
		N *= pair.second;
		ud[pair.first] = 1;
	}

	ndim   = idx.size();
//	revIdx = genRevIdx();
	step   = calcStep();

	v = std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<T*>(N, NULL));
	int cols(shape[idx[1]]);
	for (int i = 0; i < N; i++)
		(*v)[i] = (*_v)[i/cols][i%cols];
	
}

//・public member functions*・‥…─*・‥…─*・‥…─*・‥…─*
// Manipulate of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*
template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::slice(
    std::map<std::string, int> minIndices, 
    std::map<std::string, int> maxIndices){

    std::map<std::string, int> newShape;
    std::map<std::string, int> newUd;

    for(auto pair: minIndices){
        int minIdx(pair.second);
        int maxIdx(maxIndices[pair.first]);
        newShape[pair.first] = maxIdx - minIdx;
        newUd[pair.first]    = ud[pair.first];
    }

    for(auto pair: shape){
        int minIdx(pair.second);
        int maxIdx(maxIndices[pair.first]);
        if(!minIndices.count(pair.first)){
            newShape[pair.first] = shape[pair.first];
            newUd   [pair.first] = ud   [pair.first];
        }
    }

	auto ret = gen(newShape, newUd);

    bool tof(false);
    for(int i=0; i<ret->size(); i++){
        auto retIndices      = ret->genIndices(i);
        auto thisIndices     = ret->genIndices(i);

        for(auto pair: thisIndices){
            if(minIndices.count(pair.first))
                thisIndices[pair.first] += minIndices[pair.first];
        }

		*(ret->ref(retIndices)) += *ref(thisIndices); 
    }

	return ret;
}

template<typename T>
template<typename U, typename W>
std::shared_ptr<_Tensor<U>> _Tensor<T>::convertTo(){
    auto retV = std::shared_ptr<std::vector<std::shared_ptr<U>>>(new std::vector<std::shared_ptr<U>>(size()));
    auto V = getV();
    for(int i=0; i<size(); i++)
        (*retV)[i] = std::shared_ptr<U>(new U(*(*V)[i]));
        //(*retV)[i] = (std::shared_ptr<U>)(*V)[i];

    return std::shared_ptr<W>(new W(retV, shape, ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::substitute(
    std::shared_ptr<_Tensor<T>> obj, 
    std::map<std::string, int> _indices){

    auto ret = gen(v, idx, shape, ud);

    for(int i=0; i<obj->size(); i++){
        auto indices = obj->genIndices(i);
        for(auto pair: _indices){
    	    indices[pair.first] = pair.second;
    	}
        ret->ref(indices) = obj->ref(indices);
    }
    return ret;
}


template<typename T>
template<typename U>
std::shared_ptr<_Tensor<U>> _Tensor<T>::merge(){
    std::map<std::string, int> newShape, newUd;
    
    for(auto pair: shape)
        newShape[pair.first] = pair.second;

    for(auto pair: ud)
        newUd[pair.first] = pair.second;

    for(int i=0; i<v->size(); i++)
        for(auto pair: (*v)[i]->getShape())
            newShape[pair.first] = pair.second;

    for(int i=0; i<v->size(); i++)
        for(auto pair: (*v)[i]->getUd())
            newUd[pair.first] = pair.second;

    auto ret = std::shared_ptr<_Tensor<U>>(new _Tensor<U>(newShape, newUd));

    for(int i=0; i<ret->size(); i++){
        auto indices = ret->genIndices(i);
        ret->ref(indices) = ref(indices)->getT()->ref(indices);
    }
	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::putBack(std::string indices){
    auto newIdx   = idx;
    auto trimedShape = shape;


    int tSize(1);
    std::map<std::string, int> residual;
    for(int i=0; i<indices.size(); i++){
        std::string compIdx(indices.substr(i,1));
        for(int j=0; j<idx.size(); j++){
            if(newIdx[j] == compIdx){
                newIdx.erase(newIdx.begin() + j);
            	newIdx.push_back(compIdx);
                residual[compIdx] = 0;
            }
        }
    }

    for(auto pair: residual)
        trimedShape.erase(pair.first);

    for(auto pair: trimedShape)
        tSize *= pair.second;
        

    auto ret = gen(v, newIdx, shape, ud);


    ret->viewIdx  = idx;
    ret->viewStep = step;
    ret->trimedSize = tSize;

    return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::clone(){
	return std::shared_ptr<_Tensor<T>>(new auto(*this));
}

template<typename T>
std::shared_ptr<_Tensor<T>>	_Tensor<T>::gen(
                    std::shared_ptr<std::vector<std::shared_ptr<T>>> _v,
   		            std::vector<std::string> _idx,
   		            std::map<std::string, int> _shape,
                    std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>(_v, _idx, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::gen(
                std::shared_ptr<std::vector<std::shared_ptr<T>>> _v, 
                std::map<std::string, int> _shape, 
                std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>(_v, _shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::gen(
                    std::map<std::string, int> _shape, 
                    std::map<std::string, int> _ud){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>(_shape, _ud));
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::concat(
    std::shared_ptr<_Tensor<T>> obj, 
    std::string thisIdx, 
    std::string objIdx, 
    std::string concaIdx, 
    int udVal){

    std::map<std::string, int> newShape;
    std::map<std::string, int> newUd;

    for(auto pair: shape)
        newShape[pair.first] = pair.second;

    for(auto pair: obj->shape)
        newShape[pair.first] = pair.second;

    for(auto pair: ud)
        newUd[pair.first] = pair.second;

    for(auto pair: obj->ud)
        newUd[pair.first] = pair.second;

    newShape[concaIdx] = shape[thisIdx] + obj->shape[objIdx];
    newUd   [concaIdx] = udVal;
    newShape.erase(thisIdx);
    newShape.erase(objIdx);
    newUd.erase(thisIdx);
    newUd.erase(objIdx);

    auto ret = gen(newShape, newUd);

    int sepNum(shape[thisIdx]);
    for (int i=0; i<ret->size(); i++){
        auto indices     = ret->genIndices(i);
        auto thisIndices = indices;
        auto objIndices  = indices;

        if (indices[concaIdx] >= sepNum){
            objIndices[objIdx] = indices[concaIdx] - sepNum;
            //objIndices.erase(objIdx);
        	ret->ref(indices)   = obj->ref(objIndices);
        } else {
            thisIndices[thisIdx] = indices[concaIdx];
            //thisIndices.erase(thisIdx);
            ret->ref(indices) = ref(thisIndices);
        }
    }
    return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::cud(std::map<std::string, int> map){
    auto newUd    = ud;

    for(auto pair: map)
        newUd[pair.first] = pair.second;

    return gen(v, shape, newUd);
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::cidx(std::map<std::string, std::string> map){
    auto newIdx    = idx;
    auto newShape  = shape;
    auto newUd     = ud;

    for(auto pair: map){
        for(int i=0; i<newIdx.size(); i++)
            newIdx[i] = newIdx[i] == pair.first ? pair.second : newIdx[i];

        if(newShape.count(pair.first)){
            newShape[pair.second] = newShape[pair.first];
            newShape.erase(pair.first);
            newUd[pair.second] = newUd[pair.first];
            newUd.erase(pair.first);
        }
    }

    return gen(v, newIdx, newShape, newUd);
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::reshape(std::map<std::string, int> _shape, std::map<std::string, int> _ud){
    auto ret = gen(_shape, _ud);
    auto thisCpy = clone();

    std::map<std::string, int> newShape;
    std::map<std::string, int> newUd;

    std::string indices;

    for(auto pair: shape){
        if(_shape.count(pair.first)){
            indices.push_back(*const_cast<char *>(pair.first.c_str()));
            //newUd[pair.first]    = newUd[pair.first];
        }
    }

    ret = ret->putBack(indices);
    thisCpy = thisCpy->putBack(indices);

    for(int i=0; i<N; i++){
        ret->ref(ret->genIndices(i)) = thisCpy->ref(thisCpy->genIndices(i));
    }

    return ret;
}

template<typename T>
void _Tensor<T>::setV(std::shared_ptr<std::vector<std::shared_ptr<T>>> obj) {
    v = obj;
}

template<typename T>
std::shared_ptr<std::vector<std::shared_ptr<T>>> _Tensor<T>::getV() const {
	return v;
}

// Get infomation of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

template<typename T>
std::map<std::string, int> _Tensor<T>::genIndices(int i) {
	std::map<std::string, int> ret;

	for (int j = 0; j < ndim; j++) {
		ret[idx[j]] = i / step[idx[j]];
		i %= step[idx[j]];
	}

	return ret;
}

template<typename T>
int _Tensor<T>::size()
{
	return N;
}

template<typename T>
std::shared_ptr<T>& _Tensor<T>::ref(std::map<std::string, int> indices){
	std::map<std::string, int> ret;

    int sumIdx(0);
	for (int i = 0; i < idx.size(); i++)
		if(isExistThisIndex(indices, idx[i]))
			sumIdx += indices[viewIdx[i]] * viewStep[viewIdx[i]];

	return (*v)[sumIdx];
}

template<typename T>
void _Tensor<T>::view(std::ostream& os){
	// _Tensor attributes are described below
	os << "-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
	os << "Total size  : " << N << std::endl;
	os << "Shape       : (";

	for (auto i: idx)
		os << i << ": " << shape[i] << ", ";
	os << ")" << std::endl;

	os << "Num. of Dim.: " << ndim << std::endl;
	os << std::endl;

	os << "Up or down  : (";

	std::vector<std::string> ud_lut;
	ud_lut.push_back("down");
	ud_lut.push_back("up");
	for (auto pair: ud)
		os << ud_lut[pair.second] 
			      << ", ";
	os << ")" << std::endl;

	os << "-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
	os << "[";
	for (int i = 0; i < N; i++) {
		putLeftBracket(i,os);
		putValue(i,os);
		putRightBracket(i,os);
	}
	os << "]" << std::endl;
	os << std::endl;
}


template<typename T>
bool _Tensor<T>::isRefTensor() {
	return false;
}

template<typename T>
bool _Tensor<T>::isFunctionTensor(){
    return false;
}
template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::getRefTensor() {
	return refTensor;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::broadcast(std::shared_ptr<_Tensor<T>> obj,
								 bool trimUd){
	int  maxDim(idx.size());
	auto newIdx = idx;
	auto newShape = shape;
	auto newUd = ud;
	int  newSize(1);

	// making corresnpondance between index names and Dimansion names.
	std::string dupUd("");
	if (trimUd){
		for (auto a : ud) {
			for (auto b : obj->ud) {
				if (a.first == b.first &&
					a.second != b.second)
					dupUd.push_back(*a.first.c_str());
			}
		}
	}

	for (auto a : ud){ 
		newUd[a.first] = a.second;
		for (auto b : obj->ud) 
			newUd[b.first] = b.second;
    }

	for (int i=0; i<obj->idx.size(); i++)
		if(!shape.count(obj->idx[i]) && 
			dupUd.find(obj->idx[i]) == std::string::npos)
			newIdx.push_back(obj->idx[i]); // imcrement one more than maximum Dimumtsion.

	for (auto a : obj->shape) {
		newShape[a.first] = a.second; // imcrement one more than maximum Dimumtsion.
	}

	for (auto a : dupUd) {
		std::string str(1, a);
		for(int i=0; i<newIdx.size(); i++)
			if (newIdx[i] == str)
				newIdx.erase(newIdx.begin() + i);
		newShape.erase(str);
		newUd.erase(str);
	}

	for (int i = 0; i < newIdx.size(); i++)
		newSize *= newShape[newIdx[i]];

	return gen(
//		*new std::vector<T*>(newSize),
		newShape,
		newUd);
}

// Math・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::inv(std::string indices){
	std::shared_ptr<_Tensor<T>> ret =  gen(
		std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>(*v)),
		shape,
		ud);

    ret = ret->putBack(indices);

    int jSize, kSize;
    int tSize(ret->trimedSize); 

    if(shape.count(indices.substr(0,1)))
        jSize = shape[indices.substr(0,1)];
    if(shape.count(indices.substr(1,1)))
    	kSize = shape[indices.substr(1,1)];

    if(!tSize)
        tSize = 1;

	for (int i = 0; i < tSize; i++){
        cv::Mat m(jSize, kSize, CV_64F);
	    for (int j = 0; j < jSize; j++){
            T* p = m.ptr<T>(j);
	        for (int k = 0; k < kSize; k++){
                p[k] = (double)(*ret->ref(ret->genIndices(i*jSize * kSize + j*kSize + k))); 
                //m.at<double>(j, k) = (double)ret.ref(ret.genIndices(i*jSize * kSize + j*kSize + k)); 
            }
        }
        m = m.inv();
	    for (int j = 0; j < jSize; j++){
	        for (int k = 0; k < kSize; k++)
                *ret->ref(ret->genIndices(i*jSize * kSize + j*kSize + k)) =  m.at<T>(j, k); 
        }
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::sum(std::string indices){
    std::map<std::string, int> newShape;
    std::map<std::string, int> newUd;

    for(int i=0; i<indices.size(); i++){
        std::string s = indices.substr(i, 1);
        newShape[s] = shape[s];
        newUd[s] = ud[s] == 0 ? 1 : 0;
    } 

    auto o = tu::ones<T>(newShape, newUd).getT();
    auto ret = *(this->convertTo<T, ConcreteTensor<T>>()) * o;

    if(!ret->shape.size()){
        newShape = std::map<std::string, int>();
        newUd = std::map<std::string, int>();
        newShape["null"] = 1;
        newUd["null"]    = 0;
        auto ret = std::shared_ptr<T>(new T(0));
        for(auto a: *v)
            *ret += *a;
        auto retV = std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>(1, ret));
        return gen(retV, newShape, newUd);
    } else 
        return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::mean(std::string indices){
    std::map<std::string, int> newShape = shape;
    std::map<std::string, int> newUd = ud;

    for(int i=0; i<indices.size(); i++){
        std::string s = indices.substr(i, 1);
        if(newShape.count(s)){
            newShape.erase(s);
            newUd.erase(s);
        }
    } 

    if(newShape.size()){
        newShape["null"] = N;
        newUd["null"]    = 0;
    }

    auto o = tu::ones<T>(newShape, newUd).getT();
    auto ret = *this * o / n;
    return ret;    
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::min(std::string indices){
    std::map<std::string, int> newShape = shape;
    std::map<std::string, int> newUd = ud;

    for(int i=0; i<indices.size(); i++){
        std::string s = indices.substr(i, 1);
        if(newShape.count(s)){
            newShape.erase(s);
            newUd.erase(s);
        }
    } 

    if(newShape.size()){
        newShape["null"] = N;
        newUd["null"]    = 0;
    }

    auto ret = gen(newShape, newUd);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 9999999999; 

	for (int i = 0; i < N; i++){
		*ret->ref(genIndices(i)) = *ret->ref(genIndices(i)) > *ref(genIndices(i)) 
                                    ? *ref(genIndices(i))
                                    : *ret->ref(genIndices(i)); 
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::max(std::string indices){
    std::map<std::string, int> newShape = shape;
    std::map<std::string, int> newUd = ud;

    for(int i=0; i<indices.size(); i++){
        std::string s = indices.substr(i, 1);
        if(newShape.count(s)){
            newShape.erase(s);
            newUd.erase(s);
        }
    } 

    if(!newShape.size()){
        newShape["null"] = 1;
        newUd["null"]    = 0;
        auto ret = gen(newShape, newUd);
        auto maxVal = std::shared_ptr<T>(new T(0));
        for(auto a: *v)
            *maxVal = *maxVal < *a ? *a : *maxVal;
        return gen(std::shared_ptr<std::vector<std::shared_ptr<T>>>(new std::vector<std::shared_ptr<T>>(1, maxVal)), newShape, newUd);
    } else {

        auto ret = gen(newShape, newUd);

    	for(auto a: *ret->getV())
    	    *a = *(*v)[0] * 0; 

		for (int i = 0; i < N; i++){
			*ret->ref(genIndices(i)) = *ret->ref(genIndices(i)) < *ref(genIndices(i)) 
    	                                ? *ref(genIndices(i))
    	                                : *ret->ref(genIndices(i)); 
    	}
		return ret;
    }
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::sign(){
    auto ret = gen(shape, ud);    

    auto retV = ret->getV();

    for(int i=0; i<N; i++)
        if(*(*v)[i] > 0)
           *(*retV)[i] = 1;
        else if(*(*v)[i] < 0)
           *(*retV)[i] = -1;
        else 
           *(*retV)[i] = 0;


    return ret;    
}


template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::norm(std::string indices){
    std::map<std::string, int> newShape;
    std::map<std::string, int> newUd   ;

    for(int i=0; i<indices.size(); i++){
        std::string s = indices.substr(i,1);
        newShape[s] = shape[s];
        newUd[s]    = ud[s] == 0 ? 1 : 0;
    }

    auto o = tu::ones<double>(newShape, newUd).getT();
    auto o2 = ((*this) * (*this)) * o;
    auto  ret_t = tu::sqrt_t<T>(o2);
    return ret_t.getT();    
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::grad(std::shared_ptr<_Tensor<T>> obj, std::map<std::string, int> indices, double delta){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>);
}

template<typename T>
void _Tensor<T>::substituteFunc(void* _f, std::map<std::string, int> indices){
}

template<typename T>
void _Tensor<T>::genReferenceTensor(){
}

//・Operators ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

template<typename T>
template<typename U>
std::shared_ptr<_Tensor<U>> _Tensor<T>::operator *(U val) {
	return std::shared_ptr<_Tensor<U>>(new _Tensor<U>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator+(std::shared_ptr<_Tensor<T>> obj)
{
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator-(std::shared_ptr<_Tensor<T>> obj)
{
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator*(std::shared_ptr<_Tensor<T>> obj)
{
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator/(std::shared_ptr<_Tensor<T>> obj)
{
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

//template<typename T>
//std::shared_ptr<_Tensor<T>> _Tensor<T>::operator^(std::shared_ptr<_Tensor<T>> obj)
//{
//	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
//}


template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator+(T val){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator-(T val){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator*(T val){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator/(T val){
	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
}

//template<typename T>
//std::shared_ptr<_Tensor<T>> _Tensor<T>::operator^(T val){
//	return std::shared_ptr<_Tensor<T>>(new _Tensor<T>());
//}


template<typename T>
void _Tensor<T>::operator <<(T val){
    *(*v)[iniCnt++] = val;
}

template<typename T>
void _Tensor<T>::operator ,(T val){
    *(*v)[iniCnt++] = val;
}

template<typename T>
void _Tensor<T>::operator+=(std::shared_ptr<_Tensor<T>> obj)
{
}


template<typename T>
void _Tensor<T>::operator-=(std::shared_ptr<_Tensor<T>> obj)
{
}

template<typename T>
void _Tensor<T>::operator*=(std::shared_ptr<_Tensor<T>> obj)
{
}

template<typename T>
void _Tensor<T>::operator/=(std::shared_ptr<_Tensor<T>> obj)
{
}

//template<typename T>
//void _Tensor<T>::operator^=(std::shared_ptr<_Tensor<T>> obj)
//{
//}

template<typename T>
void _Tensor<T>::operator+=(T val)
{
}


template<typename T>
void _Tensor<T>::operator-=(T val)
{
}

template<typename T>
void _Tensor<T>::operator*=(T val)
{
}

template<typename T>
void _Tensor<T>::operator/=(T val)
{
}

template<typename T>
void _Tensor<T>::operator =(T val)
{
    for (int i=0; i<N; i++){
        *(*v)[i] = val;
    }
}

template<typename T>
void _Tensor<T>::operator ^=(T val)
{
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator == (T val)
{
	auto ret = std::shared_ptr<_Tensor<bool>>(new _Tensor<bool>(shape, ud));

    auto retV = ret->getV();
    auto v    = getV();

	for (int i = 0; i < retV->size(); i++){
        (*retV)[i] = std::shared_ptr<bool>(new bool(*(*v)[i] == val));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator>(T val)
{
	auto ret = std::shared_ptr<_Tensor<bool>>(new _Tensor<bool>(shape, ud));

    auto retV = ret->getV();
    auto v    = getV();

	for (int i = 0; i < retV->size(); i++){
        (*retV)[i] = std::shared_ptr<bool>(new bool(*(*v)[i] > val));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator < (T val)
{
	auto ret = std::shared_ptr<_Tensor<bool>>(new _Tensor<bool>(shape, ud));

    auto retV = ret->getV();
    auto v    = getV();

	for (int i = 0; i < retV->size(); i++){
        (*retV)[i] = std::shared_ptr<bool>(new bool(*(*v)[i] < val));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator >= (T val)
{
	auto ret = std::shared_ptr<_Tensor<bool>>(new _Tensor<bool>(shape, ud));

    auto retV = ret->getV();
    auto v    = getV();

	for (int i = 0; i < retV->size(); i++){
        (*retV)[i] = std::shared_ptr<bool>(new bool(*(*v)[i] >= val));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator <= (T val)
{
	auto ret = std::shared_ptr<_Tensor<bool>>(new _Tensor<bool>(shape, ud));

    auto retV = ret->getV();
    auto v    = getV();

	for (int i = 0; i < retV.size(); i++){
        (*retV)[i] = std::shared_ptr<bool>(new bool(*(*v)[i] <= val));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator == (std::shared_ptr<_Tensor<T>> obj)
{
	auto ret = broadcast(obj, true)->convertTo<bool, ConcreteTensor<bool> >();
	//auto ret      = broadcast(obj, true).convertTo<bool, ConcreteTensor<bool> >();
	auto noTrimed = broadcast(obj);

    std::map<std::string, int> indices;
	for (int i = 0; i < noTrimed->size(); i++){
        indices = noTrimed->genIndices(i);
        ret->ref(indices) = std::shared_ptr<bool>(new bool(*ref(indices) == *obj->ref(indices)));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator > (std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true).convertTo<bool, ConcreteTensor<bool> >();
	auto noTrimed = broadcast(obj);

    std::map<std::string, int> indices;
	for (int i = 0; i < noTrimed->size(); i++){
        indices = noTrimed->genIndices(i);
        ret->ref(indices) = std::shared_ptr<bool>(new bool(*ref(indices) > *obj->ref(indices)));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator < (std::shared_ptr<_Tensor<T>> obj)
{
	auto ret      = broadcast(obj, true).convertTo<bool, ConcreteTensor<bool> >();
	auto noTrimed = broadcast(obj);
	auto noTrimed = broadcast(obj);

    std::map<std::string, int> indices;
	for (int i = 0; i < noTrimed->size(); i++){
        indices = noTrimed->genIndices(i);
        ret->ref(indices) = std::shard_ptr<bool>(new bool(*ref(indices) < *obj->ref(indices)));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator >= (std::shared_ptr<_Tensor<T>> obj)
{
	auto ret   = broadcast(obj, true).convertTo<bool, ConcreteTensor<bool> >();
	auto noTrimed = broadcast(obj);

    std::map<std::string, int> indices;
	for (int i = 0; i < noTrimed->size(); i++){
        indices = noTrimed->genIndices(i);
        ret->ref(indices) = std::shard_ptr<bool>(new bool(*ref(indices) >= *obj->ref(indices)));
    }

	return ret;
}

template<typename T>
std::shared_ptr<_Tensor<bool>> _Tensor<T>::operator <= (std::shared_ptr<_Tensor<T>> obj)
{
	auto ret   = broadcast(obj, true).convertTo<bool, ConcreteTensor<bool> >();
	auto noTrimed = broadcast(obj);
	auto noTrimed = broadcast(obj);

    std::map<std::string, int> indices;
	for (int i = 0; i < noTrimed->size(); i++){
        indices = noTrimed->genIndices(i);
        ret->ref(indices) = std::shared_ptr<bool>(new bool(*ref(indices) <= *obj->ref(indices)));
    }

	return ret;
}


//template<typename T>
//template<typename U>
//bool _Tensor<T>::operator<(U val)
//{
//    return false;
//}

//template<typename T>
//template<typename U>
//bool _Tensor<T>::operator>(U val)
//{
//    return false;
//}

template<typename T>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator [] (std::map<std::string, int> &_indices){
    std::map<std::string, int> shape2(shape);
    std::map<std::string, int> ud2(ud);

    for(auto pair: shape){
        if(_indices.count(pair.first)){
            shape2.erase(pair.first);
            ud2.erase(pair.first);
        }
    }

	auto ret = gen(shape2, ud2);

	for (int i = 0; i < ret->size(); i++){
        auto indices = ret->genIndices(i);
        auto thisIndices = indices;
        for(auto pair: _indices){
            thisIndices[pair.first] = pair.second;
        }
		ret->ref(indices) = ref(thisIndices);
    }

	return ret;
}

template<typename T>
template<typename U>
std::shared_ptr<_Tensor<T>> _Tensor<T>::operator [] (std::shared_ptr<_Tensor<U>> obj){
    auto ret = gen(obj->shape, obj->ud);
    auto V = getV();
    auto objV = obj->getV();
    auto retV = ret->getV();

    int index(0);
    for(int i=0; i < obj->size(); i++){
        index = *(*objV)[i];
        if( index < N)
            (*retV)[i] = std::shared_ptr<T>(new T(*(*V)[index]));
    }
	return ret;
}

template<typename T>
template<typename U>
std::shared_ptr<_Tensor<T>> _Tensor<T>::suffix (std::shared_ptr<_Tensor<U>> obj, std::string sfx){

    std::shared_ptr<_Tensor<U>> obj2 = obj->clone();
    std::string objSuffix;

    for (auto pair: obj2->shape)
        objSuffix = pair.first;

    std::map<std::string, std::string> indices;
    indices[objSuffix] = sfx;
    obj2 = obj2->cidx(indices);
        
    auto newShape = shape;
    auto newUd    = ud;
    if(newShape.count(sfx)){
        newShape[sfx] = obj2->shape[sfx];
        newUd[sfx] = obj2->ud[sfx];
    }
    
	auto ret = gen(newShape, newUd);
	//_Tensor<T>& ret = * new _Tensor<double>(newShape, newUd);

    for(auto a: *ret->getV())
        *a = *(*v)[0] * 0; 

    int tmp = ret->size();
	for (int i = 0; i < tmp; i++){
        auto map = ret->genIndices(i);
        for(auto pair: obj2->genIndices(i))
            if(ret->shape.count(pair.first) != 0){
                auto aaa = ret->genIndices(i);
                auto aa = *obj2->ref(aaa);
                map[pair.first] = aa;
            }
		ret->ref(ret->genIndices(i)) = ref(map);
    }

	return ret;
}

//・protected member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

template<typename T>
std::map<std::string, int> _Tensor<T>::calcStep(){
	int mul(1);
	std::map<std::string, int> ret;
	if (idx.size()) {
		ret[idx[idx.size() - 1]] = mul;
		if (idx.size() > 1)
			for (int i = idx.size() - 2; i >= 0; i--) {
				ret[idx[i]] = mul = mul * shape[idx[i + 1]];
			}
	}
	return ret;
}

template<typename T>
void _Tensor<T>::putLeftBracket(int i, std::ostream &os) {

	for (int j = 0; j < ndim-1; j++) {
		if (i % step[idx[j]]) {
			os << " ";
		} 
	}
	if (i) 
		os << " ";

	for (int j = 0; j < ndim-1; j++) {
		if (!(i % step[idx[j]])) {
			os << "[";
		}
	}

}


template<typename T>
void _Tensor<T>::putValue(int i, std::ostream &os) {

	os 
		<< std::setw(NUM_OF_PRECISION+2)
		<< std::setprecision(NUM_OF_PRECISION) 
		<< (T)*ref(genIndices(i)) 
		<< ", ";

	if (i != 0 && 
		(i % shape[idx[ndim-1]]) &&
		!((i % shape[idx[ndim-1]]) % NUM_OF_WRAP) &&
		((i+1) % shape[idx[ndim-1]])
	   ) {
		os << std::endl;
	    os	
			<< std::setw(NUM_OF_PRECISION-4)
			<< std::setfill(' ') << " ";
	}
    

}

template<typename T>
void _Tensor<T>::putRightBracket(int i, std::ostream &os) {

	int cnt(0);
	for (int j = ndim-2; j >= 0; j--) {
		bool tof(true);
		if (!((i + 1) % step[idx[j]])) {
			os << "]";

			if (j - 1 >= 0) {
				if ((i + 1) % step[idx[j - 1]])
					os << std::endl;
			}else{
				if (i != N-1)
					os << std::endl;
			}
		}
	}
}

template<typename T>
bool _Tensor<T>::isExistThisIndex(std::map<std::string, int> indices, std::string i) {
	return indices.count(i);
}
