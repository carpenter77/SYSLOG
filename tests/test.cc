#include<string>
#include <memory>
#include <iostream>
#include <sstream>

using namespace std;
template<typename T>
class Base{
    private:
    T val;
    public:
    Base(T val):val(val){}
    void print(){
        cout<<val<<endl;
    }
};
int main(){
    stringstream ss;
    ss<<"test";
    return 0;
}