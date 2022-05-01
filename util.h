//
// Created by think on 2022/4/26.
//
#ifndef UTIL
#define UTIL
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cxxabi.h>
namespace syslog{
    long getThreadId();
    uint32_t getFiberId();

    class FSUtil{
    public:
        static void ListAllFile(std::vector<std::string>&files,const std::string& path,const std::string& subfix);
        static bool Mkdir(const std::string& dirname);
        static bool Rm(const std::string& path);
        static bool Mv(const std::string& from,const std::string& to);
        static bool Realpath(const std::string& path,std::string& rpath);
        static std::string Dirname(const std::string& filename);
        static std::string Basename(const std::string& filename);
        static bool OpenForRead(std::ifstream& ifs,const std::string& filename,std::ios_base::openmode mod);
        static bool OpenForWrite(std::ofstream& ofs,const std::string& filename,std::ios_base::openmode mod);

    };

    template<typename ...Args>
    std::string string_format(const std::string& format,Args... args){
        int n=snprintf(nullptr,0,format.c_str(),args...)+1;
        char buf[n];
        snprintf(buf,n,format.c_str(),args...);
        return std::string(buf); 
    }
    
    template<typename T>
    std::string convToString(T val){
        std::stringstream ss;
        ss<<val;
        return ss.str();
    }

    template<class T>
    const char* TypeToName() {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }
}
#endif
