# include <cctype>
# include <cstdlib>
# include <cstring>
# include <iostream>
# include <string>
# include <vector>
# include <sstream>
# include <iomanip>
# include <exception> 
# include <sstream>
# include <stdio.h>
# include <stdlib.h>


using namespace std ;

bool gIsEOF = false ;  
bool gExit = false ; 
int gNowColumn = 0 ;
int gNowRow = 1 ; 
int gNumOfParen = 0 ;
bool gEndLine = false ;

# define NOT ! 

enum Type 
{
  LEFT_PAREN, // '('

  RIGHT_PAREN, // ')'
  
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

  NONE, // EOF
  
  ERROR // ERROR
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
  int dotCnt ;
  int quoteCnt ;

}; // struct EXP 

  /*
  NOTHING,           // 沒有ERROR 
  
  SYNERR_ATOM_PAR,   
  // 語法錯誤 ( 下一個token應該要接 ATOM 或是 '(' )   
  // ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 1 is >>)<<  
  
  SYNERR_RIGHTPAREN, 
  // 語法錯誤 (  應該要接右括號卻沒有右括號  ) 
  // ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<< 
  
  STRERR,            
  // String Error 少一個雙引號 '\"' 
  // ERROR (no closing quote) : END-OF-LINE encountered at line 1, column 19   
  */

enum ExceptionType 
{ 
  NOTHING,  
  
  SYNERR_ATOM_PAR,   
  
  SYNERR_RIGHTPAREN, 
  
  STRERR      
  
}; // Enum ExceptionType 

class MyException
{  
  string mErrMsg  ;
   
  public : 
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 
  
  MyException()
  {
    mErrMsg = "This is an empty Exception.\n" ; 
  } // MyException()
  
  MyException( ExceptionType ErrType, int line, int column ) 
  {
    stringstream ss ;
    if ( ErrType == STRERR ) // String Error
    {
      ss << "ERROR (no closing quote) : "
         << "END-OF-LINE encountered at line " 
         << line
         << ", column "
         << column ; 
      mErrMsg = ss.str() ;   
    } // if 
  } // MyException() 
  
  MyException( ExceptionType ErrType, EXP token ) 
  {
    stringstream ss ;
    if ( ErrType == SYNERR_ATOM_PAR ) // ( 下一個token應該要接 ATOM 或是 '(' )
    {
      ss << "ERROR (unexpected token) : "
         << "atom or '(' expected when token at "
         << "Line " << token.row  
         << " Column " << token.column << " is >>" << token.token << "<<" ;  
      mErrMsg = ss.str() ;
    } // if 
    else if ( ErrType == SYNERR_RIGHTPAREN ) // (  應該要接右括號卻沒有右括號  )
    {
      ss << "ERROR (unexpected token) : "
         << "')' expected when token at "
         << "Line " << token.row  
         << " Column " << token.column << " is >>" << token.token << "<<" ;  
      mErrMsg = ss.str() ;
    } // else if
  } // MyException()
} ; 




EXP * gRoot = NULL ;
EXP * gHead = NULL ;
/* 
  PrintType( Type type ) 
  依造傳進去的 type 印出對應的 type 名稱 
*/ 
string PrintType( Type type )
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
} // PrintType() 


/* 
  CheckWhiteSpace(char ch) 
  * 檢查 ch 是否為 white space 
*/

bool CheckWhiteSpace( char ch )  
{

  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ) 
  {
    if ( gEndLine == true && ch == '\n' ) 
    { 
      // cout << "END LINE" << endl ; 
      // gNowRow ++ ; 
      gNowColumn = 0 ; 
    } // if 
    
    return true ; 
  } // if 

  return false ;  
} // CheckWhiteSpace()

/* 
  CheckDelimiter ( char ch )
  * 檢查 ch 是否為 Delimiter 
  * Delimiter : '('  ')'  '\''  WhiteSpace  
*/

bool CheckDelimiter( char ch )
{
  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || 
       ch == '(' || ch == ')'  || ch == '\'' || ch == ';' || ch == '\"' ) 
  {
    return true ; 
  } // if

  return false ; 
} // CheckDelimiter()

