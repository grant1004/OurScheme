# include <cctype>
# include <cstdlib>
# include <cstring>
# include <iostream>
# include <string>
# include <vector>
# include <sstream>
# include <iomanip>

using namespace std ;

bool gIsEOF = false ;  
bool gExit = false ; 
int gNowColumn = 0 ;
int gNowRow = 1 ; 
int gNumOfParen = 0 ;
bool gEndLine = false ;

#define NOT ! 

enum Type 
{
  LEFT_PAREN, // '('

  RIGHT_PAREN, // ')'

  LEFT_RIGHT_PAREN, // '()'
  
  SYMBOL, // other token

  INT, // '123', '+123', '-123'
  
  FLOAT, // '123.567', '123.', '.567', '+123.4', '-.123'

  STRING, // "string's (example)." (strings do not extend across lines)

  NIL, // 'nil' or '#f'

  T, // 't' or '#t'

  DOT, // '.' 

  QUOTE, // '\'' 單引號

  ADD, // '+'

  SUB, // '-'

  MULT, // '*'

  DIV, // '/'
  
  EMPTYPTR, // BuildTree() 空的   

  NONE 
}; // Type 

struct EXP {
  string token ;
  int column ;
  int row ; 
  Type type ; // SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN | RIGHT-PAREN
  EXP* next ;
  EXP* pre_next ;
  EXP* listPtr ;
  EXP* pre_listPtr ;

}; // struct EXP 

// 2023/02/25 超級大肥肥新增這些程式碼

EXP * root = NULL ;

/* PrintType( Type type ) 
* 依造傳進去的 type 印出對應的 type 名稱 
*/ 
string PrintType ( Type type )
{
  if ( type == LEFT_PAREN ) 
    return "LEFT_PAREN" ; // '('
  else if ( type == RIGHT_PAREN ) 
    return "RIGHT_PAREN" ; // ')'
  else if ( type == SYMBOL )
    return "SYMBOL" ;  // other token
  else if ( type == INT )
    return "INT" ; 
  else if ( type == FLOAT )
    return "FLOAT" ; 
  else if ( type == STRING )
    return "STRING" ; 
  else if ( type == NIL )
    return "NIL" ;
  else if ( type == T )
    return "T" ; 
  else if ( type == ADD )
    return "ADD" ; 
  else if ( type == SUB ) 
    return "SUB" ; 
  else if ( type == MULT )
    return "MULT" ; 
  else if ( type == DIV )
    return "DIV" ; 
  else if ( type == DOT )
    return "DOT" ; 
  else if ( type == QUOTE )
    return "QUOTE" ; 
  else if ( type == NONE ) 
    return "NONE" ; 

  return "ERROR TYPE" ; 
} // PrintType ( Type type ) 


/* CheckWhiteSpace(char ch) 
* 檢查 ch 是否為 white space 
*/
bool CheckWhiteSpace(char ch) // 判斷是否為white space, 如果是 return true, 不是 return false  
{

  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ) 
  {
    if ( gEndLine == true && ch == '\n' ) 
    { 
      // cout << "END LINE" << endl ; 
      gNowRow ++ ; 
      gNowColumn = 0 ; 
    } // if 
    return true ; 
  } // if 

  return false ;  
} // IsWhileSpace( char ch )

/* CheckDelimiter ( char ch )
* 檢查 ch 是否為 Delimiter 
* Delimiter : '('  ')'  '\''  WhiteSpace  
*/
bool CheckDelimiter ( char ch )
{
  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || 
       ch == '(' || ch == ')'  || ch == '\'' || ch == ';' ) 
  {
    return true ; 
  } // else if 

  return false ; 
} // IsDelimiter ( char ch )

/* IsStringStart ( char ch )
* 檢查是否為 '\"'  
*/
bool IsStringStart ( char ch )
{
  if ( ch == '\"' )
    return true ; 
  return false ; 
} // IsStringStart ( char ch )

/* IsComment ( char ch )
*  檢查是否為 ';' 
*/
bool IsComment ( char ch )
{
  if ( ch == ';' )
    return true ;
  return false ; 
} // IsComment ( char ch )

