#include "config.h"
#include <sstream>
#include "yaml-cpp/yaml.h"

namespace syslog{
static void listAllNode(const YAML::Node root,const std::string& prefix,std::list<std::pair<std::string,const YAML::Node> >& outputs){
    if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")!=prefix.npos){
        std::stringstream ss;
        ss<<"invalid config parma"<<prefix<<" : "<<endl; 
        LOG_ERROR(GET_ROOT_LOGGER(),ss.str());
        return ;
    }
    outputs.push_back(std::make_pair(prefix,root));
        if(root.IsMap()){
        for(auto it=root.begin();it!=root.end();it++){
            std::string key=it->first.Scalar();
            std::transform(key.begin(),key.end(),key.begin(),::tolower);
            listAllNode(it->second,prefix.empty()? key:prefix+"."+key,outputs);
        }
    }
}

void Config::loadFromYaml(const YAML::Node root){
    std::list<pair<std::string,const YAML::Node>> list;
    listAllNode(root,"",list);
    for(auto it=list.begin();it!=list.end();it++){
        std::string key=it->first;
        if(key.empty()){
            continue;
        }
        ConfigVarBase::ptr var=lookUpBase(key);
        if(var){
            if(it->second.IsScalar()){
                var->fromString(it->second.Scalar());
            }else{
                stringstream ss;
                ss<<it->second;
                var->fromString(ss.str());
            }
        }
    } 
}

    ConfigVarBase::ptr Config::lookUpBase(const string& name){
        auto it = GetDatas().find(name);
        return it == GetDatas().end() ? nullptr : it->second;
    }

}
