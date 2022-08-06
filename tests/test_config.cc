#include <iostream>
#include <string>
#include "log.h"
#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include "yaml-cpp/yaml.h"
#include <vector>

using namespace std;
using namespace syslog;

Logger::loggerPtr logger=GET_ROOT_LOGGER();
void test_yaml();
void printNode(YAML::Node,int);

ConfigVar<int>::ptr global_int_val= Config::lookUp("system.port",(int)8080,"socket port");
ConfigVar<float>::ptr global_float_val= Config::lookUp("system.value",(float)12.12,"system value");
ConfigVar<vector<int>>::ptr global_vec_val= Config::lookUp("system.info",std::vector<int>{1,2,3},"system vector value");
ConfigVar<set<int>>::ptr global_set_val= Config::lookUp("system.set_info",std::set<int>{1,2},"system set value");
ConfigVar<unordered_set<int>>::ptr global_unordered_set_val= Config::lookUp("system.unorder_set_info",std::unordered_set<int>{3,2},"system unorder_set value");
ConfigVar<map<string,int>>::ptr global_map_val= Config::lookUp("system.map_info",std::map<string,int>{{"key1",2}},"system map value");

const std::string DB_CONF = "./config.yaml";
int main(){
    cout<<" ----------- test parse yaml -------"<<endl;
    test_yaml();
    #define PRINTXX(prefix,name,var) \
        {\
            for(auto& i:var->getValue()){ \
                stringstream ss(""); \
                ss<<#prefix<<" "; \
                ss<<#name; \
                ss<<" in vec: "; \
                ss<<i<<endl; \
                LOG_INFO(GET_ROOT_LOGGER(),ss.str());\
            }\
            stringstream ss(""); \
            ss<<#prefix<<" "; \
            ss<<#name; \
            ss<<" yaml: "<<var->toString();\
            LOG_INFO(GET_ROOT_LOGGER(),ss.str());\
        }
    #define PRINTMAP(prefix,name,var) \
        {\
            for(auto& i:var->getValue()){ \
                stringstream ss(""); \
                ss<<#prefix<<" "; \
                ss<<#name; \
                ss<<" in map: "; \
                ss<<"{"<<i.first<<" : "<<i.second<<"}"<<endl; \
                LOG_INFO(GET_ROOT_LOGGER(),ss.str());\
            }\
            stringstream ss(""); \
            ss<<#prefix<<" "; \
            ss<<#name; \
            ss<<" yaml: "<<var->toString();\
            LOG_INFO(GET_ROOT_LOGGER(),ss.str());\
        }
    LOG_INFO(logger,global_int_val->toString());
    LOG_INFO(logger,global_float_val->toString());
    
    PRINTXX(before,in_vec,global_vec_val);
    PRINTXX(before,set_vec,global_set_val);
    PRINTXX(before,unordered_set_vec,global_unordered_set_val);
    PRINTMAP(before,map_vec,global_map_val);
    cout<<"after load yaml ......."<<endl;

    YAML::Node conf = YAML::LoadFile(DB_CONF);
    Config::loadFromYaml(conf);    
    LOG_INFO(logger,global_int_val->toString());
    LOG_INFO(logger,global_float_val->toString());
    
    PRINTXX(after,in_vec,global_vec_val);
    PRINTXX(after,set_vec,global_set_val);
    PRINTXX(after,unordered_set_vec,global_unordered_set_val);
    PRINTMAP(after,map_vec,global_map_val);
    return 0;
}

void test_yaml() {
    /* Node conf. */
    YAML::Node conf = YAML::LoadFile(DB_CONF);
    printNode(conf,0);
}

void printNode(YAML::Node root,int level){
    if(root.IsScalar()){
        stringstream ss;
        ss<<string(level+2,' ')<<" - "<<root.Scalar()<<" -type: "<<root.Type()<<" -level: "<<level<<endl;
        LOG_INFO(logger,ss.str());
    }else if(root.IsMap()){
        for(auto it=root.begin();it!=root.end();it++){
            stringstream ss;
            ss<<string(level+2,' ')<<" - "<<it->first<<" -type: "<<it->second.Type()<<" -level: "<<level<<endl;
            LOG_INFO(logger,ss.str());
            printNode(it->second,level+1);
        }
    }else if(root.IsSequence()){
        for(int i=0;i<root.size();i++){
            stringstream ss;
            ss<<string(level+2,' ')<<" - "<<root[i]<<" -type: "<<root[i].Type()<<" -level: "<<level<<endl;
            LOG_INFO(logger,ss.str());
            printNode(root[i],level+1);
        }
    }else if(root.IsNull()){
        stringstream ss;
        ss<<string(level+2,' ')<<" - "<<"NULL"<<" -type: "<<root.Type()<<" -level: "<<level<<endl;

    }
}