/* 
  IsStringStart ( char ch )
  * 檢查是否為 '\"'  
*/

bool IsStringStart( char ch )
{
  if ( ch == '\"' )
    return true ; 
    
  return false ; 
} // IsStringStart()

/* 
  IsComment ( char ch )
  *  檢查是否為 ';' 
*/

bool IsComment( char ch )
{
  if ( ch == ';' )
    return true ;
    
  return false ; 
} // IsComment()

/* 
  IsInt( string token )
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
    } // else if
    else  
    {
      return false ; 
    } // else 

  } // for 

  if ( token.empty() )
    return false ;
  else if ( token[0] == '+' && token.size() == 1  )
    return false ;
  else if ( token[0] == '-' && token.size() == 1  )
    return false ;

  return true ; 
} // IsInt() 

/* 
  IsFloat( string token )
  *  FLOAT : '3.25', '.25', '+.25', '-.25', '+3.'
*/

bool IsFloat( string token ) // 開頭可以是0嗎??
{
  bool cont = false ; // continue ; 
  int numOfDot = 0 ; 

  for ( int i = 0 ; i < token.size() ; i ++ ) 
  {
    if ( token[i] == '.' )
    {
      numOfDot ++ ; 
    } // if 

    if ( ( token[0] == '+' || token[0] == '-' || token[0] == '.' ) && i == 0 ) 
    { 
      cont = true ; 
    } // if 
    else if ( token[i] == '.' || token[i] == '0' || token[i] == '1' || 
              token[i] == '2' || token[i] == '3' || token[i] == '4' ||
              token[i] == '5' || token[i] == '6' || token[i] == '7' || 
              token[i] == '8' || token[i] == '9' )
    {
      cont = true ; 
    } // else if
    else  
    {
      return false ; 
    } // else 

  } // for 

  if ( numOfDot > 1 )
  {
    return false ; 
  } // if 
  
  if ( token[0] == '+' && token[1] == '.' && token.size() == 2  )
    return false ;
  else if ( token[0] == '-' && token[1] == '.' && token.size() == 2  )
    return false ; 
  else if ( token[0] == '+' && token.size() == 1  )
    return false ;
  else if ( token[0] == '-' && token.size() == 1  )
    return false ;
  
  if ( token.empty() )
    return false ; 

  return true ; 
} // IsFloat()

/* 
  IsSymbol( string token )   
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
  else if ( token.empty() )
  {
    return false ; 
  } // else if 
  else if ( NOT IsInt( token ) && NOT IsFloat( token ) )
  {
    return true ; 
  } // else if 

  return false ; 
} // IsSymbol()

/* 
  IsString ( string token ) 
*  開頭是否是 "  "
*/

bool IsString( string token )
{
  if ( token[0] == '\"' ) 
    return true ; 
  return false ; 
} // IsString()


/* 
  IsDelimiter( string token, Type & type ) 
*  檢查是Delimiter 並設定他的type 
*  ( --> LEFT_PAREN 
*  ) --> RIGHT_PAREN
*  . --> DOT 
*  ' --> QUOTE '
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
} // IsDelimiter()

/*
  IsEOF ( char ch ) 
*  是否為 EOF 
*/

bool IsEOF( char ch )
{
  if ( ch == -1 || ch == EOF )
    return true ;
  return false ; 
} // IsEOF()

/* 
  IdentifyType ( string token ) 
*  分辨 token 是什麼 type 
*/ 

Type IdentifyType( string token )
{
  Type type = NONE ; 

  if ( token == "\0" )
  {
    return NONE ; 
  } // if 
  else if ( IsDelimiter( token, type ) )
  {
    return type ; 
  } // else if 
  else if ( IsString( token ) )
  {
    return STRING ; 
  } // else if 
  else if ( IsInt( token ) )
  {
    return INT ; 
  } // else if 
  else if ( IsFloat( token ) )
  {
    return FLOAT ; 
  } // else if 
  else if ( IsSymbol( token ) )
  {
    return SYMBOL ;
  } // else if 
  
  return ERROR ; 
} // IdentifyType()

