#include "TensorUtil_detail.h"

namespace tu{
std::map<std::string, int> s(std::string first, ...){

    va_list args;
    va_start(args, first);

    std::map<std::string, int> ret;

    int cnt(1);
    std::string name(first);
    while (true) {
        if(!(cnt % 2))
            name = std::string(va_arg(args, char*));
        if (!name.size()){
            va_end(args);
            return ret;
        }
        if(cnt % 2)
            ret[name] = va_arg(args, int);
        cnt++;
    } 
}

}