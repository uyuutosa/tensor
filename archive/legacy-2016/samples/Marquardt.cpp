#include <Optimizer.h>
#include <cmath>

Tensor<double>& asdf(int i, Tensor<double>& a){
    auto& w1 = a[tu::s("l", 0, "")];
    auto& w2 = a[tu::s("l", 1, "")];
    auto& w3 = a[tu::s("l", 2, "")];
    auto& w4 = a[tu::s("l", 3, "")];
    auto& w5 = a[tu::s("l", 4, "")];
    auto& x1 = a[tu::s("l", 5, "")];
    auto& x2 = a[tu::s("l", 6, "")];
    auto& x3 = a[tu::s("l", 7, "")];
    auto& x4 = a[tu::s("l", 8, "")];
    auto& x5 = a[tu::s("l", 9, "")];

    Tensor<double>& ret = w1 * x1 + 
                          w2 * x2 + 
                          w3 * x3 + 
                          w4 * x4 + 
                          w5 * x5;

    return ret;
}

int main(){
    Tensor<double>& x = tu::normal<double>(0, 0.01, tu::s("i", 5, "j" ,10 ,""));
    Tensor<double>& y = x[tu::s("i", 0, "")] * 10 + 
                        x[tu::s("i", 1, "")] * 20 + 
                        x[tu::s("i", 2, "")] * 30 + 
                        x[tu::s("i", 3, "")] * 40 + 
                        x[tu::s("i", 4, "")] * 50;

    Tensor<double>& ini_a = (*new Tensor<double>(tu::s("k", 5, ""))     << 0,0,0,0,0);

    Optimizer opt ((void*)asdf, x, y, ini_a, 0.01, 0.2, 2., 0.0000001, 100);
    //Optimizer& opt = *new Optimizer((void*)asdf, x, y, ini_a, 0.01, 0.2, 2., 0.0000001, 100);
    opt.train();
    opt.show();

}