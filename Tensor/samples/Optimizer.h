#include <iostream>
#include <Tensor.h>
#include <TensorUtil.h>
#include <vector>

// Levenberg–Marquardt algorithm with Tensor<T>

class Optimizer{
public:
    Optimizer():
    x(*new Tensor<double>()),
    y(*new Tensor<double>())
    {}

    //Optimizer(const Optimizer &obj){
    //    *this = *const_cast<Optimizer*>(&obj);
    //}

    Optimizer(void* _func, 
              Tensor<double> &_x, 
              Tensor<double> &_y, 
              Tensor<double> &_ini_a,
              double _delta=0.1,
              double _delta_lambda_dec=0.2,
              double _delta_lambda_inc=2,
              double _delta_residual=0.01,
              int    _maxCnt=1e3):
        x(_x), 
        y(_y), 
        a(_ini_a),
        delta(_delta),
        delta_lambda_dec(_delta_lambda_dec),
        delta_lambda_inc(_delta_lambda_inc),
        delta_residual(_delta_residual),
        maxCnt(_maxCnt),
        t(*new Tensor<Tensor<double> >()),
        func(*new FunctionTensor<Tensor<double> > (_func, tu::s("q", 1, ""), tu::s("q", 0, "")))
    {
        E_old = 1e5;
        lambda = 0.05;
        isNotFirstTime = false;
        w = tu::ones<double>(tu::s("j", x.shape["j"],""), tu::s("j", 1, ""));
    }

    void train(bool isShow=false){
        cnt = 0;
        do{
            update();
            E = abs(((y - f) * (y - f).cud("j", 1, "")).getV()[0]);
            tmp_a = a;
            auto add = (alpha.inv("kl") * beta.cidx("k", "l", "").cud("l", 1, ""));
            a_prime = a + add * 100;
            a = a_prime;
            update();
            E_prime = abs(((y - f) * (y - f).cud("j", 1, "")).getV()[0]);
            if(E_prime <= E){
                lambda *= delta_lambda_dec;
                std::cout << "low" << std::endl;
            } else{
                a = tmp_a;
            	lambda *= delta_lambda_inc;
                std::cout << lambda << std::endl;
            }
            cnt ++;
            if(isShow)
                showError();
        } while(E > delta_residual && cnt < maxCnt);
    }

    void showError(){
            std::cout << E << std::endl;
    }


    void update(){
        calcT();
		calcJacobian();
		calcAlpha();
		calcBeta();
    }

    void calcT(){
        t = *new Tensor<Tensor<double> >(tu::s("q", 1, ""), tu::s("q", 0, ""));
        t.ref(tu::s("q", 0,"")) = a.concat(x, "k", "i", "l", 0);
    }

    void calcJacobian(){
        std::vector<Tensor<double>> v;
        auto &obj = (func * t); 
        f = obj.merge<double>();

        for (int i=0; i<a.getShape()["k"]; i++){
            v.push_back(func.grad(t, tu::s("l", i, ""), delta).merge<double>());
        }

        ttk = Tensor<Tensor<double> >(v, tu::s("k", a.getShape()["k"], ""));
        ttl = Tensor<Tensor<double> >(v, tu::s("l", a.getShape()["k"], ""));
        tk = ttk.merge<double>();
        tl = ttl.merge<double>();
        auto ttkl = tk * tl;
    }
    
    void calcAlpha(){
        alpha = (w*(tk * tl) * ((tu::eye<double>(
                                tk.getShape()["k"], 
                                tu::s("k", tk.getShape()["k"],"l",  tk.getShape()["k"], ""),
                                tu::s("k", 0,"l", 0, "")) * lambda) + 1));
    }

    void calcBeta(){
        beta = w * ((y - f) * tk);
    }

    void show(){
        std::cout << "x      = " << x << std::endl;
        std::cout << "y      = " << y << std::endl;
        std::cout << "cnt    = " << cnt << std::endl;
        std::cout << "MaxCnt = " << maxCnt << std::endl;
        std::cout << "lamda  = " << lambda << std::endl;
        std::cout << "E      = " << E << std::endl;
        std::cout << "a      = " << a << std::endl;
    }

    Tensor<double>& getParams(){
        return a;
    }


    double lambda, delta, delta_lambda_dec, delta_lambda_inc, delta_residual;
    Tensor<double> &x, &y;
    Tensor<double> a, a_prime, w, tmp_a;
    Tensor<double> tk, tl;
    Tensor<double> alpha;
    Tensor<double> beta;
    Tensor<double> I;
    Tensor<double> f;
    Tensor<Tensor<double>> func,t, ttk, ttl;
    double E, E_old, E_prime;
    bool isNotFirstTime;
    int cnt, maxCnt;
    
};