/* IsInt( string token )
*  INT : '3', '+3', '-3'
*/ 
bool IsInt( string token ) // 開頭可以是0嗎???
{
	
  bool cont = false ; // continue ; 
  for ( int i = 0 ; i < token.size() ; i ++ ) 
  {
    if ( i == 0 && ( token[i] == '+' || token[i] == '-' ) )
    {
      cont = true ; 
    } // if 
    else if ( token[i] == '0' || token[i] == '1' || token[i] == '2' || token[i] == '3' || token[i] == '4' || 
              token[i] == '5' || token[i] == '6' || token[i] == '7' || token[i] == '8' || token[i] == '9' )
    {
      cont = true ; 
    } // if
    else  
    {
      return false ; 
    } // else 

  } // for 

  if (token.empty())
    return false ;

  return true ; 
} // IsInt(string str) 

/* IsFloat( string token )
*  FLOAT : '3.25', '.25', '+.25', '-.25', '+3.'
*/
bool IsFloat( string token ) // 開頭可以是0嗎??
{
  bool cont = false ; // continue ; 
  int numOfDot = 0 ; 

  for ( int i = 0 ; i < token.size() ; i ++ ) 
  {
    if (token[i] == '.')
    {
      numOfDot ++ ; 
    } // if 

    if ( ( token[0] == '+' || token[0] == '-' || token[0] == '.' ) && i == 0 ) 
    {
      cont = true ; 
    } // if 
    else if ( token[i] == '.' || token[i] == '0' || token[i] == '1' || token[i] == '2' || token[i] == '3' || token[i] == '4' ||
              token[i] == '5' || token[i] == '6' || token[i] == '7' || token[i] == '8' || token[i] == '9' )
    {
      cont = true ; 
    } // if
    else  
    {
      return false ; 
    } // else 

  } // for 

  if ( numOfDot > 1 )
  {
    return false ; 
  } // if 

  if (token.empty())
    return false ; 

  return true ; 
} // IsFloat

/* IsSymbol( string token ) 
*  只要不是 INT || FLOAT || '(' || ')' || DOT || STRING || NIL || T || QUOTE 
*  剩下都當作SYMBOL
*/
bool IsSymbol( string token )
{
  if ( token == " " ) 
  {
    cout << "Is White Space" ; 
    return false ; 
  } // if 
  else if ( token == "\0" ) 
  {
    cout << "Is NULL" ; 
    return false ; 
  } // else if
  else if (token.empty())
  {
    return false ; 
  } // else if 
  else if ( NOT IsInt(token) && NOT IsFloat(token) )
  {
    return true ; 
  } // else if 

  return false ; 
} // IsSymbol 

/* IsString ( string token ) 
*  開頭是否是 "  
*/
bool IsString ( string token )
{
  if ( token[0] == '\"' ) 
    return true ; 
  return false ; 
} // IsString 


/* IsDelimiter( string token, Type & type ) 
*  檢查是Delimiter 並設定他的type 
*  ( --> LEFT_PAREN 
*  ) --> RIGHT_PAREN
*  . --> DOT 
*  ' --> QUOTE 
*  #f nil --> NIL 
*  #t t --> T 
*/ 
bool IsDelimiter( string token, Type & type )
{

  if ( token == "(" )
  {
    type = LEFT_PAREN ; 
    return true ;
  } // if 
  else if ( token == ")" )
  {
    type = RIGHT_PAREN ; 
    return true ;
  } // else if 
  else if ( token == "." )
  {
    type = DOT ; 
    return true ;
  } // else if 
  else if ( token == "#f" || token == "nil" || token == "()" ) 
  {
    type = NIL; 
    return true ;
  } // else if 
  else if ( token == "t" || token == "#t" )
  {
    type = T ; 
    return true ; 
  } // else if 
  else if ( token == "\'" )
  {
    type = QUOTE ; 
    return true ; 
  } // else if 

	return false ; 
} // IsDelimiter( char ch )

