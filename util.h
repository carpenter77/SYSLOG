//
// Created by think on 2022/4/26.
//
#ifndef UTIL
#define UTIL
#include <vector>
#include <string>
#include <fstream>
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
}
#endif
