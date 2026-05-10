//
//#pragma once
//#include "_Tensor_pointer.h"
////#pragma optimize ("", off)
//template<typename T>
//_Tensor<T*>::_Tensor(
//	std::vector<T*> _v,
//	std::vector<std::string> _idx,
//	std::map<std::string, int> _shape,
//	std::map<std::string, int> _ud) :
//	v(_v),
//    idx(_idx),
//	shape(_shape),
//	ud(_ud)
//{

//    viewIdx = idx;
//	N      = v.size();
//	ndim   = idx.size();
//	step   = calcStep();
//    viewStep = step;
//	if (!ud.size())
//		for (auto pair : shape)
//			ud[pair.first] = 0;
//}

//template<typename T>
//_Tensor<T*>::_Tensor(
//	std::vector<T> _v,
//	std::map<std::string, int> _shape,
//	std::map<std::string, int> _ud) :
//	v(_v),
//	shape(_shape),
//	ud(_ud)
//{
//	for (auto pair : shape)
//		idx.push_back(pair.first);

//    viewIdx = idx;

//	N      = v.size();
//	ndim   = idx.size();
////	revIdx = genRevIdx();
//	step   = calcStep();
//    viewStep = step;
//	if (!ud.size())
//		for (auto pair : shape)
//			ud[pair.first] = 0;
//}

//template<typename T>
//_Tensor<T*>::_Tensor(
//	std::map<std::string, int> _shape,
//	std::map<std::string, int> _ud) :
//	shape(_shape),
//	ud(_ud),
//    v(std::vector<T*>())
//{
//    N = 1;
//	for (auto pair : shape){
//		idx.push_back(pair.first);
//        N *= pair.second;
//    }

//    viewIdx = idx;
//    getV() = *new std::vector<T>(N);

//	ndim   = idx.size();
//	step   = calcStep();
//    viewStep = step;
//	if (!ud.size())
//		for (auto pair : shape)
//			ud[pair.first] = 0;
//}

//template<typename T>
//_Tensor<T*>::_Tensor(
//	std::vector<std::vector<T*> > _v,
//	std::string indices):
//    v(_v)
//	{



//	idx.push_back(indices.substr(0,1));
//	idx.push_back(indices.substr(1,1));
//	shape[idx[0]] = _v.size();
//	shape[idx[1]] = _v[0].size();
//	N = 1;

//	for (auto pair : shape) {
//		N *= pair.second;
//		ud[pair.first] = 1;
//	}

//	ndim   = idx.size();
////	revIdx = genRevIdx();
//	step   = calcStep();

//	v = * new std::vector<T>(N);
//	int cols(shape[idx[1]]);
//	for (int i = 0; i < N; i++)
//		v[i] = _v[i/cols][i%cols];
//	
//}