/* IsEOF ( char ch ) 
*  是否為 EOF 
*/
bool IsEOF ( char ch )
{
  if ( ch == -1 || ch == EOF )
    return true ;
  return false ; 
} // IsEOF

/* IdentifyType ( string token ) 
*  分辨 token 是什麼 type 
*/ 
Type IdentifyType ( string token )
{
  Type type = NONE ; 

  if ( token == "\0" )
  {
    return NONE ; 
  } // if 
  else if ( IsDelimiter ( token, type ) )
  {
    return type ; 
  } // else if 
  else if ( IsString ( token ) )
  {
    return STRING ; 
  } // else if 
  else if ( IsInt ( token ) )
  {
    return INT ; 
  } // else if 
  else if ( IsFloat ( token ) )
  {
    return FLOAT ; 
  } // else if 
  else if ( IsSymbol ( token ) )
  {
    return SYMBOL ;
  } // else if 

} // IdentifyType ( string token )

/* GetString ( ) 
*  把整個 string 讀出來，直到 ('\n') 或是另一個 ('\"')
*  如果讀到 '\n' 沒有讀到 '\"' 那就是 error  <----------------------------------------- 這裡還沒完成喔 
*/
string GetString ( )
{
  string str = "\0" ; 
  str += '\"' ;
  char ch = '\0';
  bool valid = true ; 
  while ( valid )
  {
    ch = getchar() ; 
    gNowColumn ++ ; 
    //cout << ch ; 
    if ( ch == '\\' ) // 跳脫字元 \"
    {
      // 遇到跳脫字元要把 \ 刪掉，並留下下一個字元 
      // EX:  '\"' --> '"', '\\"' --> '\"' 
      char peek = cin.peek() ; 
      ch = getchar() ; 
      gNowColumn ++ ; 
      if ( ch == 'n' )
      {
        ch = '\n' ; 
      } // if 
      else if ( ch == 't' )
      {
        ch = '\t' ; 
      } // else if 
      valid = true ; 
    } // if 
    else if ( ch == '\"' )
    {
      valid = false ; 
    } // else if 
    else if ( ch == '\n' )
    {
      valid = false ; 
    } // else if  
    str += ch ;
  } // while 

  return str ; 
} // GetString ()

/* SkipComment ( ) 
* 把 ; 後面的全部讀掉 
*/ 
void SkipComment ( )
{
  char ch = '\0' ;
  while ( ch != '\n' )
  {
    ch = getchar( ) ;
  } // while 
  
  if ( gEndLine == true ) 
  {
    gNowRow ++ ; 
  } // if 
  
  gNowColumn = 0 ; 
} // SkipCommnet() 

/* GetFirstChar ( ) 
*  跳過空白讀到第一個字元
*/ 
char GetFirstChar ( ) // skip white space to get First char 
{
  char ch = getchar() ; 
  gNowColumn ++ ;     
  while ( CheckWhiteSpace(ch) == true )
  {
    ch = getchar() ;
    gNowColumn ++ ;     
  } // while 

  if ( ch == EOF ) 
  {
    return EOF ; 
  } // if 
  
  return ch ; 
} // GetNextChar()
 
/* GetToken ( ) 
* 切下token 並判斷牠的 type  
*/ 
EXP GetToken ( ) 
{
  // (1 . (2 . (3 . nil)))
  // 切割遇到的第一個token 並判斷牠的type 

  EXP gg ;

  char ch = GetFirstChar() ;
  char peek = '\0' ;  
  bool valid = true ; // true : 不是 delimiter string EOF， false : 代表可能是 delimiter string EOF 
  
  bool skipComment = false ; 
  
  while ( NOT skipComment ) // 當還沒跳完全部註解，就進去while，如果全部跳過了才往下走
  {
    if ( IsComment ( ch ) )
    {
      SkipComment() ; 
      ch = GetFirstChar() ;
      valid = true ; 
    } // if 
    else 
    {
      skipComment = true ; 
    } // else 
  } // while 
  
  gg.column = gNowColumn ; 
  gg.row = gNowRow ; 

  if ( CheckDelimiter ( ch ) == true )
  {
    gg.token += ch ; 
    valid = false ; 
  } // if 
  else if ( IsStringStart ( ch ) )
  {
    gg.token = GetString() ; 
    valid = false ; 
  } // else if 
  else if ( IsEOF ( ch ) )
  {
    gg.token = "\0" ;
    valid = false ; 
  } // else if 

  while ( valid )  //(1 . (2 . (3 . nil)))
  { 
    gg.token += ch ; 
    peek = cin.peek() ; 
    if ( CheckDelimiter ( peek ) )
    {
      valid = false ; 
    } // if
    
    if ( valid )
    {
      ch = getchar() ; 
      gNowColumn ++ ; 
    } // if 

  } // while 


  gg.type = IdentifyType ( gg.token ) ; 

  if ( gg.token == "\0" && gg.type == NONE ) // EOF 
  {
    gIsEOF = true ;  
  } // if 

  return gg ; 
  
} // getToken() 

