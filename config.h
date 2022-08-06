#ifndef LOG_CONFIG
#define LOG_CONFIG

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <functional>
#include "log.h"
#include "singleton.h"
#include "yaml-cpp/yaml.h"
#include "util.h"
using namespace std;
namespace syslog{
class ConfigVarBase{
    public:
    typedef shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(string name,string desc)
            :_name(name)
            ,_description(desc){
                std::transform(_name.begin(),_name.end(),_name.begin(),::tolower);
            }
    virtual ~ConfigVarBase(){}
    
    const string& getName() {
        return _name;
    }
    
    const string& getDescription(){
        return _description;
    }
    
    virtual const std::string getTypeName() const =0;
 
    virtual bool fromString(const string& val)=0;
    
    virtual std::string toString() = 0;
   
    private:
         string _name;
         string _description;

};
template<typename F,typename T>
class LeixCalCast{
    public:
        T operator()(const F& v){
            return boost::lexical_cast<T>(v);
        }
};
/*
*@brief 对string类型做片特化
*/
template<typename T>
class LeixCalCast<std::string,std::vector<T> >{
    public:
        std::vector<T> operator()(const string& v){
           YAML::Node node=YAML::Load(v);
           typename std::vector<T> vc;
           for(int i=0;i<node.size();i++){
               std::stringstream ss("");
               ss<<node[i];
               vc.push_back(LeixCalCast<std::string,T>()(ss.str()));
           } 
           return vc;
        }
};

template<typename T>
class LeixCalCast<std::vector<T>,std::string >{
    public:
        const string  operator()(std::vector<T> v){
           YAML::Node node(YAML::NodeType::Sequence);
           for(int i=0;i<v.size();i++){
               node.push_back(YAML::Load(LeixCalCast<T,std::string>()(v[i])) );
           } 
           std::stringstream ss;
           ss<<node;
           return ss.str();
        }
};

template<typename T>
class LeixCalCast<std::string,std::set<T> >{
    public:
        std::set<T> operator()(const string& v){
           YAML::Node node=YAML::Load(v);
           typename std::set<T> vc;
           for(int i=0;i<node.size();i++){
               std::stringstream ss("");
               ss<<node[i];
               vc.insert(LeixCalCast<std::string,T>()(ss.str()));
           } 
           return vc;
        }
};

template<typename T>
class LeixCalCast<std::set<T>,std::string >{
    public:
        const string  operator()(std::set<T> v){
           YAML::Node node(YAML::NodeType::Sequence);
           for(auto& i:v){
               node.push_back(YAML::Load(LeixCalCast<T,std::string>()(i)) );
           } 
           std::stringstream ss;
           ss<<node;
           return ss.str();
        }
};

template<typename T>
class LeixCalCast<std::string,std::map<std::string,T> >{
    public:
        std::map<std::string,T> operator()(const string& v){
           YAML::Node node=YAML::Load(v);
           typename std::map<std::string,T> vc;
           for(auto it=node.begin();it!=node.end();it++){
               stringstream ss("");
               ss<<it->second;
               vc.insert( make_pair(it->first.Scalar(),LeixCalCast<std::string,T>()(ss.str()) ));
           }
           return vc;
        }
};

template<typename T>
class LeixCalCast<std::map<std::string,T>,std::string >{
    public:
        const string  operator()(std::map<std::string,T> v){
           YAML::Node node(YAML::NodeType::Sequence);
           for(auto& i:v){
               node[i.first]=(YAML::Load(LeixCalCast<T,std::string>()(i.second)) );
           } 
           std::stringstream ss;
           ss<<node;
           return ss.str();
        }
};
template<typename T>
class LeixCalCast<std::unordered_set<T>,std::string >{
    public:
        const string  operator()(std::unordered_set<T> v){
           YAML::Node node(YAML::NodeType::Sequence);
           for(auto& i: v){
               node.push_back(YAML::Load(LeixCalCast<T,std::string>()(i)) );
           } 
           std::stringstream ss;
           ss<<node;
           return ss.str();
        }
};

template<typename T>
class LeixCalCast<std::string,std::unordered_set<T>>{
    public:
        std::unordered_set<T> operator()(const string& v){
           YAML::Node node=YAML::Load(v);
           typename std::unordered_set<T> vc;
           for(int i=0;i<node.size();i++){
               std::stringstream ss("");
               ss<<node[i];
               vc.insert(LeixCalCast<std::string,T>()(ss.str()));
           } 
           return vc;
        }
};

template<typename T,typename FromStr=LeixCalCast<std::string,T>
    ,typename ToStr=LeixCalCast<T,std::string>>
class ConfigVar: public ConfigVarBase{
    public:
        typedef shared_ptr<ConfigVar> ptr;
        typedef function<void(const T& old_val, const T& new_val)> change_call_back;

