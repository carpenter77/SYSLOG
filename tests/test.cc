
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

using namespace std;

int main(){
  std::string val("12");
  int v=boost::lexical_cast<int>(val);
  cout<<v<<endl;
 return 0;

}