//==================================================

bool IsATOM( EXP * temp )
{ 
  if ( temp->type == SYMBOL || temp->type == INT  || temp->type == FLOAT  || temp->type == STRING  || temp->type == NIL || temp->type == T || temp->type == LEFT_RIGHT_PAREN )
    return true ;
  else
    return false ;
    
} // IsATOM()

int gnum = 0 ;
bool dotExist = false ;
bool S_EXP( EXP * &temp ) {

/*
<S-exp> ::= <ATOM> 
            | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
            | QUOTE <S-exp>
            
<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
            | NIL | T | LEFT-PAREN RIGHT-PAREN
ATOM 1
LEFT-PAREN/EMPTYPTR 2
RIGHT-PAREN 3
QUOTE 4
DOT 5
*/

/*

(1(a.b)()(123).(234 . 678) .( 123 ) ) ERROR
( 1 ( a . b . c ) ( 123 . 456 .678 ) . ( abc a .c ) ) ERROR
*/

  if ( temp != NULL )
    cout << endl << "temp->token: " << temp->token << endl ;
  cout << "gnum: " << gnum << endl ; 
//  system("pause") ;
  
  
  if ( temp == NULL && gnum == 2 ) {
    cout << "aa" << endl ;
    gnum = 8888 ;
    return true ;
  }
  else if ( temp->type == RIGHT_PAREN && ( gnum == 1 || gnum == 2 ) ) {
    cout << "bb" << endl ;
    dotExist = false ;
    gnum = 2 ; // list
    while( temp->type != LEFT_PAREN ) { // 走回去 
      temp = temp->pre_next ; 
    }
    temp = temp->pre_listPtr->next ;
    S_EXP( temp ) ;
    
  }
  else if ( temp->type == RIGHT_PAREN ) {
    cout << "cc" << endl ;
    gnum = -1 ;
    return false ;
  }
  else if ( IsATOM(temp) == true && gnum == 0 ) {
    cout << "dd" << endl ;
    gnum = 1 ;
    return true ;
  }
  else if ( IsATOM(temp) == true ) {
    cout << "ee" << endl ;
    gnum = 1 ;
    temp = temp->next ;
    S_EXP( temp ) ;
  }
  else if ( temp->type == EMPTYPTR ) {
    cout << "ff" << endl ;
    gnum = 2 ;
    dotExist = false ;
    temp = temp->listPtr->next ;
    S_EXP( temp ) ;

  }
  else if ( temp->type == DOT &&  ( temp->pre_next->type == EMPTYPTR || gnum == 1 ) && dotExist == false ) { 
    EXP * forward = temp->pre_next ;
    while( forward != NULL && forward->type != DOT ){
      forward = forward->pre_next ;
    }
    if ( forward == NULL ) {
      cout << "gg" << endl ;
      gnum = 5 ;
      dotExist = true ;
      temp = temp->next ;
      S_EXP( temp ) ;
    }
    else {
      return false ;
    }

  }
  else if ( temp->type == DOT ) {
    cout << "hh" << endl ;
    gnum = -1 ;
    return false ;
  }
  else {
    cout << "ii" << endl ;
    gnum = -1 ;
    return false ;
  }
    
} // S_EXP()

