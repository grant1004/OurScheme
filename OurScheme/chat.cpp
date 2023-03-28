#include <iostream>
#include <vector>
#include <string>
#include <stack>

using namespace std;

// ( a b ) -> ( a . ( b . nil ) )
// ( a b c ) -> ( a . ( b c ) ) -> ( a . ( b . ( c ) ) ) ->  ( a . ( b . ( c . nil ) ) )
// ( ( a b c ) d . ( a c ) ) 
// ( ( a . ( b c ) ) d . ( a c ) )   
//  -> ( ( a . ( b . ( c . nil ) ) ) . ( d . ( a . ( c . nil ) ) ) )   
// ( a b . c )   ->  ( a . ( b . c ) ) 

vector<string> format( vector<string> input )
{
  vector<string> out ;
  
  int parNum = 0 ; 
    
  int now = 0 ; 
  
  while( now < input.size() )  
  {
    
    if( input.at( now ) == "(" )
    {
      parNum ++ ; 
      now++ ;
    } // if 
    else if( input.at( now ) == ")" )
    {
      parNum -- ; 
      
    } // if 
    else 
    {
      
    } // else 
    
    
  } // while() 
  
  return out ; 
} // fromat() 

string vec2str( vector<string> input )
{
  string out ; 
  for ( int i = 0 ; i < input.size() ; i ++ )
  {
    out += input.at( i ) ; 
    out += " " ; 
  } // for 
  
  return out ; 
} // vec2str 

int main() {
    vector<string> input;
    // (a b d)
    input.push_back("(");
    input.push_back("a");
    input.push_back("b");
    input.push_back("d");
    input.push_back(")");
    
    
    
    cout << vec2str( input ); 
    
    
    
    return 0;
}
