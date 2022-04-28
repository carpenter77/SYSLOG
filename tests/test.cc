#include <iostream>
#include <memory>


int main(){
    char* b=nullptr;             
        int n= asprintf(&b, "%d, %d",10,12); 
        if (n!=-1){                  
            std::string str1=std::string(b,n); 
            free(b); 
        }
    return 0;
}