////・Public member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//template<typename T>
//template<typename U>
//_Tensor<U>& _Tensor<T*>::merge(){
//    _Tensor<U>& ret = *new _Tensor<U>(shape, ud);
//    ret = ret.broadcast(getV()[0].getT());
//    for(int i=0; i<ret.size(); i++){
//        auto indices = ret.genIndices(i);
//        ret.ref(indices) = ref(indices).getT().ref(indices);
//    }
//	return ret;
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::putBack(std::string indices){
//    auto newIdx   = idx;
//    auto trimedShape = shape;


//    int tSize(1);
//    std::map<std::string, int> residual;
//    for(int i=0; i<indices.size(); i++){
//        std::string compIdx(indices.substr(i,1));
//        for(int j=0; j<idx.size(); j++){
//            if(newIdx[j] == compIdx){
//                newIdx.erase(newIdx.begin() + j);
//            	newIdx.push_back(compIdx);
//                residual[compIdx] = 0;
//            }
//        }
//    }

//    for(auto pair: residual)
//        trimedShape.erase(pair.first);

//    for(auto pair: trimedShape)
//        tSize *= pair.second;
//        

//    _Tensor<T>& ret = gen(v, newIdx, shape, ud);


//    ret.viewIdx  = idx;
//    ret.viewStep = step;
//    ret.trimedSize = tSize;

//    return ret;
//}

////template<typename T>
////_Tensor<T>& _Tensor<T>::clone(){
////	return *new _Tensor<T>(v, shape, ud);
////}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::clone(){
//	return *new auto(*this);
//}

//template<typename T>
//_Tensor<T*>&	_Tensor<T*>::gen(
//                    std::vector<T*> &_v,
//   		            std::vector<std::string> _idx,
//   		            std::map<std::string, int> _shape,
//                    std::map<std::string, int> _ud){
//	return *new _Tensor<T>(_v, _idx, _shape, _ud);
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::gen(
//                std::vector<T*> &_v, 
//                std::map<std::string, int> _shape, 
//                std::map<std::string, int> _ud){
//	return *new _Tensor<T>(_v, _shape, _ud);
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::gen(
//                    std::map<std::string, int> _shape, 
//                    std::map<std::string, int> _ud){
//	return *new _Tensor<T>(_shape, _ud);
//}


//template<typename T>
//_Tensor<T*>& _Tensor<T*>::concat(
//    _Tensor<T*>& obj, 
//    std::string thisIdx, 
//    std::string objIdx, 
//    std::string concaIdx, 
//    int udVal){

//    std::map<std::string, int> newShape;
//    std::map<std::string, int> newUd;

//    for(auto pair: shape)
//        newShape[pair.first] = pair.second;

//    for(auto pair: obj.shape)
//        newShape[pair.first] = pair.second;

//    for(auto pair: ud)
//        newUd[pair.first] = pair.second;

//    for(auto pair: obj.ud)
//        newUd[pair.first] = pair.second;

//    newShape[concaIdx] = shape[thisIdx] + obj.shape[objIdx];
//    newUd   [concaIdx] = udVal;
//    newShape.erase(thisIdx);
//    newShape.erase(objIdx);
//    newUd.erase(thisIdx);
//    newUd.erase(objIdx);

//    _Tensor<T*>& ret = gen(newShape, newUd);

//    int sepNum(shape[thisIdx]);
//    for (int i=0; i<ret.size(); i++){
//        auto indices     = ret.genIndices(i);
//        auto thisIndices = indices;
//        auto objIndices  = indices;

//        if (indices[concaIdx] >= sepNum){
//            objIndices[objIdx] = indices[concaIdx] - sepNum;
//            //objIndices.erase(objIdx);
//        	ret.ref(indices)   = obj.ref(objIndices);
//        } else {
//            thisIndices[thisIdx] = indices[concaIdx];
//            //thisIndices.erase(thisIdx);
//            ret.ref(indices) = ref(thisIndices);
//        }
//    }
//    return ret;
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::cud(std::map<std::string, int> map){
//    auto newUd    = ud;

//    for(auto pair: map)
//        newUd[pair.first] = pair.second;

//    return gen(v, shape, newUd);
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::cidx(std::map<std::string, std::string> map){
//    auto newIdx    = idx;
//    auto newShape  = shape;
//    auto newUd     = ud;

//    for(auto pair: map){
//        for(int i=0; i<newIdx.size(); i++)
//            newIdx[i] = newIdx[i] == pair.first ? pair.second : newIdx[i];

//        if(newShape.count(pair.first)){
//            newShape[pair.second] = newShape[pair.first];
//            newShape.erase(pair.first);
//            newUd[pair.second] = newUd[pair.first];
//            newUd.erase(pair.first);
//        }
//    }

//    return gen(v, newIdx, newShape, newUd);
//}

//template<typename T>
//void _Tensor<T*>::setV(std::vector<T*> &obj) {
//    v = obj;
//}

//template<typename T>
//std::vector<T*>& _Tensor<T*>::getV() const {
//	return v;
//}

//// Get infomation of tensor・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//template<typename T>
//std::map<std::string, int> _Tensor<T*>::genIndices(int i) {
//	std::map<std::string, int> ret;

//	for (int j = 0; j < ndim; j++) {
//		ret[idx[j]] = i/step[idx[j]];
//		i %= step[idx[j]];
//	}


//	return ret;
//}

//template<typename T>
//int _Tensor<T>::size()
//{
//	return N;
//}

//template<typename T>
//T& _Tensor<T*>::ref(std::map<std::string, int> indices){
//	std::map<std::string, int> ret;

//	int sumIdx(0);
//	for (int i = 0; i < idx.size(); i++)
//		if(isExistThisIndex(indices, idx[i]))
//			sumIdx += indices[viewIdx[i]] * viewStep[viewIdx[i]];

//	return *v[sumIdx];
//}

//template<typename T>
//void _Tensor<T*>::view(){
//	// _Tensor attributes are described below
//	std::cout << "-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
//	std::cout << "Total size  : " << N << std::endl;
//	std::cout << "Shape       : (";

//	for (auto i: idx)
//		std::cout << i << ": " << shape[i] << ", ";
//	std::cout << ")" << std::endl;

//	std::cout << "Num. of Dim.: " << ndim << std::endl;
//	std::cout << std::endl;

//	std::cout << "Up or donw  : (";

//	std::vector<std::string> ud_lut;
//	ud_lut.push_back("down");
//	ud_lut.push_back("up");
//	for (auto pair: ud)
//		std::cout << ud_lut[pair.second] 
//			      << ", ";
//	std::cout << ")" << std::endl;

//	std::cout << "-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" << std::endl;
//	std::cout << "[";
//	for (int i = 0; i < N; i++) {
//		putLeftBracket(i);
//		putValue(i);
//		putRightBracket(i);
//	}
//	std::cout << "]" << std::endl;
//	std::cout << std::endl;
//}


//template<typename T>
//bool _Tensor<T>::isRefTensor() {
//	return false;
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::getRefTensor() {
//	return *refTensor;
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::broadcast(_Tensor<T>& obj,
//								 bool trimUd){
//	int  maxDim(idx.size());
//	auto newIdx = idx;
//	auto newShape = shape;
//	auto newUd = ud;
//	int  newSize(1);

//	// making corresnpondance between index names and Dimansion names.
//	std::string dupUd("");
//	if (trimUd){
//		for (auto a : ud) {
//			for (auto b : obj.ud) {
//				if (a.first == b.first &&
//					a.second != b.second)
//					dupUd.push_back(*a.first.c_str());
//			}
//		}
//	}

//	for (auto a : ud) 
//		newUd[a.first] = a.second;
//		for (auto b : obj.ud) 
//			newUd[b.first] = b.second;

//	for (int i=0; i<obj.idx.size(); i++)
//		if(!shape.count(obj.idx[i]) && 
//			dupUd.find(obj.idx[i]) == std::string::npos)
//			newIdx.push_back(obj.idx[i]); // imcrement one more than maximum Dimumtsion.

//	for (auto a : obj.shape) {
//		newShape[a.first] = a.second; // imcrement one more than maximum Dimumtsion.
//	}

//	for (auto a : dupUd) {
//		std::string str(1, a);
//		for(int i=0; i<newIdx.size(); i++)
//			if (newIdx[i] == str)
//				newIdx.erase(newIdx.begin() + i);
//		newShape.erase(str);
//		newUd.erase(str);
//	}

//	for (int i = 0; i < newIdx.size(); i++)
//		newSize *= newShape[newIdx[i]];

//	return gen(
//		std::vector<T>(newSize),
//		newShape,
//		newUd);
//}

//// Math・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::inv(std::string indices){
//	_Tensor<T*>& ret =  gen(
//		std::vector<T>(v),
//		shape,
//		ud);

//    ret = ret.putBack(indices);

//    int jSize, kSize;
//    int tSize(ret.trimedSize); 

//    if(shape.count(indices.substr(0,1)))
//        jSize = shape[indices.substr(0,1)];
//    if(shape.count(indices.substr(1,1)))
//    	kSize = shape[indices.substr(1,1)];

//    if(!tSize)
//        tSize = 1;

//	for (int i = 0; i < tSize; i++){
//        cv::Mat m(jSize, kSize, CV_64F);
//	    for (int j = 0; j < jSize; j++){
//            T* p = m.ptr<T>(j);
//	        for (int k = 0; k < kSize; k++){
//                p[k] = (double)ret.ref(ret.genIndices(i*jSize * kSize + j*kSize + k)); 
//                //m.at<double>(j, k) = (double)ret.ref(ret.genIndices(i*jSize * kSize + j*kSize + k)); 
//            }
//        }
//        m = m.inv();
//	    for (int j = 0; j < jSize; j++){
//	        for (int k = 0; k < kSize; k++)
//                ret.ref(ret.genIndices(i*jSize * kSize + j*kSize + k)) =  m.at<T>(j, k); 
//        }
//    }

//	return ret;
//}

//template<typename T>
//_Tensor<T*>& _Tensor<T*>::grad(_Tensor<T>& obj, std::map<std::string, int> indices, double delta){
//	return *new _Tensor<T*>;
//}

////・Operators ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//template<typename T>
//template<typename U>
//_Tensor<U>& _Tensor<T*>::operator *(U val) {
//	return _Tensor<U>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator+(_Tensor<T> &obj)
//{
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator-(_Tensor<T> &obj)
//{
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator*(_Tensor<T> &obj)
//{
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator/(_Tensor<T> &obj)
//{
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}


//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator+(T val){
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T*>
//_Tensor<T>& _Tensor<T*>::operator-(T val){
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator*(T val){
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator/(T val){
//    _Tensor<T>& ret = *new _Tensor<T>();
//	return ret;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator <<(T val){
//    v.clear();v.shrink_to_fit();
//    v.push_back(val);
//	return *this;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator ,(T val){
//    v.push_back(val);
//	return *this;
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator+=(_Tensor<T> &obj)
//{
//    return *new _Tensor<T>();
//}


//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator-=(_Tensor<T> &obj)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator*=(_Tensor<T> &obj)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator/=(_Tensor<T> &obj)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator+=(T val)
//{
//    return *new _Tensor<T>();
//}


//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator-=(T val)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator*=(T val)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator/=(T val)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator=(T val)
//{
//    return *new _Tensor<T>();
//}

//template<typename T>
//_Tensor<T>& _Tensor<T*>::operator [] (std::map<std::string, int> &_indices){
//    std::map<std::string, int> shape2(shape);
//    std::map<std::string, int> ud2(ud);

//    for(auto pair: shape){
//        if(_indices.count(pair.first)){
//            shape2.erase(pair.first);
//            ud2.erase(pair.first);
//        }
//    }

//	_Tensor<T*>& ret = gen(shape2, ud2);

//	for (int i = 0; i < ret.size(); i++){
//        auto indices = ret.genIndices(i);
//        auto thisIndices = indices;
//        for(auto pair: _indices){
//            thisIndices[pair.first] = pair.second;
//        }
//		ret.ref(indices) = ref(thisIndices);
//    }

//	return ret;
//}

////・protected member functions ・‥…─*・‥…─*・‥…─*・‥…─*・‥…─*

//template<typename T>
//std::map<std::string, int> _Tensor<T*>::calcStep(){
//	int mul(1);
//	std::map<std::string, int> ret;
//	if (idx.size()) {
//		ret[idx[idx.size() - 1]] = mul;
//		if (idx.size() > 1)
//			for (int i = idx.size() - 2; i >= 0; i--) {
//				ret[idx[i]] = mul = mul * shape[idx[i + 1]];
//			}
//	}
//	return ret;
//}

//template<typename T>
//void _Tensor<T*>::putLeftBracket(int i) {

//	for (int j = 0; j < ndim-1; j++) {
//		if (i % step[idx[j]]) {
//			std::cout << " ";
//		} //else if (step[idx[j]] == 1)
//	}
//	if (i) 
//		std::cout << " ";

//	for (int j = 0; j < ndim-1; j++) {
//		if (!(i % step[idx[j]])) {
//			std::cout << "[";
//		}
//	}

//}


//template<typename T>
//void _Tensor<T*>::putValue(int i) {

//	std::cout 
//		<< std::setw(NUM_OF_PRECISION+2)
//		<< std::setprecision(NUM_OF_PRECISION) 
//		<< ref(genIndices(i)) 
//		<< ", ";

//	if (i != 0 && 
//		(i % shape[idx[ndim-1]]) &&
//		!((i % shape[idx[ndim-1]]) % NUM_OF_WRAP) &&
//		((i+1) % shape[idx[ndim-1]])
//	   ) {
//		std::cout << std::endl;
//		std::cout
//			<< std::setw(NUM_OF_PRECISION-4)
//			<< std::setfill(' ') << " ";
//	}


//}

//template<typename T>
//void _Tensor<T*>::putRightBracket(int i) {

//	int cnt(0);
//	for (int j = ndim-2; j >= 0; j--) {
//		bool tof(true);
//		if (!((i + 1) % step[idx[j]])) {
//			std::cout << "]";

//			if (j - 1 >= 0) {
//				if ((i + 1) % step[idx[j - 1]])
//					std::cout << std::endl;
//			}else{
//				if (i != N-1)
//					std::cout << std::endl;
//			}
//		}
//	}
//}

//template<typename T>
//bool _Tensor<T*>::isExistThisIndex(std::map<std::string, int> indices, std::string i) {
//	return indices.count(i);
//}

