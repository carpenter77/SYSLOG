
#include "test.h"
#include<iostream>
using namespace std;
//template<typename T>
//map<string,T> A<T>::datas;
int main(){
    {
      A<int> a;
      a.put("123",12);
      cout<<a.getVal("123")<<endl;
    }
    A<int> b;
    
    cout<<b.getVal("123")<<endl;
    return 0;
}