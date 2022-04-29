#ifndef LOG_CONFIG
#define LOG_CONFIG

#include <string>
#include <sstream>
#include "log.h"
#include "singleton.h"
#include "util.h"
using namespace std;
namespace syslog{
class ConfigVarBase{
    public:
    typedef shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(string name,string desc)
            :_name(name)
            ,_description(desc){   
            }
    virtual ~ConfigVarBase(){}
    const string& getName() {
        return _name;
    }
    const string& getDescription(){
        return _description;
    }
    private:
         string _name;
         string _description;
    virtual string toString()=0;
    virtual bool fromString(const string& val)=0;
};

template<typename T>
class ConfigVar: public ConfigVarBase{
    public:
        typedef shared_ptr<ConfigVar> ptr;
        ConfigVar(const string& name,const T& default_value,const string& desc)
        :m_val(default_value)
        ,ConfigVarBase(name,desc){

        }
        string toString() override{
            try{
                stringstream ss;
                ss<<m_val;
                return ss.str();
            }catch(std::exception& e){
                string str=string_format("%s\n cannot convert %s to string\n",e.what(),typeid(m_val).name());
                LOG_ERROR(syslog::GetInstancePtr<Logger>(),str);
            }
            return "";
        }
        bool fromString(const string& val) override{
            try{
                stringstream ss(val);
                ss>>m_val;
                return true;
            }catch(std::exception& e){
                string str=string_format("%s\n cannot convert string to %s \n",e.what(),typeid(m_val).name());
                LOG_ERROR(syslog::GetInstancePtr<Logger>(),str);
            }
            return false;
        }
        const T getValue() { return m_val;}
    private:
        T m_val;
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
            return dynamic_pointer_cast<ConfigVar<T>>(it->second);
        } 
        
        template<typename T>
        static typename ConfigVar<T>::ptr LookUp(const string& name,const T& default_value,const string& desc=""){
                auto tmp=lookUp<T>(name);
                if(tmp){
                    LOG_INFO(syslog::GetInstancePtr<Logger>()," look name "+ name +" exists");
                    return tmp;
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
        
};
//Config::ConfigVarMap s_datas;
}
#endif