/* 
  GetString ( ) 
*  把整個 string 讀出來 直到 ('\n') 或是另一個 ('\"')
*  如果讀到 '\n' 沒有讀到 '\"' 那就是 error  
*/

string GetString()
{
  string str = "\0" ; 
  str += '\"' ;
  char ch = '\0';
  bool valid = true ; 
  while ( valid )
  {
    ch = getchar() ; 
    gNowColumn ++ ; 
    // cout << ch ; 
    if ( ch == '\\' ) // 跳脫字元 \"
    {
      // 遇到跳脫字元要把 \ 刪掉，並留下下一個字元 
      // EX:  '\"' --> '"', '\\"' --> '\"' 
      char peek = cin.peek() ; 
      if ( peek == 'n' || peek == 't' || peek == '\'' || peek == '\"' || peek == '\\' )
      {
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

      } // if 
      
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
    else if ( ch == -1 )
    {
      valid = false ; 
    } // else if
    
    str += ch ;
  } // while 
  
  if ( ch == '\n' || ch == -1 )
  {
    throw MyException( STRERR, gNowRow, gNowColumn ) ;   
  } // if  

  return str ; 
} // GetString()

/* 
  SkipComment ( ) 
* 把 ; 後面的全部讀掉 
*/ 

void SkipComment()
{
  char ch = '\0' ;
  while ( ch != '\n' )
  {
    ch = getchar( ) ;
  } // while 
  
  /*
  if ( gEndLine == true ) 
  {
    gNowRow ++ ; 
  } // if 
  */ 
  gNowColumn = 0 ; 
} // SkipComment() 

/* 
  GetFirstChar ( ) 
*  跳過空白讀到第一個字元
*/ 

char GetFirstChar() // skip white space to get First char 
{
  char ch = getchar() ; 
  if ( ch == '\n' ) 
  {
    gNowColumn = 0 ;     
  } // if 
  else 
  {
    gNowColumn ++ ;     
  } // else 
  
  while ( CheckWhiteSpace( ch ) == true )
  {
    ch = getchar() ;
    if ( ch == '\n' )
    {
      gNowColumn =  0 ; 
    } // if 
    else 
    {
      gNowColumn ++ ;     
    } // else 
    
  } // while 

  if ( ch == EOF ) 
  {
    return EOF ; 
  } // if 
  
  return ch ; 
} // GetFirstChar()
 
/* 
  GetToken ( ) 
* 切下token 並判斷牠的 type  
*/ 

EXP GetToken() 
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
    if ( IsComment( ch ) )
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

  
  if ( IsStringStart( ch ) )
  {
    gg.token = GetString() ; 
    valid = false ; 
  } // if
  else if ( CheckDelimiter( ch ) == true )
  {
    gg.token += ch ; 
    valid = false ; 
  } // if 
  else if ( IsEOF( ch ) )
  {
    gg.token = "\0" ;
    valid = false ; 
  } // else if 

  while ( valid )  // (1 . (2 . (3 . nil)))
  { 
    gg.token += ch ; 
    peek = cin.peek() ; 
    if ( CheckDelimiter( peek ) )
    {
      valid = false ; 
    } // if
    
    if ( valid )
    {
      ch = getchar() ; 
      gNowColumn ++ ; 
    } // if 

  } // while 


  gg.type = IdentifyType( gg.token ) ; 

  if ( gg.token == "\0" && gg.type == NONE ) // EOF 
  {
    gIsEOF = true ;  
  } // if 

  return gg ; 
  
} // GetToken() 


bool IsATOM( EXP * temp )
{ 
  if ( temp->type == SYMBOL || temp->type == INT  || temp->type == FLOAT  || 
       temp->type == STRING  || temp->type == NIL || temp->type == T )
    return true ;
  else
    return false ;
    
} // IsATOM()

