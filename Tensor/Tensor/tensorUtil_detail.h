#pragma once
#include "TensorUtil.h"
//#pragma optimize ("", off)

namespace tu {

//int cnt(0);
//std::map<std::string, int> ret_global;
//std::string key;
//int val(0);
//std::map<std::string, int> s() {
//	std::map<std::string, int> ret(ret_global);
//	ret_global.clear();
//	return ret;
//}

//template <class Head, class... Tail>
//std::map<std::string, int> s(Head head, Tail... tail)
//{
//	bool tof(cnt++ % 2);
//	if(!tof)
//		key = (const char*)head;

//	if(tof)
//		val = (int)head;

//	if (key.size() && val != 0) {
//		ret_global[key] = val;
//		key = "";
//		val = 0;
//		cnt = 0;
//	}
//	// パラメータパックtailをさらにheadとtailに分割する
//	//print(std::move(tail)...);
//	return s(tail...);
//}

template<typename T>
Tensor<T> zeros(std::map<std::string, int> shape, std::map<std::string, int> ud) {
	int N(1);
	for (auto pair : shape)
		N *= pair.second;

    auto& v = *new std::vector<T*>(N);
    for(int i=0; i<v.size(); i++){
        v[i] = new T(0);
    }

	return Tensor<T>(v, shape, ud);
}

template<typename T>
Tensor<T> ones(std::map<std::string, int> shape, std::map<std::string, int> ud) {
	int N(1);
	for (auto pair : shape)
		N *= pair.second;

    auto& v = *new std::vector<T*>(N);
    for(int i=0; i<v.size(); i++)
        v[i] = new T(1);
	return Tensor<T>(v, shape, ud);
}

template<typename T>
Tensor<T> sqrt_t(Tensor<T> &obj){
    Tensor<T>& ret = obj.clone(); 

    std::vector<T*> &V = obj.getT()->getV();
    for(int i=0; i<obj.size(); i++){
        (*V[i]) = sqrt(*V[i]);
    }

	return ret;
}

template<typename T>
Tensor<T> exp_t(Tensor<T> &obj){
    Tensor<T>& ret = obj.clone(); 

    std::vector<T*> &V = obj.getT()->getV();
    std::vector<T*> &retV = ret.getT()->getV();
    for(int i=0; i<obj.getT()->size(); i++){
        (*retV[i]) = exp((*V[i]));
    }

	return ret;
}

template<typename T>
Tensor<T> sin_t(Tensor<T> &obj){
    Tensor<T>& ret = obj.clone(); 

    std::vector<T*> &V = obj.getT()->getV();
    std::vector<T*> &retV = ret.getT()->getV();
    for(int i=0; i<obj.getT()->size(); i++){
        (*retV[i]) = sin((*V[i]));
    }

	return ret;
}

template<typename T>
Tensor<T> cos_t(Tensor<T> &obj){
    Tensor<T> ret = obj.clone(); 

    std::vector<T*> &V = obj.getT()->getV();
    std::vector<T*> &retV = ret.getT()->getV();
    for(int i=0; i<obj.getT()->size(); i++){
        (*retV[i]) = cos((*V[i]));
    }

	return ret;
}

template<typename T>
Tensor<T> Mat2Tensor(cv::Mat &obj, 
                      std::map<std::string, int> _shape,
                      std::map<std::string, int> _ud){

    std::vector<T*>& v = *new std::vector<T*>(obj.total()*obj.elemSize());
    for(int i=0; i<obj.rows; i++){
        T* p = obj.ptr<T>(i);
        for(int j=0; j<obj.cols; j++){
            if(obj.channels() > 1){
                v[i * obj.cols * 3 + j * 3 + 0] =
                    new T(p[j * 3 + 0]);

                v[i * obj.cols * 3 + j * 3 + 1] =
                    new T(p[j * 3 + 1]);

                v[i * obj.cols * 3 + j * 3 + 2] =
                    new T(p[j * 3 + 2]);
                
            } else {
                v[i * obj.cols + j] = new T(p[j]);
            }
        }
    }
    
    return Tensor<T>(v, _shape, _ud);
}

template<typename T>
cv::Mat Tensor2Mat(Tensor<T> &obj, int type, std::string _indices){ 
    cv::Mat ret;
    if(_indices.size() > 2)
        ret = cv::Mat(obj.getShape()[_indices.substr(0, 1)],
    	            obj.getShape()[_indices.substr(1,1)],
    	            type);
    else
        ret = cv::Mat(obj.getShape()[_indices.substr(0,1)],
    	            obj.getShape()[_indices.substr(1,1)],
    	            type);

    obj.putBack(_indices);

    for(int i=0; i<ret.rows; i++){
        T* p = ret.ptr<T>(i);
        for(int j=0; j<ret.cols; j++){
            if(ret.channels() > 1){
                p[j * 3 + 0] = obj.ref(obj.getT().genIndices(i * ret.cols * 3 + j * 3 + 0));
                p[j * 3 + 1] = obj.ref(obj.getT().genIndices(i * ret.cols * 3 + j * 3 + 1));
                p[j * 3 + 2] = obj.ref(obj.getT().genIndices(i * ret.cols * 3 + j * 3 + 2));
            } else {
                p[j] = obj.ref(obj.getT().genIndices(i * ret.cols + j));
            }
        }
    }
    
    return ret;
}

template<typename T>
Tensor<T> eye(int _N, std::map<std::string, int> shape, std::map<std::string, int> ud) {
	int N(1);
	for (auto pair : shape){
		N *= pair.second;
    }

	std::vector<T*>& v = *new std::vector<T*>(N, 0);
    int cnt(0);
    for (int i=0; i<N; i++)
        if (cnt == i){
            v[i] = new T(1);
            cnt+=_N+1;
        } else 
            v[i] = new T(0);

	return Tensor<T>(v, shape, ud);
}

template<typename T>
Tensor<T> normal(double mean, double sigma, std::map<std::string, int> shape, std::map<std::string, int> ud) {
	int N(1);
	for (auto pair : shape)
		N *= pair.second;


	std::vector<T*>& v = *new std::vector<T>(N, 0);
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::normal_distribution<double> distribution(mean, sigma);

    for(int i=0; i<N; i++)
        v[i] = distribution(generator);

	return Tensor<T>(v, shape, ud);

}

template<typename T>
Tensor<T> anyVals(std::map<std::string, int> shape, T val, std::map<std::string, int> ud) {

	int N(1);
	for (auto pair : shape)
		N *= pair.second;

	std::vector<T*>& v = *new std::vector<T*>(N, new T(val));
	return Tensor<T>(v, shape, ud);

}



template<typename T>
std::vector<T> _l(T *list, size_t count){
    //std::vector<T> ret;
    //for (int i=0; i<count; i++){
    //    val = list[i];
    //} 
}

template<typename T>
Tensor<T> t(
             std::map<std::string, int> shape, 
             std::map<std::string, int> ud, 
             ...){

    va_list args;
    va_start(args, ud);

    std::map<std::string, int> ret;

    int cnt(1);

    std::vector<Tensor<T>*> &v = *new std::vector<Tensor<T>*>();
    
    for(auto pair: shape)
        for(int i=0; i<pair.second; i++){
    	    v.push_back(&va_arg(args, Tensor<T>));
    	}

    Tensor<Tensor<T> >& obj = Tensor<Tensor<T> >(v, shape, ud);
    return obj.merge<T>().convertTo<T, ConcreteTensor<T>>();
}

template<typename T>
std::vector<T> l(T list[]){
    size_t count = sizeof(list) / sizeof(T);
    std::vector<T*> ret;
    for (int i=0; i<count; i++){
        ret.push_back(new T(list[i]));
    } 
    return ret;
}


template<typename T>
std::vector<T*>& range(T st, T end, T step){
    std::vector<T*> &ret = *new std::vector<T>();
    for (T i = 0; i < end; i+=step)
        ret.push_back(new T(i));
    return ret;
}

template<typename T>
Tensor<T> rangeTensor(T st, T end, T step, std::string i, int ud){
    std::vector<T*>* ret = new std::vector<T*>();

    for (T i = 0; i < end; i+=step)
        ret->push_back(new T(i));

    return Tensor<T>(*ret, tu::s(i, ret->size(), ""), tu::s(i, ud, ""));
}

}


//#define tul(TYPE, ...) tu::_l<TYPE>(__VA_ARGS__, sizeof((TYPE[]){__VA_ARGS__}) / sizeof(TYPE))
//#define tus(...) tu::_s(__VA_ARGS__, "")