int paren_num = 0 ; 
int tab = 2 ; 
void printTab( int numOfTab ) 
{
  for ( int i = 0 ; i < numOfTab ; i ++ ) 
    cout << "  " ;  
} // printTab( int numOfTab ) 

void PrintS_EXP( vector<EXP> s_exp ) 
{

  for ( int i = 0 ; i < s_exp.size() ; i ++ ) 
    cout << s_exp.at( i ).token << " Column : "<< s_exp.at( i ).column << " Row : "<< s_exp.at( i ).row << endl ; 

} // PrintS_EXP( EXP * sExp ) 



EXP *getValue( vector<EXP> vec, int &i ) {
  EXP * ptr = new EXP() ;
  ptr -> token = vec.at(i).token ; 
  ptr -> type = vec.at(i).type ;
  ptr -> column = vec.at(i).column ;
  ptr -> row = vec.at(i).row ;
  ptr -> next = NULL ;
  ptr -> pre_next = NULL ;
  ptr -> listPtr = NULL ;
  ptr -> pre_listPtr = NULL ;
  i++ ;
  return ptr ;
  
} // getValue()

void preOrderTraversal(EXP* focusNode) {
  if (focusNode != NULL) {
    std::cout << focusNode->token << " ";
    preOrderTraversal(focusNode->next);
    preOrderTraversal(focusNode->listPtr);
  }
}


void buildTree( vector<EXP> s_exp, int &i ) {
  EXP * temp = NULL ;
   // (1(2)
  while ( i < s_exp.size() ) {
//    cout << endl << "s_exp.at(i).token: "       << s_exp.at(i).token << endl ;
//    if( temp != NULL )
//      cout << "temp->token: " << temp->token << endl ;
//    preOrderTraversal(root) ;  
      
    if ( s_exp.at(i).type == RIGHT_PAREN ) {
      temp->next = getValue(s_exp, i) ; 
      temp->next->pre_next = temp ;
      
      
      while( temp->type != LEFT_PAREN ) {
        temp = temp->pre_next ; 
      }
      temp = temp->pre_listPtr ;
      
    } // if
    else if ( s_exp.at(i).type == LEFT_PAREN ) {
      if ( root == NULL ) {
        root = new EXP() ;
        root->type = EMPTYPTR ;
        root->token = "XXEMPTYXX" ;
        root->next = NULL ;
        root->listPtr = getValue(s_exp, i) ;
        root->listPtr->pre_listPtr = root ; 
        temp = root->listPtr ;
      }
      else {
        temp->next = new EXP() ;
        temp->next->type = EMPTYPTR ;
        temp->next->token = "XXEMPTYXX" ;
        temp->next->pre_next = temp ;
        temp->next->next = NULL ;
        temp = temp->next ;
        
        temp->listPtr = getValue(s_exp, i) ;
        temp->listPtr->pre_listPtr = temp ;
        temp = temp->listPtr ;
               
      }
//      cout << "s_exp.at(i).token: " << s_exp.at(i).token << endl ;

    }
    else {
      if ( root == NULL ) {
        root = getValue(s_exp, i) ; 
        temp = root ;
      }
      else {
        temp->next = getValue(s_exp, i) ; 
        temp->next->pre_next = temp ;
        temp = temp->next ;        
      }

    }     
    
//    system("pause") ;

  }
   
//  cout << "end" << endl ;
//  if( temp != NULL )
//      cout << "temp->token: " << temp->token << endl ;
    
  
}

string rounding( string str ) { // 小數點後四位+四捨五入 

  stringstream ss ;
  ss << fixed << setprecision( 3 ) << atof( str.c_str() ) ;
  
  return ss.str() ;
  
} // rounding()