int gnum = 0 ;
int gAfterDotCnt = 0 ;
int gAfterQuoteCnt = 0 ;  
bool S_EXP( EXP * temp ) { // new 

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

(1(a.b)()(123).(234 . 678) .( 123 ) ) ERROR
( 1 ( a . b . c ) ( 123 . 456 .678 ) . ( abc a .c ) ) ERROR
( 1 . 2 4 5 6 ) 

!!!!!!input: . 1 的gettoken??
*/


// 每個 return false 是什麼意思呢 ??  

//  if ( temp != NULL )
//    cout << endl << "temp->token: " << temp->token << endl ;
//  cout << "gnum: " << gnum << endl ; 
//  system("pause") ;
  
  
  if ( temp == NULL && gnum == 2 ) {
    gnum = 8888 ;
    return true ;
  } // if
  else if ( temp == NULL && gRoot->type == QUOTE ) { // new
    return true ;
  } // else if
  else if ( temp->type == RIGHT_PAREN && ( gnum == 1 || gnum == 2 ) ) { 
    //  cout << "bb" << endl ;
    gnum = 2 ; // list
    while ( temp->type != LEFT_PAREN ) { // 走回去 
      temp = temp->pre_next ; 
    } // while
    
    temp = temp->pre_listPtr->next ;
    S_EXP( temp ) ;
    
  } // else if 
  else if ( temp->type == RIGHT_PAREN ) {
    //  cout << "cc" << endl ;
    gnum = -1 ;
    throw MyException( SYNERR_ATOM_PAR, *temp ) ; 
    return false ; // temp 應該是s_EXP 
  } // else if
  else if ( IsATOM( temp ) == true && gnum == 0 && temp->next == NULL && temp->listPtr == NULL ) {
    //  cout << "dd" << endl ;
    gnum = 1 ;
    return true ;
  } // else if
  else if ( IsATOM( temp ) == true ) { 
    //  cout << "gg" << endl ;
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
      //  cout << "ee" << endl ;
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      throw MyException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // 應該是右括號 ex: . 3 3 
    } // if
    else if ( temp->pre_next != NULL && temp->pre_next->quoteCnt != 0 ) {
      //  cout << "ff" << endl ;
      temp->quoteCnt = temp->pre_next->quoteCnt+1 ;
      throw MyException( SYNERR_ATOM_PAR, *temp ) ;
      return false ; // 沒有這種ERROR 寫錯了 
    } // else if
    else {
      if ( gnum == 5 ) { // 前面是DOT 
        //  cout << "hh" << endl ;
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
      } // if
      else if ( gnum == 4 ) { // QUOTE
        //  cout << "ii" << endl ;
        gAfterQuoteCnt++ ;
        temp->quoteCnt = gAfterQuoteCnt ;
      } // else if
      //  cout << "jj" << endl ;
      gnum = 1 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    } // else 

  } // else if
  else if ( temp->type == EMPTYPTR ) { 
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      throw MyException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // 應該是右括號 ex: . (1) (1)  
    } // if
    else if ( temp->pre_next != NULL && temp->pre_next->quoteCnt != 0 ) {
      temp->quoteCnt = temp->pre_next->quoteCnt+1 ;
      throw MyException( SYNERR_ATOM_PAR, *temp ) ;
      return false ; // 沒有這種ERROR 寫錯了 
    } // else if
    else {
      if ( gnum == 5 ) { // DOT
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
        
      } // if
      else if ( gnum == 4 ) { // QUOTE
        gAfterQuoteCnt++ ;
        temp->quoteCnt = gAfterQuoteCnt ;
      } // else if
      
      gnum = 2 ;
      gAfterDotCnt = 0 ;
      gAfterQuoteCnt = 0 ;
      temp = temp->listPtr->next ;
      S_EXP( temp ) ;
    } // else

  } // else if
  else if ( gnum == 0 && temp->type == DOT ) {
    throw MyException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // ex: .
  } // else if
  else if ( temp->type == DOT &&  
            ( ( temp->pre_next != NULL && temp->pre_next->type == EMPTYPTR ) || gnum == 1 ) && 
            gnum != 5 ) { 
    EXP * forward = temp->pre_next ;
    while ( forward != NULL && forward->type != DOT ) {
      forward = forward->pre_next ;
    } // while
    
    if ( forward == NULL ) {
      gnum = 5 ;
      gAfterDotCnt = 0 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    } // if
    else {
      throw MyException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // ex: (1 . 3 . 3 ) // 出現第二個.了 
    } // else 

  } // else if
  else if ( temp->type == DOT ) {  
    gnum = -1 ;
    throw MyException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // 忘記這是甚麼ERROR了先放著 
  } // else if
  else if ( temp->type == QUOTE ) { 

    gAfterQuoteCnt = 0 ;
    gnum = 4 ;
    temp = temp->next ;
    S_EXP( temp ) ;
  } // else if
  else {
    gnum = -1 ;
    throw MyException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // 沒有這種東西 
  } // else 

  return true ;     
} // S_EXP()

 
void PrintTab( int numOfTab ) 
{
  for ( int i = 0 ; i < numOfTab ; i ++ ) 
    cout << " " ;  
} // PrintTab() 

