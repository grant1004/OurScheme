#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std ; 

int main() {
  map< string, vector<int>* > msymbolMap ;
  
  vector<int> aa ; 
  
  aa.push_back( 1 ) ; 
  aa.push_back( 1 ) ; 
  aa.push_back( 1 ) ; 
  aa.push_back( 1 ) ; 
  aa.push_back( 1 ) ; 
  aa.push_back( 1 ) ; 
  
  msymbolMap[ "aa" ] = &aa ; 
  
  
  vector<int> * bb = msymbolMap[ "aa" ] ;
  
  for ( int i = 0 ; i < bb->size() ; i ++ )
    cout << bb->at( i ) << endl ; 
}