void fixToken( vector<EXP> & s_exp ) { // () 沒處理 
/*
float小數點三位
四捨五入
t or #t 系統印出來的是 #t
nil or () or #f 系統印出來的是nil
()
*/ 
  int i = 0 ;
  while ( i < s_exp.size() ) {
    if ( s_exp.at(i).type == FLOAT ) {
      s_exp.at(i).token = rounding( s_exp.at(i).token ) ;
    }
    else if ( s_exp.at(i).token == "t" ) {
      s_exp.at(i).token = "#t" ;
    }
    else if ( s_exp.at(i).token == "#f" ) {
      s_exp.at(i).token = "nil" ;
    }
    else if ( s_exp.at(i).type == LEFT_PAREN && i+1 < s_exp.size() && s_exp.at(i+1).type == RIGHT_PAREN ) {
      s_exp.at(i).token = "()" ;
      s_exp.at(i).type = LEFT_RIGHT_PAREN ;
      s_exp.erase( s_exp.begin()+i+1 ) ;
      
    }
    i++ ;
  }
} // fixToken()

void test(vector<EXP> s_exp){
  for ( int i = 0; i < s_exp.size(); i++){
    cout << s_exp[i].token << "," << PrintType(s_exp[i].type) << endl ;
  }
}

int main() { // +3 -> 3

  bool readEXP = true ;
  int parnum = 0 ; 
  int i = 0 ;
  bool syntaxIsTrue ;
  cout << "Welcome to OurScheme!" << endl ;
  
  bool ALL_EXP_DONE = false ; 
  
  vector<EXP> s_exp ;
  EXP nextToken ;
  
  
  while ( NOT ALL_EXP_DONE ) 
  { 
 
    readEXP = true ;
    parnum = 0 ; 
    gNowColumn = 0 ; 
    gNowRow = 1 ;  
    s_exp.clear() ; 
    gEndLine = false ; // false : 不要計算換行 ; true : 開始計算換行 
    while ( readEXP )
    {

      nextToken = GetToken() ; 
      gEndLine = true ; 
      s_exp.push_back( nextToken ) ;  
      
      
      if ( nextToken.type == LEFT_PAREN ) 
      { 
        parnum ++ ; 
        // cout << "Left Paren :" << parnum << endl ; 
      } // if       
      else if ( nextToken.type == RIGHT_PAREN ) 
      {
        parnum -- ; 
        // cout << "Right Paren :" << parnum << endl ; 
      } // else if 
      
      if ( parnum == 0 ) // 這條指令結束 
      {
        
        if ( nextToken.type == QUOTE )
        {
          cout << "This is QUOTE" << endl ; 
          readEXP = true ;
        } // else if
        else if ( nextToken.type != NONE ) // 還沒讀到 EOF，還有其他指令還沒讀
        {
          
          PrintS_EXP( s_exp ) ;     
          fixToken(s_exp) ; // 更正token 
      //    test(s_exp) ;
      //    system("pause") ;
          delete root ;  
          root = NULL ;
          i = 0 ;
          buildTree( s_exp, i ) ;
          cout << endl << "============ Tree ========" << endl ;
          preOrderTraversal(root) ;
          cout << endl << "check syntax START" << endl ;
          
          gnum = 0 ;
          dotExist = false ;
          bool isTrue = S_EXP( root ) ;
          if ( isTrue == true ) {
            cout << "Correct!" << endl ;
          }
          else{
            cout << "ERROR!" << endl ;
          }
          /////IRIS
          cout << endl << "EXP DONE" << endl << "================================" << endl ; 
          readEXP = false ;
        } // if 
        else // nextToken == NONE 代表讀到 EOF 了，沒有任何指令了 
        {
          cout << ">>> ALL s_exp Read Done " << endl ; 
          readEXP = false ;
        } // else 
         
      } // if 
      else if ( parnum < 0 )
      {
        cout << endl << "ERROR > 多了一個右括號" << endl ;
        readEXP = false ; 
      } // else if 
      
      
    } // while ( readEXP )
    

    
                                                                                                                         
    if ( nextToken.type == NONE ) // 讀到 EOF
    {
      ALL_EXP_DONE = true ;
    } // if 沒有 (exit) 
    else if ( s_exp.at( 0 ).token == "("  && s_exp.at( 0 ).token == "exit" && s_exp.at( 0 ).token == ")" )
    {
      ALL_EXP_DONE = true ;
    } // else if 
    
  } // while ( ALL_EXP ) 
    
} // main()     