void PrintS_EXP( vector<EXP> s_exp ) 
{
  int tab = 0 ; 
  if ( s_exp.at( 0 ).token == "("  && s_exp.at( 1 ).token == "exit" && s_exp.at( 2 ).token == ")" )
  {
    
  } // if 
  else 
  {
    for ( int i = 0 ; i < s_exp.size() ; i++ )
    {
      if ( s_exp.at( i ).type == LEFT_PAREN )
      {
        try
        {
          if ( s_exp.at( i - 1 ).type == LEFT_PAREN )
          {
            PrintTab( 0 ); 
          } // if 
          else
          {
            PrintTab( tab ); 
          } // else  
        } // try 
        catch ( exception ex )
        {
          PrintTab( 0 ) ; 
        } // catch 
        
  
        cout << "(" << " " ;
        tab += 2 ; 
      } // if 
      else if ( s_exp.at( i ).type == QUOTE )
      {
        PrintTab( tab ) ;
        cout << "quote" << endl ; 
      } // else if 
      else if ( s_exp.at( i ).type == RIGHT_PAREN )
      {
        tab -= 2 ; 
        PrintTab( tab ) ;
        cout << ")" << endl ; 
      } // else if 
      else
      {
        try
        {
          if ( s_exp.at( i - 1 ).type == LEFT_PAREN )
          {
            PrintTab( 0 ); 
          } // if 
          else
          {
            PrintTab( tab ); 
          } // else 
        } // try 
        catch ( exception ex ) 
        {
          PrintTab( 0 );
        } // catch 
        
        
        cout << s_exp.at( i ).token << endl ; 
      } // else 
    } // for 
  } // else 
  

} // PrintS_EXP() 



EXP *GetValue( vector<EXP> vec, int &i ) {
  EXP * ptr = new EXP() ;
  ptr -> token = vec.at( i ).token ; 
  ptr -> type = vec.at( i ).type ;
  ptr -> column = vec.at( i ).column ;
  ptr -> row = vec.at( i ).row ;
  ptr -> next = NULL ;
  ptr -> pre_next = NULL ;
  ptr -> listPtr = NULL ;
  ptr -> pre_listPtr = NULL ;
  ptr -> quoteCnt = 0 ;
  ptr -> dotCnt = 0 ;
  i++ ;
  return ptr ;
  
} // GetValue()

// void preOrderTraversal(EXP* focusNode) {
//  if (focusNode != NULL) {
//    cout << focusNode->token << " ";
//    preOrderTraversal(focusNode->listPtr);
//    preOrderTraversal(focusNode->next);
//  }
// }
//

