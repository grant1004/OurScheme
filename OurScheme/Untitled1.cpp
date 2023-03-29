# include <iostream> 
# include <vector> 

using namespace std ; 

int main()
{
  vector<int> i ; 
  i.push_back( 1 ) ; 
  i.push_back( 1 ) ; 
  i.push_back( 1 ) ; 
  i.push_back( 1 ) ; 
  i.push_back( 1 ) ; 
  i.push_back( 1 ) ; 
  i.push_back( 1 ) ; 
  // 1 * 7 
  
  vector<int> ii ; 
  ii.push_back( 2 ) ; 
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  ii.push_back( 2 ) ;
  
  ii.assign( i.begin(), i.end() ) ; 
  
  for ( int a = 0 ; a < ii.size(); a ++ ) 
    cout << ii.at( a ) << endl ; 
} // main() 
