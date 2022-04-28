//
// Created by think on 2022/4/26.
//
#include "util.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/syscall.h"
#include "unistd.h"
#include<string> 
#include<cstring> 

namespace syslog{

long getThreadId(){
    return syscall(SYS_gettid);
}

uint32_t getFiberId(){
    return 0000;
}
std::string FSUtil::Dirname(const std::string &filename) {
    if(filename.empty()){
        return ".";
    }
    auto pos=filename.rfind("/");
    if(pos==0){
        return "/";
    }else if(pos==std::string::npos){
        return ".";
    }else{
        return filename.substr(0,pos);
    }
}
std::string FSUtil::Basename(const std::string& filename){
    if(filename.empty()){
        return filename;
    }
    auto pos=filename.rfind('/');
    if(pos==std::string::npos){
        return filename;
    }else{
        return filename.substr(pos+1);
    };
}
static int _lstat(const char* file,struct stat* st=nullptr){
    struct stat lst;
    int ret=lstat(file,&lst);
    if(st){
        *st=lst;
    }
    return ret;
}
static int _mkdir(const char *dirname){
    if(access(dirname,F_OK)){
        return 0;
    }
    return mkdir(dirname,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
}
bool FSUtil::Mkdir(const std::string& dirname){
    if(_lstat(dirname.c_str())==0){
       return 0;
    }
    char* path=strdup(dirname.c_str());
    char* ptr=strchr(path+1,'/');
    do{
        for(;ptr;*ptr='/',ptr=strchr(ptr+1,'/')){
            *ptr='0';
            if(_mkdir(path)!=0){
                break;
            }
        }
        if(ptr!=nullptr){
            break;
        }else if(_mkdir(path)!=0){
            break;
        }
        free(path);
        return true;
    }while(0);
    free(path);
    return false;
}
bool FSUtil::OpenForRead(std::ifstream &ifs, const std::string &filename, std::ios_base::openmode mod) {
     ifs.open(filename.c_str(),mod);
     return ifs.is_open();
}
bool FSUtil::OpenForWrite(std::ofstream &ofs, const std::string &filename, std::ios_base::openmode mod) {
    ofs.open(filename,mod);
    if(!ofs.is_open()){
        std::string dir=Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(),mod);
    }
    return ofs.is_open();
}
}