void BuildTree( vector<EXP> s_exp, int &i ) {
  EXP * temp = NULL ;
   // (1(2)
  while ( i < s_exp.size() ) {
    //  cout << endl << "s_exp.at(i).token: "       << s_exp.at(i).token << endl ;
    //  if( temp != NULL )
    //    cout << "temp->token: " << temp->token << endl ;
    //  preOrderTraversal(gRoot) ;  
      
    if ( s_exp.at( i ).type == RIGHT_PAREN ) {
      temp->next = GetValue( s_exp, i ) ; 
      temp->next->pre_next = temp ;
      
      
      while ( temp->type != LEFT_PAREN ) {
        temp = temp->pre_next ; 
      } // while
      
      temp = temp->pre_listPtr ;
      
    } // if
    else if ( s_exp.at( i ).type == LEFT_PAREN ) {
      if ( gRoot == NULL ) {
        gRoot = new EXP() ;
        gRoot->type = EMPTYPTR ;
        gRoot->token = "XXEMPTYXX" ;
        gRoot->next = NULL ;
        gRoot->listPtr = GetValue( s_exp, i ) ;
        gRoot->listPtr->pre_listPtr = gRoot ; 
        temp = gRoot->listPtr ;
      } // if
      else {
        temp->next = new EXP() ;
        temp->next->type = EMPTYPTR ;
        temp->next->token = "XXEMPTYXX" ;
        temp->next->pre_next = temp ;
        temp->next->next = NULL ;
        temp = temp->next ;
        
        temp->listPtr = GetValue( s_exp, i ) ;
        temp->listPtr->pre_listPtr = temp ;
        temp = temp->listPtr ;
               
      } // else

    } // else if
    else {
      if ( gRoot == NULL ) {
        gRoot = GetValue( s_exp, i ) ; 
        temp = gRoot ;
      } // if
      else {
        temp->next = GetValue( s_exp, i ) ; 
        temp->next->pre_next = temp ;
        temp = temp->next ;        
      } // else

    } // else 
    

  } // while
    
  
} // BuildTree()

string Rounding( string str ) { // 小數點後四位+四捨五入 

  stringstream ss ;
  ss << fixed << setprecision( 3 ) << atof( str.c_str() ) ;
  
  return ss.str() ;
  
} // Rounding()

string IntToString( int num ) {
  stringstream ss ;
  ss << num ;
  return ss.str() ;
} // IntToString()
 
void FixToken( vector<EXP> & s_exp ) {

  int i = 0 ;
  while ( i < s_exp.size() ) {
    if ( s_exp.at( i ).type == FLOAT ) {
      s_exp.at( i ).token = Rounding( s_exp.at( i ).token ) ;
    } // if 
    else if ( s_exp.at( i ).type == INT ) {
      s_exp.at( i ).token = IntToString( atoi( s_exp.at( i ).token.c_str() ) ) ;
    } // else if 
    else if ( s_exp.at( i ).token == "t" ) {
      s_exp.at( i ).token = "#t" ;
    } // else if 
    else if ( s_exp.at( i ).token == "#f" ) {
      s_exp.at( i ).token = "nil" ;
    } // else if 
    else if ( s_exp.at( i ).type == LEFT_PAREN && i+1 < s_exp.size() 
              && s_exp.at( i + 1 ).type == RIGHT_PAREN ) {
      s_exp.at( i ).token = "nil" ;
      s_exp.at( i ).type = NIL ;
      s_exp.erase( s_exp.begin()+i+1 ) ;
      
    } // else if 
    
    i++ ;
  } // while
  
} // FixToken()

void FixQuote( vector<EXP> & s_exp ) { // '(1 '4) , '(1), (1 '2 4 5)
  int i = 0 ;
  EXP temp ;
  int parnum = 0 ;
  while ( i < s_exp.size() ) {
    
    if ( s_exp.at( i ).token == "\'" && i+1 < s_exp.size() && s_exp.at( i+1 ).type == LEFT_PAREN ) { // '(
      
      s_exp.at( i ).token = "(" ;
      s_exp.at( i ).type = LEFT_PAREN ;
      
      temp.token = "quote" ;
      temp.type = QUOTE ;
      s_exp.insert( s_exp.begin()+i+1, temp ) ;
      i = i + 2 ;
      
      parnum = 0 ;
      int k = i ;
      
      while ( k < s_exp.size() ) {
        if ( s_exp.at( k ).type == LEFT_PAREN ) {
          parnum++ ;
        } // if
        else if ( s_exp.at( k ).type == RIGHT_PAREN ) {
          parnum-- ;
        } // else if
        
        if ( parnum == 0 ) { // 遇到正確的右括號 
          temp.token = ")" ;
          temp.type = RIGHT_PAREN ;
          s_exp.insert( s_exp.begin()+k+1, temp ) ;
          k = s_exp.size() ; // 出迴圈 
        } // if

        k++ ;
      } // while
      
      i = -1 ;
    } // if
    else if ( s_exp.at( i ).token == "\'" && i+1 < s_exp.size() && 
              ( s_exp.at( i + 1 ).type == SYMBOL || s_exp.at( i + 1 ).type == INT || 
                s_exp.at( i + 1 ).type == FLOAT || s_exp.at( i + 1 ).type == STRING ||
                s_exp.at( i + 1 ).type == NIL || s_exp.at( i + 1 ).type == T ) ) { // 'ATOM
      s_exp.at( i ).token = "(" ;
      s_exp.at( i ).type = LEFT_PAREN ;
      
      temp.token = "quote" ;
      temp.type = QUOTE ;
      s_exp.insert( s_exp.begin()+i+1, temp ) ;
      
      i = i + 2 ; // ATOM
      temp.token = ")" ;
      temp.type = RIGHT_PAREN ;
      s_exp.insert( s_exp.begin()+i+1, temp ) ;
      
      i = -1 ;
    } // else if
    
    i++ ;
  } // while
} // FixQuote()


