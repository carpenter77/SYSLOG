

#ifndef SINGLETON_H
#define SINGLETON_H
#include <memory>
namespace syslog{
    template <typename T>
    static T& GetInstance(){
        static T v;
        return &v;      
    }
    template <typename T>
    static std::shared_ptr<T> GetInstancePtr(){
        static auto vPtr=std::make_shared<T>();      
        return vPtr;
    }  
}
#endif