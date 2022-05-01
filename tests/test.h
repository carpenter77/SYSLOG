#include<map>
#include <string>
#include <iostream>
using namespace std;
template<typename T>
class A{
    public:
    A(){   
    }
    static T getVal(string key){    
        cout<<"daats address: "<<&getMap()<<endl;
        if(getMap().find(key)!=getMap().end()){
            return getMap()[key];
        }
        return -1;
    }
    static void put(string key,T value){
        getMap()[key]=value;
    }
    private:
        static map<string,T>& getMap(){
            static map<string,T> datas;
            return datas;
        }
};