static int uTestNum = -1 ; 

int main() { // +3 -> 3
  cin >> uTestNum ; 
  bool readEXP = true ;
  int parnum = 0 ; 
  int i = 0 ;
  bool syntaxIsTrue ;
  cout << "Welcome to OurScheme!" << endl << endl ;
  
  bool aLL_EXP_DONE = false ; 
  bool hasErr = false ; 
  
  vector<EXP> s_exp ;
  EXP nextToken ;
  
  
  while ( NOT aLL_EXP_DONE ) 
  { 
 
    readEXP = true ;
    parnum = 0 ; 
    gNowColumn = 0 ; 
    gNowRow = 1 ;  
    s_exp.clear() ; 
    hasErr = false ;
    gEndLine = false ; // false : 不要計算換行 ; true : 開始計算換行 
    
    cout << endl << "> " ; 
    while ( readEXP )
    {
      
      try 
      {
        nextToken = GetToken() ; 

      } // try 
      catch ( MyException exp ) // string Error  
      {
        cout << exp.What() << endl ; 
        readEXP = false ; 
        nextToken.token = "ERROR" ;
        nextToken.type = ERROR ; 
        hasErr = true ; 
      } // catch 
      
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
          readEXP = true ;
        } // if 
        
        else if ( nextToken.type != NONE && nextToken.type != ERROR ) 
        {
          
          
          FixToken( s_exp ) ; // 更正token 
          
          delete gRoot ;  
          gRoot = NULL ;
          i = 0 ;
          BuildTree( s_exp, i ) ;
          gHead = gRoot ; // new
          bool isTrue = true ; 
          try 
          {
            gnum = 0 ;
            isTrue = S_EXP( gHead ) ;

          } // try 
          catch ( MyException exp )
          {
            cout << exp.What() << endl ; 
            hasErr = true ; 
          } // catch 

          FixQuote( s_exp ) ;
          
          if ( NOT hasErr )
          {
            PrintS_EXP( s_exp ) ;   
          } // if 
              

          readEXP = false ;
        } // if 
        else if ( nextToken.type == NONE ) // nextToken == NONE 代表讀到 EOF 了，沒有任何指令了 
        {
          cout << "ERROR (no more input) : END-OF-FILE encountered" ; 
          readEXP = false ;
        } // else if 
        
         
      } // if 
      else if ( parnum < 0 )
      {
        try 
        {
          throw MyException( SYNERR_ATOM_PAR, nextToken ) ; 
        } // try 
        catch ( MyException exp )
        { readEXP = false ; 
          cout << exp.What() << endl ; 
        } // catch 
        
      } // else if 
      
      
      
    } // while ( readEXP )

         
    if ( nextToken.type == NONE ) // 讀到 EOF
    {
      aLL_EXP_DONE = true ;
    } // if 沒有 (exit) 
    else if ( s_exp.at( 0 ).token == "("  && s_exp.at( 1 ).token == "exit" && s_exp.at( 2 ).token == ")" )
    {
      aLL_EXP_DONE = true ;
    } // else if 
    
  } // while ( ALL_EXP ) 
  
  printf( "\nThanks for using OurScheme!" ) ;

  return 0;
    
} // main()  