        ConfigVar(const string& name,const T& default_value,const string& desc)
        :m_val(default_value)
        ,ConfigVarBase(name,desc){

        }
        string toString() override{
            try{
                return ToStr()(m_val);
            }catch(std::exception& e){
                string str=string_format("%s\n cannot convert %s to string\n",e.what(),typeid(m_val).name());
                LOG_ERROR(syslog::GetInstancePtr<Logger>(),str);
            }
            return "";
        }
        bool fromString(const string& val) override{
            try{
                m_val=FromStr()(val);
                return true;
            }catch(std::exception& e){
                string str=string_format("%s\n cannot convert string to %s \n",e.what(),typeid(m_val).name());
                LOG_ERROR(syslog::GetInstancePtr<Logger>(),str);
            }
            return false;
        }
        const T getValue() { return m_val;}

        void setValue(const T& new_value){
            if(new_value==m_val){
                return;
            }
            for(auto& f: m_cbs){
                f(m_val,new_value);
            }
            m_val=new_value;
        }

        const std::string getTypeName() const override { return TypeToName<T>();}

        void addListener(uint64_t key,change_call_back bc){
            m_cbs[key]=bc;
        }

        void delListener(uint64_t key){
            m_cbs.erase(key);
        }
        
        change_call_back getListener(uint64_t key){
            auto it=m_cbs.find(key);
            return it==m_cbs.end()? nullptr: it->second;
        }

    private:
        T m_val;
        //回调函数集合，当有新值更新的时候能够通知该函数。为了区分的不同的回调函数，使用uint64来区别不同的函数
        std::map<uint64_t,change_call_back> m_cbs;
};

class Config{
    public:
        typedef map<string,ConfigVarBase::ptr> ConfigVarMap;
        
    private:
        //static ConfigVarMap GetDatas();        
    public:
        template<typename T>
        static typename ConfigVar<T>::ptr lookUp(const string& name){
            auto it=GetDatas().find(name);
            if(it!=GetDatas().end()){
                return nullptr;
            }                
            auto ret=dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if(ret){
                LOG_INFO(GET_ROOT_LOGGER()," look name "+ name +" exists");
                return ret;
            }else{
                std::string str=string_format("LookUp name: %s, exists but not type: %s, real type: %s",name,TypeToName<T>(),it->second->getTypeName());
                LOG_ERROR(GET_ROOT_LOGGER(),str);
                return ret;
            }
        } 
        
        template<typename T>
        static typename ConfigVar<T>::ptr lookUp(const string& name,const T& default_value,const string& desc=""){
                //原来存在，但是类型不正确
                auto it=GetDatas().find(name);
                if(it!=GetDatas().end()){
                    auto tmp=dynamic_pointer_cast<ConfigVar<T>>(it->second);
                    if(tmp){
                        LOG_INFO(syslog::GetInstancePtr<Logger>()," look name "+ name +" exists");
                        return tmp;
                    }else{
                        std::string str=string_format("LookUp name: %s, exists but not type: %s, real type: %s",name,TypeToName<T>(),it->second->getTypeName());
                        LOG_ERROR(GET_ROOT_LOGGER(),str);
                    }
                }
                if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
                != std::string::npos) {
                    LOG_ERROR(syslog::GetInstancePtr<Logger>(),"Lookup name invalid "+name);
                    throw std::invalid_argument(name);
                }
                typename ConfigVar<T>::ptr v(new ConfigVar<T>(name,default_value,desc));
                GetDatas()[name]=v;
                return v;
        }
        static ConfigVarMap& GetDatas() {
            static ConfigVarMap s_datas;
            return s_datas;
        }
        
        static ConfigVarBase::ptr lookUpBase(const string &key);

        static void loadFromYaml(const YAML::Node root);
};
//Config::ConfigVarMap s_datas;
}
#endif