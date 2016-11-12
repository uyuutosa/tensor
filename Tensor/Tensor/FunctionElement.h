#pragma once
#include <iostream>
#include <map>
#include <vector>

//

template<typename T>
class FunctionElement{
public:
    FunctionElement(){
    }


    virtual T& func(T& x){
        return x;
	}

    T& operator *(T& x){
        return func(x);
    }

    int getRank(){
        return rank;
    }

    std::map<std::string, int> indices;

    int rank;
    int i, j, k, l, m, n, o, p, q, r;
    T& (*f1) (int, T&);
    T& (*f2) (int, int, T&);
    T& (*f3) (int, int, int, T&);
    T& (*f4) (int, int, int, int, T&);
    T& (*f5) (int, int, int, int, int, T&);
    T& (*f6) (int, int, int, int, int, int, T&);
    T& (*f7) (int, int, int, int, int, int, int, T&);
    T& (*f8) (int, int, int, int, int, int, int, int, T&);
    T& (*f9) (int, int, int, int, int, int, int, int, int, T&);
    T& (*f10)(int, int, int, int, int, int, int, int, int, int, T&);
};

template<typename T>
class FunctionElementRank1: public FunctionElement<T>{
public:
    FunctionElementRank1(int _i, T& (*_f)(int, T&)):
        i(_i),rank(1){
       f1 = _f;
    };

    FunctionElementRank1(std::map<std::string, int> _indices,
                        T& (*_f)(int, T&))
        {
        indices = _indices;
        f1 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        rank = 1;
    }

         

    T& func(T& x){
        return (*f1)(i, x);
	}

};

template<typename T>
class FunctionElementRank2: public FunctionElement<T>{
public:
    FunctionElementRank2(int _i, int _j, T& (*_f)(int, int, T&)):
        i(_i), j(_j),rank(2){
       f2 = _f;
    };

    FunctionElementRank2(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, T&))
        {
        indices = _indices;
        f2 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        rank = 2;
    }

    T& func(T& x){
        return (*f2)(i, j, x);
	}
};

template<typename T>
class FunctionElementRank3: public FunctionElement<T>{
public:
    FunctionElementRank3(int _i, int _j, int _k, T (*_f)(int, int, int, T)):
        i(_i), j(_j), k(_k),rank(3){
       f3 = _f;
    };

    FunctionElementRank3(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, T&))
        {
        indices = _indices;

        f3 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
    }

    T& func(T& x){
        return (*f3)(i, j, k, x);
	}
};

template<typename T>
class FunctionElementRank4: public FunctionElement<T>{
public:
    FunctionElementRank4(int _i, int _j, int _k, int _l, 
                         T& (*_f)(int, int, int, int, T&)):
        i(_i), j(_j), k(_k), l(_l){
       f4 = _f;
    };

    FunctionElementRank4(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, T&))
        {
        indices = _indices;

        f4 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
    }

    T& func(T& x){
        return (*f4)(i, j, k, l, x);
	}
};

template<typename T>
class FunctionElementRank5: public FunctionElement<T>{
public:
    FunctionElementRank5(int _i, int _j, int _k, int _l, int _m, 
                         T& (*_f)(int, int, int, int, int, T&)):
        i(_i), j(_j), k(_k), l(_l), m(_m){
       f5 = _f;
    };

    FunctionElementRank5(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, int, T&))
        {
        indices = _indices;

        f5 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
        m = tmp[4];
    }

    T& func(T& x){
        return (*f5)(i, j, k, l, m, x);
	}
};

template<typename T>
class FunctionElementRank6: public FunctionElement<T>{
public:
    FunctionElementRank6(int _i, 
                         int _j, 
                         int _k, 
                         int _l, 
                         int _m, 
                         int _n,
                         T (*_f)(int, int, int, int, int, T)):
        i(_i), j(_j), k(_k), l(_l), m(_m), n(_n){
       f6 = _f;
    };

    FunctionElementRank6(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, int, int, T&))
        {
        indices = _indices;

        f6 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
        m = tmp[4];
        n = tmp[5];
    }

    T& func(T& x){
        return (*f6)(i, j, k, l, m, n, x);
	}
};

template<typename T>
class FunctionElementRank7: public FunctionElement<T>{
public:
    FunctionElementRank7(int _i, 
                         int _j, 
                         int _k, 
                         int _l, 
                         int _m, 
                         int _n,
                         int _o,
                         T (*_f)(int, int, int, int, int, int, int, T)):
        i(_i), j(_j), k(_k), l(_l), m(_m), n(_n){
       f7 = _f;
    };

    FunctionElementRank7(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, int, int, int, T&))
        {
        indices = _indices;

        f7 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
        m = tmp[4];
        n = tmp[5];
        o = tmp[6];
    }

    T& func(T& x){
        return (*f7)(i, j, k, l, m, n, o, x);
	}
};

template<typename T>
class FunctionElementRank8: public FunctionElement<T>{
public:
    FunctionElementRank8(int _i, 
                         int _j, 
                         int _k, 
                         int _l, 
                         int _m, 
                         int _n,
                         int _o,
                         int _p,
                         T& (*_f)(int, int, int, int, int, int, int, int, T&)):
        i(_i), j(_j), k(_k), l(_l), m(_m), n(_n), o(_o), p(_p){
       f8 = _f;
    };

    FunctionElementRank8(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, int, int, int, int, T&))
        {
        indices = _indices;

        f8 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
        m = tmp[4];
        n = tmp[5];
        o = tmp[6];
        p = tmp[7];
    }

    T& func(T& x){
        return (*f8)(i, j, k, l, m, n, o, p, x);
	}
};

template<typename T>
class FunctionElementRank9: public FunctionElement<T>{
public:
    FunctionElementRank9(int _i, 
                         int _j, 
                         int _k, 
                         int _l, 
                         int _m, 
                         int _n,
                         int _o,
                         int _p,
                         T& (*_f)(int, int, int, int, int, int, int, int, T&)):
        i(_i), j(_j), k(_k), l(_l), m(_m), n(_n), o(_o){
       f9 = _f;
    };

    FunctionElementRank9(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, int, int, int, int, int, T&))
        {
        indices = _indices;

        f9 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
        m = tmp[4];
        n = tmp[5];
        o = tmp[6];
        p = tmp[7];
        q = tmp[8];
    }

    T& func(T& x){
        return (*f9)(i, j, k, l, m, n, o, p, q, x);
	}
};

template<typename T>
class FunctionElementRank10: public FunctionElement<T>{
public:
    FunctionElementRank10(int _i, 
                          int _j, 
                          int _k, 
                          int _l, 
                          int _m, 
                          int _n,
                          int _o,
                          int _p,
                          int _q,
                          int _r,
                          T& (*_f)(int, int, int, int, int, int, int, int, int, int, T&)):
        i(_i), j(_j), k(_k), l(_l), m(_m), n(_n), o(_o), p(_q), r(_r){
       f10 = _f;
    };

    FunctionElementRank10(std::map<std::string, int> _indices,
                        T& (*_f)(int, int, int, int, int, int, int, int, int, int, T&))
        {
        indices = _indices;

        f10 = _f;
        std::vector<int> tmp;
        for(auto pair: indices)
            tmp.push_back(pair.second);
        i = tmp[0];
        j = tmp[1];
        k = tmp[2];
        l = tmp[3];
        m = tmp[4];
        n = tmp[5];
        o = tmp[6];
        p = tmp[7];
        q = tmp[8];
        r = tmp[9];
    }

    T& func(T& x){
        return (*f10)(i, j, k, l, m, n, o, p, q, r, x);
	}
};
