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

const std::string DB_CONF = "./config.yaml";
int main(){
    cout<<" ----------- test parse yaml -------"<<endl;
    test_yaml();

    LOG_INFO(logger,global_int_val->toString());
    LOG_INFO(logger,global_float_val->toString());
    
    cout<<"after load yaml ......."<<endl;

    YAML::Node conf = YAML::LoadFile(DB_CONF);
    Config::loadFromYaml(conf);    
    LOG_INFO(logger,global_int_val->toString());
    LOG_INFO(logger,global_float_val->toString());
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