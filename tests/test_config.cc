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
void test_yaml();
int main(){
    Logger::loggerPtr logger=GET_ROOT_LOGGER();
    ConfigVar<string>::ptr p(new ConfigVar<string>("port","8080","socket port"));
    LOG_INFO(logger,p->toString());

    ConfigVar<float>::ptr ptr(new ConfigVar<float>("port",(float)12.7f,"socket port"));
    LOG_INFO(logger,ptr->toString());

    test_yaml();
    return 0;
}

const std::string DB_CONF = "./config.yaml";
void test_yaml() {

    /* Node conf. */
    YAML::Node conf = YAML::LoadFile(DB_CONF);
    
    /* vector of name string. */
    std::vector<std::string> name_vec = conf["hello"]["num_config"].as<std::vector<std::string> >();
    if(!name_vec.empty())
      std::cout << name_vec[0] << std::endl;
}
