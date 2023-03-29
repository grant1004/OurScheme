# include <map>
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
# include <stack>

using namespace std ;

bool gIsEOF = false ;  
bool gExit = false ; 
int gNowColumn = 0 ;
int gNowRow = 1 ; 
int gLastRow = 0 ; 
int gNumOfParen = 0 ;
bool gEndLine = false ;

# define NOT ! 

enum Type 
{
  LEFT_PAREN, RIGHT_PAREN, SYMBOL, INT, 
  
  FLOAT, STRING, NIL, T, DOT, QUOTE, 
  
  EMPTYPTR, NONE, ERROR,
  
  CONS, LIST, DEFINE, CAR, CDR, ATOM_QMARK,
  
  PAIR_QMARK, LIST_QMARK, NULL_QMARK,
  
  INTEGER_QMARK, REAL_QMARK, NUMBER_QMARK,
  
  STRING_QMARK, BOOLEAN_QMARK, SYMBOL_QMARK,
  
  ADD, SUB, MULT, DIV, NOTT, AND, OR,
  
  BIGGERTHAN, BIGGEREQUAL, 
  
  LESSTHAN, LESSEQUAL, EQUAL,
  
  STRING_APPEND, STRING_BIGGER,
  
  STRING_LESS, STRING_EQUAL,
  
  EQV_QMARK, EQUAL_QMARK, BEGIN,
  
  IF, COND, CLEAN_ENVIRONMENT
   
}; // Type 
 

struct EXP {
  string token ;
  int column ;
  int row ; 
  int nowRow ; // 紀錄有效字元行數
  Type type ; 
  EXP* next ;
  EXP* pre_next ;
  EXP* listPtr ;
  EXP* pre_listPtr ;
  int dotCnt ;
  vector<EXP> vec ; // EMPTYPTR用 
  int memSpace ;

}; // struct EXP 

EXP * gRoot = NULL ;
EXP * gHead = NULL ;

string PrettyString( vector<EXP> exp ) ; 

enum ExceptionType 
{ 
  NOTHING,  
  
  SYNERR_ATOM_PAR,   
  
  SYNERR_RIGHTPAREN   
  
}; // Enum ExceptionType 

class SyntaxErrorException
{
  string mErrMsg  ;

  public : 
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 

  SyntaxErrorException( ExceptionType ErrType, EXP token ) 
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
  } // SyntaxErrorException()

}; // SyntaxErrorException

class NotAStringException
{  
  string mErrMsg  ;
   
  public : 
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 
  
  NotAStringException( int line, int column ) 
  {
    stringstream ss ;
    gNowRow ++ ; 
    // cout << endl << "ERROR END LINE " << gNowRow ; 
    ss << "ERROR (no closing quote) : "
        << "END-OF-LINE encountered at Line "
        << line
        << " Column "
        << column ; 
    mErrMsg = ss.str() ;   
  } // NotAStringException() 

} ; 

class EofException // read EOF 
{
  string mErrMsg  ;

  public : 
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 

  EofException() 
  {
    stringstream ss ;
    ss << "ERROR (no more input) : END-OF-FILE encountered" ;
    mErrMsg = ss.str() ;   
  } // EofException() 

}; // EofException

class UnboundException
{
  string mErrMsg  ;
  public:
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 

  UnboundException( EXP * token )  
  {
    stringstream ss ;
    ss << "ERROR (unbound symbol) : " << token->token ;
    mErrMsg = ss.str() ;   
  } // UnboundException() 

}; // UnboundException

class NonListException
{
  string mErrMsg  ;
  public:
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 

  NonListException( vector<EXP> exp )  
  {
    stringstream ss ;
    ss << "ERROR (non-list) : " << PrettyString( exp ) ;
    mErrMsg = ss.str() ;   
  } // NonListException() 
}; // NonListException 

class IncorrectArgumentException
{
  string mErrMsg  ;
  public:
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 

  IncorrectArgumentException( string func, EXP exp )  // EXP exp 
  {
    stringstream ss ;
    ss << "ERROR (" << func << " with incorrect argument type) : " << exp.token ;
    mErrMsg = ss.str() ;   
  } // IncorrectArgumentException() 

  IncorrectArgumentException( string func2, vector<EXP> exp2 )  // vector exp2
  {
    stringstream ss ;
    ss << "ERROR (" << func2 << " with incorrect argument type) : " << PrettyString( exp2 ) ;
    mErrMsg = ss.str() ;   
  } // IncorrectArgumentException()

}; // IncorrectArgumentException

class IncorrectNumberException
{
  string mErrMsg  ;
  public:
  const char* What()
  {
    return mErrMsg.c_str() ;       
  } // What() 

  IncorrectNumberException( string func )  // EXP exp 
  {
    stringstream ss ;
    ss << "ERROR (incorrect number of arguments) : " << func ;
    mErrMsg = ss.str() ;   
  } // IncorrectNumberException() 

}; // IncorrectNumberException

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
  else if ( type == DOT )
    return "DOT" ; 
  else if ( type == QUOTE )
    return "QUOTE" ; 
  else if ( type == NONE ) 
    return "NONE" ; 

  return "ERROR TYPE" ; 
} // PrintType() 

bool CheckWhiteSpace( char ch )  
{

  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' ) 
  {
    if ( ch == '\n' ) 
    { 
      gNowColumn = 0 ; 
    } // if 
    
    return true ; 
  } // if 

  return false ;  
} // CheckWhiteSpace()

bool CheckDelimiter( char ch )
{
  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || 
       ch == '(' || ch == ')'  || ch == '\'' || ch == ';' || ch == '\"' ) 
  {
    return true ; 
  } // if

  return false ; 
} // CheckDelimiter()

bool IsStringStart( char ch )
{
  if ( ch == '\"' )
    return true ; 
    
  return false ; 
} // IsStringStart()

bool IsComment( char ch )
{
  if ( ch == ';' )
    return true ;
    
  return false ; 
} // IsComment()

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

bool IsSymbol( string token )
{
  if ( token == " " ) 
  {
    // cout << "Is White Space" ; 
    return false ; 
  } // if 
  else if ( token == "\0" ) 
  {
    // cout << "Is NULL" ; 
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

// void printRoot() {
//  cout << endl << "root Content: " << endl ;
//  cout << PrettyString( gHead->vec ) ; 
//  cout << endl << "========= root Content END =============" << endl ;
// } 

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
  
  int line = 0 ; 
  if ( gNowRow == gLastRow )
  {
    line = 1 ; 
  } // if 
  else
  {
    line = gNowRow - gLastRow ; 
  } // else 

  if ( ch == '\n' )
  {

    throw NotAStringException( line, gNowColumn ) ;
      
  } // if  
  else if ( ch == -1 )
  {
    throw NotAStringException( line, gNowColumn ) ; 

  } // else if   

  return str ; 
} // GetString()

/* 
  SkipComment ( ) 
* 把 ; 後面的全部讀掉 
*/ 

void SkipComment()
{
  char ch = '\0' ;
  do
  {
    ch = getchar() ;
  } while ( ch != '\n' && ch != -1 && ch != EOF ) ; 

  if ( ch == -1 || ch == EOF )
  {
    throw EofException() ; 
  } // if 

  gNowRow ++ ; 
  // cout << endl << "  COMMENT ADD LINE  " << gNowRow ;  
  gNowColumn = 0 ; 
} // SkipComment() 

/* 
   GetFirstChar ( ) 
*  跳過空白讀到第一個字元
*/ 

char GetFirstChar() // skip white space to get First char 
{
  char ch = '\0' ;

  do
  {
    ch = getchar() ; 

    if ( ch == '\n' )
    {
 
      gNowColumn = 0 ;
      gNowRow ++ ; 
      // cout << endl << "  ADD END LINE  " << gNowRow ;
    } // if 
    else
    {
      gNowColumn ++ ;
    } // else 

  } while ( CheckWhiteSpace( ch ) == true ) ; 

  if ( ch == EOF || ch == -1 ) 
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
  if ( gNowRow == gLastRow ) // on same line 
  {
    // cout << "  SUB  " ; 
    gLastRow -- ; 
  } // if 

  gg.row = gNowRow - gLastRow ; 
  gg.nowRow = gNowRow ; 
  // cout << endl << "NOW : " << gNowRow << ", Last : " << gLastRow << ", " << gg.nowRow << endl ;
  
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
    // cout << "Get EOF" ; 
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
    else if ( peek == EOF || peek == -1 )
    {
      valid = false ; 
    } // else if 
    
    if ( valid )
    {
      ch = getchar() ; 
      gNowColumn ++ ; 
    } // if 

  } // while 

  if ( gg.token == "\0" ) // EOF 
  {
    // cout << "Get EOF" << endl ; 
    throw EofException() ; 
  } // if
  else
  {
    // cout << "GET : " << gg.token << endl ; 
  } // else 


  gg.type = IdentifyType( gg.token ) ; 

  return gg ; 
  
} // GetToken() 

bool IsATOM( EXP ex )
{ 
  if ( ex.type == SYMBOL || ex.type == INT  || ex.type == FLOAT  || 
       ex.type == STRING  || ex.type == NIL || ex.type == T )
    return true ;
  else if ( ex.type == CONS || ex.type == LIST || ex.type == QUOTE || 
            ex.type == DEFINE || ex.type == CAR || ex.type == CDR || 
            ex.type == ATOM_QMARK || ex.type == PAIR_QMARK || 
            ex.type == LIST_QMARK || ex.type == NULL_QMARK || 
            ex.type == INTEGER_QMARK || ex.type == REAL_QMARK || 
            ex.type == NUMBER_QMARK || ex.type == STRING_QMARK || 
            ex.type == BOOLEAN_QMARK || ex.type == SYMBOL_QMARK || 
            ex.type == ADD || ex.type == SUB || ex.type == MULT || 
            ex.type == DIV || ex.type == NOTT || ex.type == AND || 
            ex.type == OR || ex.type == BIGGERTHAN || 
            ex.type == BIGGEREQUAL || ex.type == LESSTHAN || 
            ex.type == LESSEQUAL || ex.type == EQUAL || 
            ex.type == STRING_APPEND || ex.type == STRING_BIGGER || 
            ex.type == STRING_LESS || ex.type == STRING_EQUAL || 
            ex.type == EQV_QMARK || ex.type == EQUAL_QMARK || 
            ex.type == BEGIN || ex.type == IF || ex.type == COND || 
            ex.type == CLEAN_ENVIRONMENT ) {
              
    return true ;
  } // else if
  else
    return false ;
    
} // IsATOM()

bool IsATOM( EXP * temp )
{ 
  if ( temp->type == SYMBOL || temp->type == INT  || temp->type == FLOAT  || 
       temp->type == STRING  || temp->type == NIL || temp->type == T )
    return true ;
  else if ( temp->type == CONS || temp->type == LIST || temp->type == QUOTE || 
            temp->type == DEFINE || temp->type == CAR || temp->type == CDR || 
            temp->type == ATOM_QMARK || temp->type == PAIR_QMARK || 
            temp->type == LIST_QMARK || temp->type == NULL_QMARK || 
            temp->type == INTEGER_QMARK || temp->type == REAL_QMARK || 
            temp->type == NUMBER_QMARK || temp->type == STRING_QMARK || 
            temp->type == BOOLEAN_QMARK || temp->type == SYMBOL_QMARK || 
            temp->type == ADD || temp->type == SUB || temp->type == MULT || 
            temp->type == DIV || temp->type == NOTT || temp->type == AND || 
            temp->type == OR || temp->type == BIGGERTHAN || 
            temp->type == BIGGEREQUAL || temp->type == LESSTHAN || 
            temp->type == LESSEQUAL || temp->type == EQUAL || 
            temp->type == STRING_APPEND || temp->type == STRING_BIGGER || 
            temp->type == STRING_LESS || temp->type == STRING_EQUAL || 
            temp->type == EQV_QMARK || temp->type == EQUAL_QMARK || 
            temp->type == BEGIN || temp->type == IF || temp->type == COND || 
            temp->type == CLEAN_ENVIRONMENT ) {
              
    return true ;
  } // else if
  else
    return false ;
    
} // IsATOM()

int gnum = 0 ;
int gAfterDotCnt = 0 ;

bool S_EXP( EXP * temp ) { 

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



//  if ( temp != NULL )
//    cout << endl << "temp->token: " << temp->token << endl ;
//  cout << "gnum: " << gnum << endl ; 
//  system("pause") ;
  
  
  if ( temp == NULL && gnum == 2 ) {
    //      cout << "aa" << endl ;
    gnum = 8888 ;
    return true ;
  } // if
  else if ( temp == NULL && gRoot->type == QUOTE ) { // new
    //      cout << "uu" << endl ;
    return true ;
  } // else if
  else if ( temp->type == RIGHT_PAREN && ( gnum == 1 || gnum == 2 ) ) { 
    //      cout << "bb" << endl ;
    gnum = 2 ; // list
    while ( temp->type != LEFT_PAREN ) { // 走回去 
      temp = temp->pre_next ; 
    } // while
    
    temp = temp->pre_listPtr->next ;
    S_EXP( temp ) ;
    
  } // else if 
  else if ( temp->type == RIGHT_PAREN ) {
    //    cout << "cc" << endl ;
    gnum = -1 ;
    throw SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ; 
    return false ; // temp 應該是s_EXP 
  } // else if
  else if ( IsATOM( temp ) == true && gnum == 0 && temp->next == NULL && temp->listPtr == NULL ) {
    //    cout << "dd" << endl ;
    gnum = 1 ;
    return true ;
  } // else if
  else if ( IsATOM( temp ) == true ) { 
     //    cout << "gg" << endl ;
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
      //      cout << "ee" << endl ;
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      throw SyntaxErrorException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // 應該是右括號 ex: . 3 3 
    } // if
    else {
      if ( gnum == 5 ) { // 前面是DOT 
        //        cout << "hh" << endl ;
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
      } // if
      //      cout << "jj" << endl ;
      gnum = 1 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    } // else 

  } // else if
  else if ( temp->type == EMPTYPTR ) { 
    //    cout << "kk" << endl ;
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
      //      cout << "mm" << endl ;
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      temp = temp->listPtr ; 
      throw SyntaxErrorException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // 應該是右括號 ex: . (1) (1)  
    } // if
    else {
      //      cout << "oo" << endl ;
      if ( gnum == 5 ) { // DOT
        //        cout << "pp" << endl ;
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
        
      } // if
      
      gnum = 2 ;
      gAfterDotCnt = 0 ;
      temp = temp->listPtr->next ;
      S_EXP( temp ) ;
    } // else

  } // else if
  else if ( gnum == 0 && temp->type == DOT ) {
    //    cout << "rr" << endl ;
    throw SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // ex: .
  } // else if
  else if ( temp->type == DOT &&  
            ( ( temp->pre_next != NULL && temp->pre_next->type == EMPTYPTR ) || gnum == 1 ) && 
            gnum != 5 ) { 
    //    cout << "ss" << endl ;
    EXP * forward = temp->pre_next ;
    while ( forward != NULL && forward->type != DOT ) {
      forward = forward->pre_next ;
    } // while
    
    if ( forward == NULL ) {
      //      cout << "tt" << endl ;
      gnum = 5 ;
      gAfterDotCnt = 0 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    } // if
    else {
      //      cout << "vv" << endl ;
      throw SyntaxErrorException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // ex: (1 . 3 . 3 ) // 出現第二個.了 
    } // else 

  } // else if
  else if ( temp->type == DOT ) {  
    //    cout << "ww" << endl ;
    gnum = -1 ;
    throw SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // 忘記這是甚麼ERROR了先放著 
  } // else if
  else if ( temp->type == QUOTE ) { 
    //    cout << "xx" << endl ;
    gnum = 4 ;
    temp = temp->next ;
    S_EXP( temp ) ;
  } // else if
  else {
    //    cout << "yy" << endl ;
    gnum = -1 ;
    throw SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // 沒有這種東西 
  } // else 

  return true ;     
} // S_EXP()

 
void PrintTab( int numOfTab ) 
{
  for ( int i = 0 ; i < numOfTab ; i ++ ) 
    cout << " " ;  
} // PrintTab() 

void PrintVec( vector<EXP> vec )
{
  for ( int i = 0 ; i < vec.size() ; i ++ )
    cout << vec.at( i ).token << " " ; 
  cout << endl ; 
} // PrintVec()

void DeleteDotParen( vector<EXP> & s_exp )
{
  int parnum = 0 ;
  for ( int i = 0 ; i < s_exp.size() ; i++ )
  {
    if ( s_exp.at( i ).type == DOT )
    {
      if ( s_exp.at( i + 1 ).type == LEFT_PAREN ) // . ( 
      { 
        bool bb = false ; 
        for ( int r = i+1 ; r < s_exp.size( ) && NOT bb  ; r++ ) // i = '.'  i+1 = '('
        {
          if ( s_exp.at( r ).type == LEFT_PAREN )
          {
            parnum ++ ; 
          } // if 
          else if ( s_exp.at( r ).type == RIGHT_PAREN )
          {
            parnum -- ; 
            if ( parnum == 0 )
            {

              s_exp.erase( s_exp.begin() + r ) ; 
              bb = true ; 
            } // if 
          } // else if 

        } // for

        s_exp.erase( s_exp.begin() + i ) ; // delete '.' 
        s_exp.erase( s_exp.begin() + i ) ; // delete '('
        // PrintVec( s_exp ) ;  
      } // if 
      else if ( s_exp.at( i + 1 ).type == NIL ) // . nil 
      {
        s_exp.erase( s_exp.begin()+i ) ; 
        s_exp.erase( s_exp.begin()+i ) ; 
      } // else if 

    } // if 
     
  

  } // for



} // DeleteDotParen()

bool PrintS_EXP( vector<EXP> s_exp ) 
{
  int tab = 0 ; 
  int parnum = 0 ;  
  // (1 . (2 . (3 . 4))) --> ( 1 2 3 . 4 ) 
  if ( s_exp.at( 0 ).token == "("  && s_exp.at( 1 ).token == "exit" && s_exp.at( 2 ).token == ")" )
  {
    return true ; 
  } // if 
  else 
  {
    for ( int i = 0 ; i < s_exp.size() ; i++ )
    {
      // cout << "Line :" << s_exp.at( i ).nowRow << " " ;
      if ( s_exp.at( i ).type == LEFT_PAREN )
      {
        parnum ++ ; 
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
        cout << "quote" << endl ; 
      } // else if 
      else if ( s_exp.at( i ).type == RIGHT_PAREN )
      {
        parnum -- ;
        
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

  return false ;
} // PrintS_EXP() 


string AddTab( int numOfTab ) 
{
  string tab = "" ;
  for ( int i = 0 ; i < numOfTab ; i ++ ) 
    tab += " " ; 

  return tab ; 
} // AddTab() 

string PrettyString( vector<EXP> s_exp )
{
  int tab = 0 ; 
  int parnum = 0 ; 
  string str = "" ;
  // (1 . (2 . (3 . 4))) --> ( 1 2 3 . 4 ) 

  if ( s_exp.empty() )
  {
    return "Empty Root!" ; 
  } // if 

  for ( int i = 0 ; i < s_exp.size() ; i++ )
  {
    // cout << "Line :" << s_exp.at( i ).nowRow << " " ;
    if ( s_exp.at( i ).type == LEFT_PAREN )
    {
      parnum ++ ; 
      try
      {
        if ( s_exp.at( i - 1 ).type == LEFT_PAREN )
        {
          str += AddTab( 0 ); 
        } // if 
        else
        {
          str += AddTab( tab ); 
        } // else  
      } // try 
      catch ( exception ex )
      {
        str += AddTab( 0 ) ; 
      } // catch 


      str += "( " ;  
      tab += 2 ; 
    } // if 
    else if ( s_exp.at( i ).type == QUOTE )
    {
      try
      {
        if ( s_exp.at( i - 1 ).type == LEFT_PAREN )
        {
          str += AddTab( 0 ); 
        } // if 
        else
        {
          str += AddTab( tab ); 
        } // else 
      } // try 
      catch ( exception ex ) 
      {
        str += AddTab( 0 );
      } // catch 
      str += "quote" ;
      str += "\n";
    } // else if 
    else if ( s_exp.at( i ).type == RIGHT_PAREN )
    {
      parnum -- ;

      tab -= 2 ; 
      str += AddTab( tab ) ;
      str += ")" ; 
      str += "\n";
    } // else if 
    else
    {
      try
      {
        if ( s_exp.at( i - 1 ).type == LEFT_PAREN )
        {
          str += AddTab( 0 ); 
        } // if 
        else
        {
          str += AddTab( tab ); 
        } // else 
      } // try 
      catch ( exception ex ) 
      {
        str += AddTab( 0 );
      } // catch 


      str += s_exp.at( i ).token ; 
      str += "\n" ; 
    } // else 
  } // for 

  return str ; 
} // PrettyString()

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
  ptr -> dotCnt = 0 ;
  i++ ;
  return ptr ;
  
} // GetValue()

void PreOrderTraversal( EXP* focusNode ) {

  if ( focusNode != NULL ) {
    cout << focusNode->token << " ";
    PreOrderTraversal( focusNode->listPtr ) ;
    PreOrderTraversal( focusNode->next ) ;
  } // if 

} // PreOrderTraversal()


void BuildTree( vector<EXP> s_exp, int &i ) {
  EXP * temp = NULL ;
  while ( i < s_exp.size() ) {
      
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

string FloatToString( float value ) {
  stringstream ss ;
  ss << value ;
  return ss.str() ;
} // FloatToString()

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
      s_exp.at( i ).row = s_exp.at( i + 1 ).row ;
      s_exp.at( i ).nowRow = s_exp.at( i + 1 ).nowRow ;
      s_exp.erase( s_exp.begin()+i+1 ) ;
      
    } // else if 
    else if ( s_exp.at( i ).token == "define" ) {
      s_exp.at( i ).type = DEFINE ;
      //      s_exp.at( i ).token = "#<procedure define>" ;
    } // else if
    else if ( s_exp.at( i ).token == "cons" ) {
      s_exp.at( i ).type = CONS ;
      //      s_exp.at( i ).token = "#<procedure cons>" ;
    } // else if
    else if ( s_exp.at( i ).token == "list" ) {
      s_exp.at( i ).type = LIST ;
      //      s_exp.at( i ).token = "#<procedure list>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "\'" || s_exp.at( i ).token == "quote" ) {
      s_exp.at( i ).type = QUOTE ;
      //      s_exp.at( i ).token = "#<procedure quote>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "car" ) {
      s_exp.at( i ).type = CAR ;
      //      s_exp.at( i ).token = "#<procedure car>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "cdr" ) {
      s_exp.at( i ).type = CDR ;
      //      s_exp.at( i ).token = "#<procedure cdr>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "atom?" ) {
      s_exp.at( i ).type = ATOM_QMARK ;
      //      s_exp.at( i ).token = "#<procedure atom?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "pair?" ) {
      s_exp.at( i ).type = PAIR_QMARK ;
      //      s_exp.at( i ).token = "#<procedure pair?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "list?" ) {
      s_exp.at( i ).type = LIST_QMARK ;
      //      s_exp.at( i ).token = "#<procedure list?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "null?" ) {
      s_exp.at( i ).type = NULL_QMARK ;
      //      s_exp.at( i ).token = "#<procedure null?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "integer?" ) {
      s_exp.at( i ).type = INTEGER_QMARK ;
      //      s_exp.at( i ).token = "#<procedure integer?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "real?" ) {
      s_exp.at( i ).type = REAL_QMARK ;
      //      s_exp.at( i ).token = "#<procedure real?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "number?" ) {
      s_exp.at( i ).type = NUMBER_QMARK ;
      //      s_exp.at( i ).token = "#<procedure number?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "string?" ) {
      s_exp.at( i ).type = STRING_QMARK ;
      //      s_exp.at( i ).token = "#<procedure string?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "boolean?" ) {
      s_exp.at( i ).type = BOOLEAN_QMARK ;
      //      s_exp.at( i ).token = "#<procedure boolean?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "symbol?" ) {
      s_exp.at( i ).type = SYMBOL_QMARK ;
      //      s_exp.at( i ).token = "#<procedure symbol?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "+" ) {
      s_exp.at( i ).type = ADD ;
      //      s_exp.at( i ).token = "#<procedure +>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "-" ) {
      s_exp.at( i ).type = SUB ;
      //      s_exp.at( i ).token = "#<procedure ->" ;
    } // else if 
    else if ( s_exp.at( i ).token == "*" ) {
      s_exp.at( i ).type = MULT ;
      //      s_exp.at( i ).token = "#<procedure *>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "/" ) {
      s_exp.at( i ).type = DIV ;
      //      s_exp.at( i ).token = "#<procedure />" ;
    } // else if 
    else if ( s_exp.at( i ).token == "not" ) {
      s_exp.at( i ).type = NOTT ;
      //      s_exp.at( i ).token = "#<procedure not>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "and" ) {
      s_exp.at( i ).type = AND ;
      //      s_exp.at( i ).token = "#<procedure and>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "or" ) {
      s_exp.at( i ).type = OR ;
      //      s_exp.at( i ).token = "#<procedure or>" ;
    } // else if 
    else if ( s_exp.at( i ).token == ">" ) {
      s_exp.at( i ).type = BIGGERTHAN ;
      //      s_exp.at( i ).token = "#<procedure >>" ;
    } // else if 
    else if ( s_exp.at( i ).token == ">=" ) {
      s_exp.at( i ).type = BIGGEREQUAL ;
      //      s_exp.at( i ).token = "#<procedure >=>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "<" ) {
      s_exp.at( i ).type = LESSTHAN ;
      //      s_exp.at( i ).token = "#<procedure <>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "<=" ) {
      s_exp.at( i ).type = LESSEQUAL ;
      //      s_exp.at( i ).token = "#<procedure <=>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "=" ) {
      s_exp.at( i ).type = EQUAL ;
      //      s_exp.at( i ).token = "#<procedure =>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "string-append" ) {
      s_exp.at( i ).type = STRING_APPEND ;
      //      s_exp.at( i ).token = "#<procedure string-append>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "string>?" ) {
      s_exp.at( i ).type = STRING_BIGGER ;
      //      s_exp.at( i ).token = "#<procedure string>>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "string<?" ) {
      s_exp.at( i ).type = STRING_LESS ;
      //      s_exp.at( i ).token = "#<procedure string<>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "string=?" ) {
      s_exp.at( i ).type = STRING_EQUAL ;
      //      s_exp.at( i ).token = "#<procedure string=>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "eqv?" ) {
      s_exp.at( i ).type = EQV_QMARK ;
      //      s_exp.at( i ).token = "#<procedure eqv?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "equal?" ) {
      s_exp.at( i ).type = EQUAL_QMARK ;
      //      s_exp.at( i ).token = "#<procedure equal?>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "begin" ) {
      s_exp.at( i ).type = BEGIN ;
      //      s_exp.at( i ).token = "#<procedure begin>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "if" ) {
      s_exp.at( i ).type = IF ;
      //      s_exp.at( i ).token = "#<procedure if>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "cond" ) {
      s_exp.at( i ).type = COND ;
      //      s_exp.at( i ).token = "#<procedure cond>" ;
    } // else if 
    else if ( s_exp.at( i ).token == "clean-environment" ) {
      s_exp.at( i ).type = CLEAN_ENVIRONMENT ;
      //      s_exp.at( i ).token = "#<procedure clean-environment>" ;
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

struct DotCheck 
{
  Type type ; 
  bool isCheck ; 
  int leftParNum ; 
} ; // DotCheck 

int CountInVec( vector<Type> vec, Type tp )
{
  int cnt = 0 ; 
  for ( int i = 0 ; i < vec.size( ) ; i++ )
  {
    if ( vec.at( i ) == tp )
    {
      cnt ++ ; 
    } // if 
  } // for

  return cnt ; 
} // CountInVec()

static int uTestNum = -1 ; 

class Functions {

private: 
  map< string, vector<EXP> > msymbolMap ; // map symbol
  EXP* mexeNode ; 
  vector<EXP> mnonListVec ; // used to save non-list  
  int mlevel ; 
  int memNum ;
  vector<EXP> mresult ; 

  int FindMemNum( string str ) {
    map< string,vector<EXP> > :: iterator item = msymbolMap.find( str ) ;
    if ( item != msymbolMap.end() ) {
      return item->second.at( 0 ).memSpace ;
    } // if
    else {
      return -1 ;
    } // else
    
  } // FindMemNum()

  bool CheckNumOfArg( int num ) {
    EXP* temp = mexeNode->next ;
    int arg = 0 ;

    while ( temp->type != RIGHT_PAREN ) {
      arg++ ;
      temp = temp->next ; 
    } // while
    
    if ( arg != num ) 
      return false ;
    else 
      return true ;
    
  } // CheckNumOfArg()  

  bool IsAtomButNotStr( Type type ) {
    if ( type == SYMBOL || type == INT  || type == FLOAT || type == NIL || type == T )
      return true ;
    else
      return false ;
  } // IsAtomButNotStr()

  bool CompareVectors( vector<EXP> vec1, vector<EXP> vec2 ) {
    if ( vec1.size() != vec2.size() ) {
      return false ;
    } // if

    for ( int i = 0; i < vec1.size() ; i++ ) {
      if ( ( vec1.at( i ).token != vec2.at( i ).token ) || ( vec1.at( i ).type != vec2.at( i ).type ) ) {
        return false ;
      } // if
    } // for

    return true ;
  } // CompareVectors()

  string FixDoubleQuotes( string str ) { // iris
    string result = "" ;
    for ( int i = 1; i < str.length() - 1 ; i++ ) {
      result += str[i] ;
    } // for
     
    return result ; 
  } // FixDoubleQuotes()
  
  void TraversalEmpty( EXP* focusNode ) {

    if ( focusNode != NULL ) {
      if ( focusNode->type != EMPTYPTR )
      {
        mnonListVec.push_back( *focusNode ) ;
      } // if 
      
      TraversalEmpty( focusNode->listPtr ) ;
      TraversalEmpty( focusNode->next ) ;
    } // if 
    
  } // TraversalEmpty()

  void SetDotPair( vector<EXP> & vec )
  {
    stack<EXP> st ; 

    for ( int i = 0 ; i < vec.size() ; i++ )
    {
      st.push( vec.at( i ) ) ; 
    } // for 
    
  } // SetDotPair()

  bool IsNonList( EXP* temp )
  {
    EXP* listHead = temp ; // '(' 
    bool non = false ; 
    mnonListVec.clear() ; 
    while ( listHead->type != RIGHT_PAREN ) {
      if ( listHead->type == EMPTYPTR )
      {
        // listHead is EmptyPtr  
        EXP * left_paren = listHead->listPtr ; 
        vector<EXP> out ; 
        TraversalEmpty( left_paren ) ;

      } // if 
      else 
      {
        mnonListVec.push_back( *listHead ) ;
      } // else 

      if ( listHead->type == DOT ) {
        non = true ; 
      } // if

      listHead = listHead->next ;

    } // while

    mnonListVec.push_back( *listHead ) ; 
    return non ; 
  } // IsNonList() 

  bool IsInternalFunction( vector<EXP> vec )
  {
    if ( vec.size() == 1 )
    {
      if ( IsSystemPrimitive( vec.at( 0 ).type ) )
      {
        return true ; 
      } // if 
    } // if 

    return false ; 
  } // IsInternalFunction()
  
public : // 早安胖嘟嘟肥肥 
  void SetRoot() ; // iris
  void Eval() ; // iris
  void Define() ; // iris
  void Cons() ; // iris
  bool IsSystemPrimitive( Type type ) ; // iris
  //    void PrintMap() ; // iris
  bool FindMap( string str, vector<EXP> &new_vector ) ; // iris
  void Qmark( string whichQmark ) ; 
  // atom? , null? , integer? , real? , boolean?                         
  void Quote() ; // iris
  void Car() ; // iris
  void Cdr() ; // iris 
  void Arithmetic_Add_Sub_Mul_DIV( string whichOperator ) ; // + - * / iris 
  void CompareNum( string whichOperator ) ; // > , >= , < , <= , = iris
  void CompareString( string whichOperator ) ; // string>? , string<? , string=?
  void String_append() ; // iris
  void Not() ; // iris
  void Equal_qmark() ; // iris
  void Eqv_qmark() ; // iris
  void List() ; // iris
  void And() ; // iris
  void Or() ; // iris
  void Clean_Environment() ; // iris
  void ResetMemNum() ;
  void ResetLevel()
  {
    mlevel = 0 ; 
  } // ResetLevel()
  
  vector<EXP> GetResult( )
  {
    return mresult ; 
  } // GetResult() 
    
  //    EXP Pair_qmark() ; // pair?
  //    EXP List_qmark() ; // list?
  //    EXP Begin() ; // begin
  //    EXP If() ; // if 
  //    EXP Cond() ; // cond 

  void Execute() { // ptr指在function call上面 
  
    if ( mexeNode->type == DEFINE )
      Define() ;
    else if ( mexeNode->type == CONS )
      Cons() ; 
    else if ( mexeNode->type == ATOM_QMARK ) 
      Qmark( "atom?" ) ;
    else if ( mexeNode->type == NULL_QMARK )
      Qmark( "null?" ) ;
    else if ( mexeNode->type == INTEGER_QMARK )
      Qmark( "integer?" ) ;
    else if ( mexeNode->type == REAL_QMARK || mexeNode->type == NUMBER_QMARK ) // real? and number? 
      Qmark( "real?" ) ;
    else if ( mexeNode->type == BOOLEAN_QMARK )
      Qmark( "boolean?" ) ;
    else if ( mexeNode->type == STRING_QMARK )
      Qmark( "string?" ) ;
    else if ( mexeNode->type == SYMBOL_QMARK )
      Qmark( "symbol?" ) ; 
    else if ( mexeNode->type == QUOTE ) 
      Quote() ;
    else if ( mexeNode->type == CAR )
      Car() ;
    else if ( mexeNode->type == CDR )
      Cdr() ; 
    else if ( mexeNode->type == ADD )
      Arithmetic_Add_Sub_Mul_DIV( "+" ) ;
    else if ( mexeNode->type == SUB )
      Arithmetic_Add_Sub_Mul_DIV( "-" ) ;
    else if ( mexeNode->type == MULT )
      Arithmetic_Add_Sub_Mul_DIV( "*" ) ;
    else if ( mexeNode->type == DIV )
      Arithmetic_Add_Sub_Mul_DIV( "/" ) ;
    else if ( mexeNode->type == BIGGERTHAN )
      CompareNum( ">" ) ;
    else if ( mexeNode->type == BIGGEREQUAL )
      CompareNum( ">=" ) ;
    else if ( mexeNode->type == LESSTHAN ) 
      CompareNum( "<" ) ;
    else if ( mexeNode->type == LESSEQUAL )
      CompareNum( "<=" ) ;
    else if ( mexeNode->type == EQUAL )
      CompareNum( "=" ) ; 
    else if ( mexeNode->type == STRING_BIGGER )
      CompareString( "string>?" ) ;
    else if ( mexeNode->type == STRING_LESS )
      CompareString( "string<?" ) ;
    else if ( mexeNode->type == STRING_EQUAL ) 
      CompareString( "string=?" ) ;
    else if ( mexeNode->type == STRING_APPEND )
      String_append() ; 
    else if ( mexeNode->type == NOTT )
      Not() ; 
    else if ( mexeNode->type == EQUAL_QMARK )
      Equal_qmark() ;
    else if ( mexeNode->type == EQV_QMARK )
      Eqv_qmark() ;
    else if ( mexeNode->type == LIST ) 
      List() ; 
    else if ( mexeNode->type == AND ) 
      And() ;      
    else if ( mexeNode->type == OR ) 
      Or() ;
    else if ( mexeNode->type == CLEAN_ENVIRONMENT )
      Clean_Environment() ; 
    else 
      cout << "ERROR (unbound symbol) : 1879" << endl ;

     
    // ================= Grant ===============================          

    //    else if ( mexeNode->type == "pair?" )
    //    {
    //      Pair_qmark() ;
    //    } // else if 
    //    else if ( mexeNode->type == "list?" )
    //    {
    //      List_qmark() ;
    //    } // else if               
    //    else if ( mexeNode->type == "begin" )
    //    {
    //      Begin() ; 
    //    } // else if 
    //    else if ( mexeNode->type == "if" )
    //    {
    //      If() ;
    //    } // else if 
    //    else if ( mexeNode->type == "cond" )                                    
    //    {
    //      Cond() ;
    //    } // else if 
  } // Execute()

}; // Functions

void Functions::ResetMemNum() {
  memNum = 0 ;
} // Functions::ResetMemNum()

void Functions::SetRoot() {
  mexeNode = gHead ;
} // Functions::SetRoot()

// void Functions::PrintMap() {
//  vector<EXP> new_vector ;
//  cout << endl << "======= Map Content Start=======" << endl ;
//  for ( auto it = msymbolMap.rbegin(); it != msymbolMap.rend(); it++ ) {
//      cout << "string: " << (*it).first << "  ";
//      cout << "Content: " ;
//      new_vector.assign( ( *it ).second.begin(), (*it).second.end());
//      for ( int i = 0; i < new_vector.size(); i++ ){
//        cout << new_vector.at( i ).token << " ";
//      } // for 
//      cout << endl ;
//  } // for 
// } // PrintMap()

bool Functions::FindMap( string str, vector<EXP> &new_vector ) {
  map< string,vector<EXP> > :: iterator item = msymbolMap.find( str ) ;
  if ( item != msymbolMap.end() ) {
    new_vector.assign( item->second.begin(), item->second.end() ) ;
    return true ;
  } // if
  else {
    return false ;
  } // else
  
} // Functions::FindMap()

void Functions::Clean_Environment() {
  if ( CheckNumOfArg( 0 ) ) {
    msymbolMap.clear() ;
    cout << "environment cleaned" << endl ;
  } // if
  else {
    throw IncorrectNumberException( "clean-environment" ) ; 
    // cout << "ERROR (incorrect number of arguments) : clean-environment" << endl ;
  } // else
  
} // Functions::Clean_Environment()

void Functions::Or() { // arg >= 2 
  // 回傳第一個不是nil的arg 如果都是nil 回傳最後一個arg 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  vector<EXP> lastStmt ;
  bool bbreak = false ;
  EXP ex ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw IncorrectNumberException( "or" ) ;
    // cout << "ERROR (incorrect number of arguments) : and" << endl ;
  } // if
  else { 
    while ( temp->type != RIGHT_PAREN && bbreak == false ) { 
      
      if ( FindMap( temp->token, new_vector ) == true ) {
        if ( new_vector.size() == 1 && new_vector.at( 0 ).type == NIL ) 
          ;
        else {
          lastStmt.assign( new_vector.begin(), new_vector.end() ) ;
          bbreak = true ;
        } // else
           
      } // if
      else if ( temp->type == SYMBOL ) {
        throw UnboundException( temp ) ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        if ( temp->vec.size() == 1 && temp->vec.at( 0 ).type == NIL ) 
          ;
        else {
          lastStmt.assign( temp->vec.begin(), temp->vec.end() ) ;
          bbreak = true ;
        } // else
        
      } // else if 
      else {
        if ( temp->type == NIL )
          ;
        else {
          bbreak = true ;
          ex.token = temp->token ;
          ex.type = temp->type ;
          lastStmt.push_back( ex ) ;
        } // else

      } // else
      
      temp = temp->next ;

    } // while
    
    if ( bbreak == false ) {
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // if
    else {
      emptyptr->vec.assign( lastStmt.begin(), lastStmt.end() ) ;
    } // else
    
  } // else
  
} // Functions::Or()

void Functions::And() { // arg >= 2 
  // 回傳第一個出現的nil 如果沒有 回傳最後一個arg 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  vector<EXP> lastStmt ;
  EXP ex ;
  bool isNIL = false ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw IncorrectNumberException( "and" ) ;
    // cout << "ERROR (incorrect number of arguments) : and" << endl ;
  } // if
  else { 
    while ( temp->type != RIGHT_PAREN ) { 
      
      if ( temp->type == NIL ) {
        isNIL = true ;
      } // if
      else if ( FindMap( temp->token, new_vector ) == true ) {
        if ( new_vector.size() == 1 && new_vector.at( 0 ).type == NIL ) 
          isNIL = true ;
        else 
          lastStmt.assign( new_vector.begin(), new_vector.end() ) ;
           
      } // else if
      else if ( temp->type == SYMBOL ) {
        throw UnboundException( temp ) ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        if ( temp->vec.size() == 1 && temp->vec.at( 0 ).type == NIL ) 
          isNIL = true ;
        else 
          lastStmt.assign( temp->vec.begin(), temp->vec.end() ) ;
          
      } // else if 
      else {
        ex.token = temp->token ;
        ex.type = temp->type ;
        lastStmt.push_back( ex ) ;
      } // else
      
      temp = temp->next ;

    } // while
    
    if ( isNIL == true ) {
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // if
    else {
      emptyptr->vec.assign( lastStmt.begin(), lastStmt.end() ) ;
    } // else
    
  } // else
  
} // Functions::And()

void Functions::List() {
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  EXP ex ;
  ex.token = "(" ;
  ex.type = LEFT_PAREN ;
  emptyptr->vec.push_back( ex ) ;
  
  while ( temp->type != RIGHT_PAREN ) { 
    if ( FindMap( temp->token, new_vector ) == true ) {
      for ( int i = 0; i < new_vector.size() ; i++ ) {
        emptyptr->vec.push_back( new_vector.at( i ) ) ;
      } // for

    } // if 
    else if ( temp->type == SYMBOL ) {
      throw UnboundException( temp ) ; 
    } // else if 
    else if (  temp->type == EMPTYPTR ) {
      mexeNode = temp ;
      Eval() ;
      
      for ( int i = 0; i < temp->vec.size() ; i++ ) {
        emptyptr->vec.push_back( temp->vec.at( i ) ) ;
      } // for
      
    } // else if
    else {
      ex.token = temp->token ;
      ex.type = temp->type ;
      emptyptr->vec.push_back( ex ) ;
    } // else

    temp = temp->next ;
  } // while  
  
  ex.token = ")" ;
  ex.type = RIGHT_PAREN ;
  emptyptr->vec.push_back( ex ) ;

} // Functions::List()

void Functions::Eqv_qmark() { // arg == 2 
  // 只有比較的東西是atom(不能是string)時 或是兩個object有相同的mem space 是true 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ; 
  EXP* temp = mexeNode->next ;         
  EXP ex ;
  vector<EXP> new_vector ;
  int argNum = 1 ;    
  EXP firstArg ;
  EXP secondArg ;
  bool noError = true ;
  bool sameMap = false ;
  bool isNil = false ;
  string symbolDefined ;
  if ( CheckNumOfArg( 2 ) ) {

    while ( temp->type != RIGHT_PAREN ) {
      if ( FindMap( temp->token, new_vector ) == true ) {
        if ( argNum == 1 ) {
          symbolDefined = temp->token ;
          firstArg.vec.assign( new_vector.begin(), new_vector.end() ) ;
        } // if
        else if ( argNum == 2 ) {
          if ( FindMemNum( temp->token ) == FindMemNum( symbolDefined ) ) {
            secondArg.vec.assign( new_vector.begin(), new_vector.end() ) ;
            sameMap = true ;
          } // if
          else 
            isNil = true ;
        } // else if

      } // if
      else if ( temp->type == SYMBOL ) {                            
        throw UnboundException( temp ) ; 
        noError = false ;
      } // else if
      else if ( temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        isNil = true ;
      } // else if
      else {
        if ( argNum == 1 ) {
          ex.token = temp->token ;
          ex.type = temp->type ;
          firstArg.vec.push_back( ex ) ;
        } // if
        else {
          ex.token = temp->token ;
          ex.type = temp->type ;
          secondArg.vec.push_back( ex ) ;
        } // else

      } // else
      
      argNum++ ;
      temp = temp->next ;
    } // while
    
    EXP ex ;
    if ( noError == false ) {
      ;
    } // if
    else if ( sameMap == true || ( isNil == false && firstArg.vec.size() == 1 
                                   && secondArg.vec.size() == 1 
                                   && IsAtomButNotStr( firstArg.vec.at( 0 ).type ) 
                                   && IsAtomButNotStr( secondArg.vec.at( 0 ).type ) 
                                   && CompareVectors( firstArg.vec, secondArg.vec ) ) ) {
      ex.token = "#t" ;
      ex.type = T ;
      emptyptr->vec.push_back( ex ) ;
    } // else if
    else {
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // else 
    

    
  } // if
  else {
    throw IncorrectNumberException( "eqv?" ) ;
    // cout << "ERROR (incorrect number of arguments) : equal?" << endl ;
  } // else 
  
} // Functions::Eqv_qmark()

void Functions::Equal_qmark() { // arg == 2 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ; 
  EXP* temp = mexeNode->next ;         
  EXP ex ;
  vector<EXP> new_vector ;
  int argNum = 1 ;    
  EXP firstArg ;
  EXP secondArg ;
  bool isTrue = true ;
  
  if ( CheckNumOfArg( 2 ) ) {

    while ( temp->type != RIGHT_PAREN ) {
      if ( FindMap( temp->token, new_vector ) == true ) {
        if ( argNum == 1 )
          firstArg.vec.assign( new_vector.begin(), new_vector.end() ) ;
        else
          secondArg.vec.assign( new_vector.begin(), new_vector.end() ) ;
      } // if
      else if ( temp->type == SYMBOL ) {                            
        throw UnboundException( temp ) ; 
        isTrue = false ;
      } // else if
      else if ( temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        
        if ( argNum == 1 )
          firstArg.vec.assign( temp->vec.begin(), temp->vec.end() ) ;
        else
          secondArg.vec.assign( temp->vec.begin(), temp->vec.end() ) ;
      } // else if
      else {
        if ( argNum == 1 ) {
          ex.token = temp->token ;
          ex.type = temp->type ;
          firstArg.vec.push_back( ex ) ;
        } // if
        else {
          ex.token = temp->token ;
          ex.type = temp->type ;
          secondArg.vec.push_back( ex ) ;
        } // else

      } // else
      
      argNum++ ;
      temp = temp->next ;
    } // while
    
    EXP ex ;
    
    if ( isTrue == false ) {
      ;
    } // if
    else if ( CompareVectors( firstArg.vec, secondArg.vec ) ) {
      ex.token = "#t" ;
      ex.type = T ;
      emptyptr->vec.push_back( ex ) ;
    } // else if
    else {
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // else 
    

    
  } // if
  else {
    throw IncorrectNumberException( "=?" ) ;
    // cout << "ERROR (incorrect number of arguments) : equal?" << endl ;
  } // else 
  
} // Functions::Equal_qmark()

void Functions::Not() { // arg == 1
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;          
  EXP ex ;
  vector<EXP> new_vector ;
  bool isNIL = false ;
  if ( CheckNumOfArg( 1 ) ) {
    if ( temp->type == NIL || temp->type == T ) {
      if ( temp->type == NIL )  
        isNIL = true ;
    } // if
    else if ( FindMap( temp->token, new_vector ) == true ) {
      if ( new_vector.size() == 1 ) {
        if ( new_vector.at( 0 ).type == NIL )  
          isNIL = true ;
      } // if
    } // else if
    else if ( temp->type == SYMBOL ) {                            
      throw UnboundException( temp ) ; 
    } // else if
    else if ( temp->type == EMPTYPTR ) {
      mexeNode = temp ;
      Eval() ;
      
      if ( temp->vec.size() == 1 ) {
        if ( temp->vec.at( 0 ).type == NIL )  
          isNIL = true ;
      } // if
    } // else if
      
  } // if
  else {
    throw IncorrectNumberException( "not" ) ;
    // cout << "ERROR (incorrect number of arguments) : not" << endl ;
  } // else
  
  if ( isNIL == true ) {
    ex.token = "#t" ;
    ex.type = T ;
  } // if
  else {
    ex.token = "nil" ;
    ex.type = NIL ;
  } // else
  
  emptyptr->vec.push_back( ex ) ;
  
} // Functions::Not()

void Functions::String_append() { // FixDoubleQuotes()
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  string str = "\"" ;
  bool isTrue = true ;
  EXP ex ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw IncorrectNumberException( "string-append" ) ;
    // cout << "ERROR (incorrect number of arguments) : string-append" << endl ;
  } // if
  else {
    while ( temp->type != RIGHT_PAREN && isTrue == true ) { 
      if ( temp->type == STRING ) {
        str = str + FixDoubleQuotes( temp->token ) ;
      } // if
      else if ( FindMap( temp->token, new_vector ) == true ) {
        if ( new_vector.size() == 1 && new_vector.at( 0 ).type == STRING ) {
          str = str + FixDoubleQuotes( new_vector.at( 0 ).token ) ;
        } // if
        else {
          isTrue = false ;
          cout << "ERROR (string-append with incorrect argument type) : " << endl ;
        } // else

      } // else if 
      else if ( temp->type == SYMBOL ) {
        isTrue = false ;
        throw UnboundException( temp ) ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        
        if ( temp->vec.size() == 1 && temp->vec.at( 0 ).type == STRING ) {
          str = str + FixDoubleQuotes( temp->vec.at( 0 ).token ) ;
        } // if
        else {
          isTrue = false ;
          cout << "ERROR (string-append with incorrect argument type) : " << endl ;
        } // else
        
      } // else if
      else {
        isTrue = false ;
        cout << "ERROR (string-append with incorrect argument type) : " << endl ;
      } // else

      temp = temp->next ;
    } // while  
    
    str = str + "\"" ;
    ex.token = str ;
    ex.type = STRING ;
    emptyptr->vec.push_back( ex ) ;
  } // else

} // Functions::String_append()

void Functions::CompareString( string whichOperator ) { // string>? , string<? , string=?
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  string previous_string ;
  vector<EXP> new_vector ;
  EXP ex ;
  bool isTrue = true ;
  bool isFirstArg = true ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw IncorrectNumberException( whichOperator ) ;
    // cout << "ERROR (incorrect number of arguments) : " << whichOperator << endl ;
  } // if
  else {
    while ( temp->type != RIGHT_PAREN ) { 
      if ( temp->type == STRING ) {
        if ( isFirstArg == true ) {
          previous_string = FixDoubleQuotes( temp->token ) ;
          isFirstArg = false ;
        } // if
        else if ( whichOperator == "string>?" ) {
          if ( previous_string > FixDoubleQuotes( temp->token ) ) 
            previous_string = FixDoubleQuotes( temp->token ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        else if ( whichOperator == "string<?" ) {
          if ( previous_string < FixDoubleQuotes( temp->token ) ) 
            previous_string = FixDoubleQuotes( temp->token ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        else if ( whichOperator == "string=?" ) {
          if ( previous_string == FixDoubleQuotes( temp->token ) ) 
            previous_string = FixDoubleQuotes( temp->token ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
          
      } // if
      else if ( FindMap( temp->token, new_vector ) == true ) {
        if ( new_vector.size() == 1 && new_vector.at( 0 ).type == STRING ) {
  
          if ( isFirstArg == true ) {
            previous_string = FixDoubleQuotes( new_vector.at( 0 ).token ) ;
            isFirstArg = false ;
          } // if
          else if ( whichOperator == "string>?" ) {
            if ( previous_string > FixDoubleQuotes( new_vector.at( 0 ).token ) ) 
              previous_string = FixDoubleQuotes( new_vector.at( 0 ).token ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "string<?" ) {
            if ( previous_string < FixDoubleQuotes( new_vector.at( 0 ).token ) ) 
              previous_string = FixDoubleQuotes( new_vector.at( 0 ).token ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "string=?" ) {
            if ( previous_string == FixDoubleQuotes( new_vector.at( 0 ).token ) ) 
              previous_string = FixDoubleQuotes( new_vector.at( 0 ).token ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          
        } // if
        else {
          isTrue = false ;
          cout << "ERROR (" << whichOperator << " wit4h incorrect argument type)" << endl ;
        } // else

      } // else if 
      else if ( temp->type == SYMBOL ) {
        isTrue = false ;
        cout << "ERROR (unbound symbol)" << endl ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        
        if ( temp->vec.size() == 1 && temp->vec.at( 0 ).type == STRING ) { 

          if ( isFirstArg == true ) {
            previous_string = FixDoubleQuotes( temp->vec.at( 0 ).token ) ;
            isFirstArg = false ;
          } // if
          else if ( whichOperator == "string>?" ) {
            if ( previous_string > FixDoubleQuotes( temp->vec.at( 0 ).token ) ) 
              previous_string = FixDoubleQuotes( temp->vec.at( 0 ).token ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "string<?" ) {
            if ( previous_string < FixDoubleQuotes( temp->vec.at( 0 ).token ) ) 
              previous_string = FixDoubleQuotes( temp->vec.at( 0 ).token ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "string=?" ) {
            if ( previous_string == FixDoubleQuotes( temp->vec.at( 0 ).token ) ) 
              previous_string = FixDoubleQuotes( temp->vec.at( 0 ).token ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if

          
        } // if
        else {
          isTrue = false ;
          cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
        } // else
        
      } // else if
      else {
        isTrue = false ;
        cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
      } // else

      temp = temp->next ;
    } // while

    if ( isTrue == true ) {
      ex.token = "#t" ;
      ex.type = T ;
                                     
    } // if
    else {
      ex.token = "nil" ;
      ex.type = NIL ;
    } // else
    
    emptyptr->vec.push_back( ex ) ;
  } // else  
  
} // Functions::CompareString()

void Functions::CompareNum( string whichOperator ) { // arg >= 2
                             
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  float previous_number = 0 ;
  vector<EXP> new_vector ;
  EXP ex ;
  bool isTrue = true ;
  bool isFirstArg = true ;
  
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw IncorrectNumberException( whichOperator ) ;
    // cout << "ERROR (incorrect number of arguments) : " << whichOperator << endl ;
  } // if
  else {
    while ( temp->type != RIGHT_PAREN ) { 
      if ( temp->type == INT || temp->type == FLOAT ) {
          
        if ( isFirstArg == true ) {
          previous_number = atof( temp->token.c_str() ) ;
          isFirstArg = false ;
        } // if
        else if ( whichOperator == ">" ) {
          if ( previous_number > atof( temp->token.c_str() ) ) 
            previous_number = atof( temp->token.c_str() ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        else if ( whichOperator == ">=" ) {
          if ( previous_number >= atof( temp->token.c_str() ) ) 
            previous_number = atof( temp->token.c_str() ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        else if ( whichOperator == "<" ) {
          if ( previous_number < atof( temp->token.c_str() ) ) 
            previous_number = atof( temp->token.c_str() ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        else if ( whichOperator == "<=" ) {
          if ( previous_number <= atof( temp->token.c_str() ) ) 
            previous_number = atof( temp->token.c_str() ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        else if ( whichOperator == "=" ) {
          if ( previous_number == atof( temp->token.c_str() ) ) 
            previous_number = atof( temp->token.c_str() ) ;
          else {
            isTrue = false ;
          } // else  
        } // else if
        
      } // if
      else if ( FindMap( temp->token, new_vector ) == true ) {
        if ( new_vector.size() == 1 && 
             ( new_vector.at( 0 ).type == INT 
               || new_vector.at( 0 ).type == FLOAT ) ) {
  
          if ( isFirstArg == true ) {
            previous_number = atof( new_vector.at( 0 ).token.c_str() ) ;
            isFirstArg = false ;
          } // if
          else if ( whichOperator == ">" ) {
            if ( previous_number > atof( new_vector.at( 0 ).token.c_str() ) ) 
              previous_number = atof( new_vector.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == ">=" ) {
            if ( previous_number >= atof( new_vector.at( 0 ).token.c_str() ) ) 
              previous_number = atof( new_vector.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "<" ) {
            if ( previous_number < atof( new_vector.at( 0 ).token.c_str() ) ) 
              previous_number = atof( new_vector.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "<=" ) {
            if ( previous_number <= atof( new_vector.at( 0 ).token.c_str() ) ) 
              previous_number = atof( new_vector.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "=" ) {
            if ( previous_number == atof( new_vector.at( 0 ).token.c_str() ) ) 
              previous_number = atof( new_vector.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          
        } // if
        else {
          isTrue = false ;
          cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
        } // else

      } // else if 
      else if ( temp->type == SYMBOL ) {
        isTrue = false ;
        throw UnboundException( temp ) ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        
        if ( temp->vec.size() == 1 
             && ( temp->vec.at( 0 ).type == INT 
                  || temp->vec.at( 0 ).type == FLOAT ) ) {
          if ( isFirstArg == true ) {
            previous_number = atof( temp->vec.at( 0 ).token.c_str() ) ;
            isFirstArg = false ;
          } // if
          else if ( whichOperator == ">" ) {
            if ( previous_number > atof( temp->vec.at( 0 ).token.c_str() ) ) 
              previous_number = atof( temp->vec.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == ">=" ) {
            if ( previous_number >= atof( temp->vec.at( 0 ).token.c_str() ) ) 
              previous_number = atof( temp->vec.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "<" ) {
            if ( previous_number < atof( temp->vec.at( 0 ).token.c_str() ) ) 
              previous_number = atof( temp->vec.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "<=" ) {
            if ( previous_number <= atof( temp->vec.at( 0 ).token.c_str() ) ) 
              previous_number = atof( temp->vec.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          else if ( whichOperator == "=" ) {
            if ( previous_number == atof( temp->vec.at( 0 ).token.c_str() ) ) 
              previous_number = atof( temp->vec.at( 0 ).token.c_str() ) ;
            else {
              isTrue = false ;
            } // else  
          } // else if
          
        } // if
        else {
          isTrue = false ;
          cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
        } // else
        
      } // else if
      else {
        isTrue = false ;
        cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
      } // else

      temp = temp->next ;
    } // while

    if ( isTrue == true ) {
      ex.token = "#t" ;
      ex.type = T ;
                                     
    } // if
    else {
      ex.token = "nil" ;
      ex.type = NIL ;
    } // else
    
    emptyptr->vec.push_back( ex ) ;
              
  } // else
  
} // Functions::CompareNum()

void Functions::Arithmetic_Add_Sub_Mul_DIV( string whichOperator ) {  // arg >= 2

  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  float sum = 0 ;
  vector<EXP> new_vector ;
  bool noError = true ;
  bool isFirstArg = true ;
  bool hasFloat = false ;
  
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw IncorrectNumberException( whichOperator ) ;
    // cout << "ERROR (incorrect number of arguments) : " << whichOperator << endl ;
  } // if
  else {
    while ( temp->type != RIGHT_PAREN && noError == true ) {
      if ( temp->type == INT || temp->type == FLOAT ) {
        if ( temp->type == FLOAT )
          hasFloat = true ;
          
        if ( isFirstArg == true ) {
          sum = atof( temp->token.c_str() ) ;
          isFirstArg = false ;
        } // if
        else if ( whichOperator == "+" )
          sum = sum + atof( temp->token.c_str() ) ;
        else if ( whichOperator == "-" ) 
          sum = sum - atof( temp->token.c_str() ) ;
        else if ( whichOperator == "*" ) 
          sum = sum * atof( temp->token.c_str() ) ;
        else if ( whichOperator == "/" ) {
          if ( temp->token == "0" ) {
            cout << "ERROR (division by zero) : /" << endl ;
            noError = false ;
          } // if
          else {
            sum = sum / atof( temp->token.c_str() ) ;
          } // else()
        } // else if
  
      } // if
      else if ( FindMap( temp->token, new_vector ) == true ) {
        if ( new_vector.size() == 1 
             && ( new_vector.at( 0 ).type == INT 
                  || new_vector.at( 0 ).type == FLOAT ) ) {
          if ( new_vector.at( 0 ).type == FLOAT )
            hasFloat = true ;
            
          if ( isFirstArg == true ) {
            sum = atof( new_vector.at( 0 ).token.c_str() ) ;
            isFirstArg = false ;
          } // if
          else if ( whichOperator == "+" )
            sum = sum + atof( new_vector.at( 0 ).token.c_str() ) ;
          else if ( whichOperator == "-" ) 
            sum = sum - atof( new_vector.at( 0 ).token.c_str() ) ;
          else if ( whichOperator == "*" ) 
            sum = sum * atof( new_vector.at( 0 ).token.c_str() ) ;
          else if ( whichOperator == "/" ) {
            if ( new_vector.at( 0 ).token == "0" ) {
              cout << "ERROR (division by zero) : /" << endl ;
              noError = false ;
            } // if
            else {
              sum = sum / atof( new_vector.at( 0 ).token.c_str() ) ;
            } // else()
          } // else if
        
        } // if
        else {
          noError = false ;
          cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
        } // else
      } // else if
      else if ( temp->type == SYMBOL ) {
        noError = false ;
        throw UnboundException( temp ) ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        
        if ( temp->vec.size() == 1 
             && ( temp->vec.at( 0 ).type == INT 
                  || temp->vec.at( 0 ).type == FLOAT ) ) {
          if ( temp->vec.at( 0 ).type == FLOAT )
            hasFloat = true ;
            
          if ( isFirstArg == true ) {
            sum = atof( temp->vec.at( 0 ).token.c_str() ) ;
            isFirstArg = false ;
          } // if
          else if ( whichOperator == "+" )
            sum = sum + atof( temp->vec.at( 0 ).token.c_str() ) ;
          else if ( whichOperator == "-" ) 
            sum = sum - atof( temp->vec.at( 0 ).token.c_str() ) ;
          else if ( whichOperator == "*" ) 
            sum = sum * atof( temp->vec.at( 0 ).token.c_str() ) ;
          else if ( whichOperator == "/" ) {
            if ( temp->vec.at( 0 ).token == "0" ) {
              cout << "ERROR (division by zero) : /" << endl ;
              noError = false ;
            } // if
            else {
              sum = sum / atof( temp->vec.at( 0 ).token.c_str() ) ;
            } // else()
          } // else if
          
        } // if
        else {
          noError = false ;
          cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
        } // else
        
      } // else if
      else {
        cout << "ERROR (" << whichOperator << " with incorrect argument type)" << endl ;
        noError = false ;
      } // else 
      
      temp = temp->next ;
      
    } // while
    
    if ( noError == true ) {
      EXP ex ;
      if ( hasFloat == true ) {
        
        ex.token = Rounding( FloatToString( sum ) ) ;
        ex.type = FLOAT ;      
      } // if
      else { // INT
        ex.token = IntToString( sum ) ;
        ex.type = INT ;
      } // else
        
      cout << "ex.token:  " << ex.token << endl ;                    
                   
      emptyptr->vec.push_back( ex ) ;
    } // if
                                                             
  } // else
  
} // Functions::Arithmetic_Add_Sub_Mul_DIV() 
 
void Functions::Qmark( string whichQmark ) { 
  // atom? , null? , integer? , real? 
  // boolean? , string? , symbol?
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
                       
  EXP ex ;
  vector<EXP> new_vector ;
  bool isTrue = false ;
  if ( CheckNumOfArg( 1 ) ) { // 1
    
    if ( whichQmark == "atom?" 
         && temp->type != SYMBOL && IsATOM( temp ) ) {
      isTrue = true ;
    } // if
    else if ( whichQmark == "null?" && temp->type == NIL ) {
      isTrue = true ;
    } // else if
    else if ( whichQmark == "integer?" && temp->type == INT ) {
      isTrue = true ;
    } // else if
    else if ( whichQmark == "real?" 
              && ( temp->type == INT || temp->type == FLOAT ) ) {
      isTrue = true ;
    } // else if
    else if ( whichQmark == "boolean?" 
              && ( temp->type == NIL || temp->type == T ) ) {
      isTrue = true ;
    } // else if
    else if ( whichQmark == "string?" 
              && temp->type == STRING ) {
      isTrue = true ;
    } // else if
    else if ( whichQmark == "symbol?" 
              && IsSystemPrimitive( temp->type ) ) {
      isTrue = true ;
    } // else if
    else if ( FindMap( temp->token, new_vector ) == true ) {
      
      if ( new_vector.size() == 1 ) {
        ex.token = new_vector.at( 0 ).token ;
        ex.type = new_vector.at( 0 ).type ;
        if ( whichQmark == "atom?" && IsATOM( ex ) ) 
          isTrue = true ;
        else if ( whichQmark == "null?" && new_vector.at( 0 ).type == NIL ) 
          isTrue = true ;
        else if ( whichQmark == "integer?" && new_vector.at( 0 ).type == INT ) 
          isTrue = true ; 
        else if ( whichQmark == "real?" 
                  && ( new_vector.at( 0 ).type == INT 
                       || new_vector.at( 0 ).type == FLOAT ) ) 
          isTrue = true ;
        else if ( whichQmark == "boolean?" 
                  && ( new_vector.at( 0 ).type == NIL 
                       || new_vector.at( 0 ).type == T ) ) 
          isTrue = true ;
        else if ( whichQmark == "string?" 
                  && new_vector.at( 0 ).type == STRING ) 
          isTrue = true ;
        else if ( whichQmark == "symbol?" 
                  && ( IsSystemPrimitive( new_vector.at( 0 ).type ) ||
                       new_vector.at( 0 ).type == SYMBOL ) ) 
          isTrue = true ;
        else 
          isTrue = false ;
      } // if
      else
        isTrue = false ;
      
    } // else if
    else if ( temp->type == SYMBOL ) {
      throw UnboundException( temp ) ; 
    } // else if
    else if ( temp->type == EMPTYPTR ) {
      mexeNode = temp ;
      Eval() ;
      
      if ( temp->vec.size() == 1 ) {
        ex.token = temp->vec.at( 0 ).token ;
        ex.type = temp->vec.at( 0 ).type ;
        if ( whichQmark == "atom?" && IsATOM( ex ) ) 
          isTrue = true ;
        else if ( whichQmark == "null?" && temp->vec.at( 0 ).type == NIL ) 
          isTrue = true ;
        else if ( whichQmark == "integer?" && temp->vec.at( 0 ).type == INT ) 
          isTrue = true ; 
        else if ( whichQmark == "real?" 
                  && ( temp->vec.at( 0 ).type == INT 
                       || temp->vec.at( 0 ).type == FLOAT ) ) 
          isTrue = true ;
        else if ( whichQmark == "boolean?" 
                  && ( temp->vec.at( 0 ).type == NIL 
                       || temp->vec.at( 0 ).type == T ) ) 
          isTrue = true ;
        else if ( whichQmark == "string?" 
                  && temp->vec.at( 0 ).type == STRING ) 
          isTrue = true ;
        else if ( whichQmark == "symbol?" 
                  && ( IsSystemPrimitive( temp->vec.at( 0 ).type ) 
                       || temp->vec.at( 0 ).type == SYMBOL ) ) 
          isTrue = true ;
        else 
          isTrue = false ;
      } // if
      else
        isTrue = false ;
      
    } // else if 
    else 
      isTrue = false ;
      
      
    if ( isTrue == true ) {
      ex.token = "#t" ;
      ex.type = T ;
      emptyptr->vec.push_back( ex ) ;
    } // if
    else { // isTrue = false
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // else 
    
  } // if
  else {
    throw IncorrectNumberException( whichQmark ) ;
    // cout << "problem with the number of parameters" << endl ;
  } // else 
  
} // Functions::Qmark()

void Functions::Cdr() {

  vector<EXP> new_vector ;
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP ex ;
  int i = 0 ;

  if ( CheckNumOfArg( 1 ) ) {
    if ( FindMap( temp->token, new_vector ) == true ) {
      ex.token = "(" ;
      ex.type = LEFT_PAREN ;
      emptyptr->vec.push_back( ex ) ;
      if ( new_vector.at( 0 ).type == LEFT_PAREN ) {
        if ( new_vector.size() >= 2 &&  new_vector.at( 1 ).type == LEFT_PAREN ) {
          int parnum = 1 ;
          i = 2 ;

          while ( parnum != 0 ) {
            if ( new_vector.at( i ).type == LEFT_PAREN ) 
              parnum++ ;
            else if ( new_vector.at( i ).type == RIGHT_PAREN )
              parnum-- ;
            i++ ;
          } // while

          while ( i < new_vector.size() ) {
            ex.token = new_vector.at( i ).token ;
            ex.type = new_vector.at( i ).type ;
            emptyptr->vec.push_back( ex ) ;
            i++ ;
          } // while

        } // if
        else {
          i = 2 ;
          while ( i < new_vector.size() ) {
            ex.token = new_vector.at( i ).token ;
            ex.type = new_vector.at( i ).type ;
            emptyptr->vec.push_back( ex ) ;
            i++ ;
          } // while
        } // else

      } // if
      else {
        throw IncorrectArgumentException( "cdr", new_vector ) ;
        // cout << "ERROR (car with incorrect argument type)" << endl ;
      } // else

    } // if 
    else if ( temp->type == SYMBOL ) {
      throw UnboundException( temp ) ; 
      // cout << "ERROR (unbound symbol)" << endl ; 
    } // else if 
    else if ( temp->type == EMPTYPTR 
              && temp->listPtr->next->type == QUOTE 
              && temp->listPtr->next->next->type == EMPTYPTR ) { 
      mexeNode = temp ;
      Eval() ;

      ex.token = "(" ;
      ex.type = LEFT_PAREN ;
      emptyptr->vec.push_back( ex ) ;
      if ( temp->vec.size() >= 2 && temp->vec.at( 1 ).type == LEFT_PAREN ) {
        int parnum = 1 ;
        i = 2 ;  

        while ( parnum != 0 ) {
          if ( temp->vec.at( i ).type == LEFT_PAREN )
            parnum++ ;
          else if ( temp->vec.at( i ).type == RIGHT_PAREN )
            parnum-- ;
          i++ ;  
        } // while

        while ( i < temp->vec.size() ) {
          ex.token = temp->vec.at( i ).token ;
          ex.type = temp->vec.at( i ).type ;
          emptyptr->vec.push_back( ex ) ;
          i++ ;
        } // while
      } // if
      else {
        i = 2 ;
        while ( i < temp->vec.size() ) {
          ex.token = temp->vec.at( i ).token ;
          ex.type = temp->vec.at( i ).type ;
          emptyptr->vec.push_back( ex ) ;
          i++ ;
        } // while
      } // else

    } // if temp->type == EMPTYPTR
    else {
      throw IncorrectArgumentException( "cdr", *temp ) ; 
      // cout << "ERROR (cdr with incorrect argument type)" << endl ;
    } // else
  } // if CheckNumOfArg( 1 )

  else {
    throw IncorrectNumberException( "cdr" ) ;
    // cout << "ERROR (incorrect number of arguments)" << endl ;
  } // else


  if ( emptyptr->vec.at( 1 ).type == DOT )
  {
    emptyptr->vec.erase( emptyptr->vec.begin() ) ; // erase ( 
    emptyptr->vec.erase( emptyptr->vec.begin() ) ; // erase . 
    emptyptr->vec.pop_back() ;  // erase ) 
  } // if 

} // Functions::Cdr()

void Functions::Car() {
  vector<EXP> new_vector ;
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP ex ;
  if ( CheckNumOfArg( 1 ) ) {
    if ( FindMap( temp->token, new_vector ) == true ) {

      if ( new_vector.at( 0 ).type == LEFT_PAREN ) {
        if ( new_vector.size() >= 2 &&  new_vector.at( 1 ).type == LEFT_PAREN ) {
          int parnum = 1 ;
          int i = 2 ;
          ex.token = "(" ;
          ex.type = LEFT_PAREN ;
          emptyptr->vec.push_back( ex ) ;

          while ( parnum != 0 ) {
            if ( new_vector.at( i ).type == LEFT_PAREN ) 
              parnum++ ;
            else if ( new_vector.at( i ).type == RIGHT_PAREN )
              parnum-- ;

            ex.token = new_vector.at( i ).token ;
            ex.type = new_vector.at( i ).type ;
            emptyptr->vec.push_back( ex ) ;
            i++ ;

          } // while

        } // if
        else {
          ex.token = new_vector.at( 1 ).token ;
          ex.type = new_vector.at( 1 ).type ;
          emptyptr->vec.push_back( ex ) ;
        } // else

      } // if
      else {
        cout << "ERROR (car with incorrect argument type)" << endl ;
      } // else

    } // if FindMap
    else if ( temp->type == SYMBOL ) {
      cout << "ERROR (unbound symbol)" << endl ;
    } // else if 							  
    else if ( temp->type == EMPTYPTR 
              && temp->listPtr->next->type == QUOTE 
              && temp->listPtr->next->next->type == EMPTYPTR ) { 
      mexeNode = temp ;
      Eval() ;
      if ( temp->vec.size() >= 2 && temp->vec.at( 1 ).type == LEFT_PAREN ) {
        int parnum = 1 ;
        int i = 2 ;
        ex.token = "(" ;
        ex.type = LEFT_PAREN ;
        emptyptr->vec.push_back( ex ) ;

        while ( parnum != 0 ) {
          if ( temp->vec.at( i ).type == LEFT_PAREN ) 
            parnum++ ;
          else if ( temp->vec.at( i ).type == RIGHT_PAREN )
            parnum-- ;

          ex.token = temp->vec.at( i ).token ;
          ex.type = temp->vec.at( i ).type ;
          emptyptr->vec.push_back( ex ) ;
          i++ ;
        } // while
      } // if
      else {
        ex.token = temp->vec.at( 1 ).token ;
        ex.type = temp->vec.at( 1 ).type ;
        emptyptr->vec.push_back( ex ) ;
      } // else

    } // if temp->type == EMPTYPTR
    else {
      cout << "ERROR (car with incorrect argument type)" << endl ;
    } // else
  } // if CheckNumOfArg( 1 )
  else {
    throw IncorrectNumberException( "car" ) ; 
    // cout << "ERROR (incorrect number of arguments)" << endl ;
  } // else 

} // Functions::Car() 

void Functions::Quote() {
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP ex ;
  bool bbreak = false ;
  
  if ( CheckNumOfArg( 1 ) ) {
    while ( bbreak == false ) {
  
      if ( IsSystemPrimitive( temp->type ) ) {
        temp->type = SYMBOL ;
      } // if
      if ( temp->type == EMPTYPTR ) {
        temp = temp->listPtr ;
      } // if
      else if ( temp->type == RIGHT_PAREN ) {
        ex.token = temp->token ;
        ex.type = temp->type ;
        emptyptr->vec.push_back( ex ) ;
        
        while ( temp->type != LEFT_PAREN ) { // 走回去 
          temp = temp->pre_next ; 
        } // while
        
        temp = temp->pre_listPtr ;
        if ( temp == emptyptr ) {
          bbreak = true ;
        } 
        else {
          temp = temp->next ;
        }
      } // else if 
      else {
        ex.token = temp->token ;
        ex.type = temp->type ;
        emptyptr->vec.push_back( ex ) ;
        temp = temp->next ;
      } // else
  
    } // while
    
    emptyptr->vec.erase( emptyptr->vec.end() - 1 ) ;

  } // if
  else {
    throw IncorrectNumberException( "\'" ) ;
    // cout << "ERROR (incorrect number of arguments)" << endl ;
  } // else

} // Functions::Quote()

void Functions::Cons() { 
  string str ;
  EXP ex ;
  int argNum = 0 ;
  vector<EXP> new_vector ;
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  
  if ( CheckNumOfArg( 2 ) ) {
    ex.token = "(" ;
    ex.type = LEFT_PAREN ;
    emptyptr->vec.push_back( ex ) ;
    while ( temp != NULL && temp->type != RIGHT_PAREN ) {
      if ( temp->type != SYMBOL && IsATOM( temp ) ) {
        ex.token = temp->token ;
        ex.type = temp->type ;
        emptyptr->vec.push_back( ex ) ;
      } // if
      else if ( FindMap( temp->token, new_vector ) == true ) {
        for ( int i = 0; i < new_vector.size(); i++ ) {
          emptyptr->vec.push_back( new_vector.at( i ) ) ;
        } // for
        new_vector.clear() ;
        
      } // else if
      else if ( temp->type == SYMBOL ) {
        throw UnboundException( temp ) ; 
      } // else if 
      else if ( temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        for ( int i = 0; i < temp->vec.size(); i++ ) {
          emptyptr->vec.push_back( temp->vec.at( i ) ) ;
        } // for
        
      } // else if

      if ( argNum == 0 ) {
        ex.token = "." ;
        ex.type = DOT ;
        emptyptr->vec.push_back( ex ) ;
      } // if
      
      temp = temp->next ;
      argNum++ ;
      
    } // while
    
    ex.token = ")" ;
    ex.type = RIGHT_PAREN ;
    emptyptr->vec.push_back( ex ) ;
 
  } // else if
  else {
    throw IncorrectNumberException( "cons" ) ;
    // cout << "incorrect number of arguments" << endl ; // function call Name
  } // else
  
} // Cons()

void Functions::Define() { 
  memNum++ ;
  string str ;
  vector<EXP> vs ;
  vector<EXP> new_vector ; 
  vector<EXP> new_vector2 ; 
  EXP ex ;
  EXP* temp = mexeNode->next ;
  
  if ( CheckNumOfArg( 2 ) ) { 
    if ( temp->type == SYMBOL ) {
      str = temp->token ;
      temp = temp->next ;
      if ( temp->type == EMPTYPTR ) { 
        mexeNode = temp ;
        Eval() ;
        if ( FindMap( str, new_vector ) ){
          temp->vec.at( 0 ).memSpace = memNum ;
          msymbolMap[str] = temp->vec ;
        } // if
        else{
          temp->vec.at( 0 ).memSpace = memNum ;
          msymbolMap.insert( pair< string,vector<EXP> >(str,temp->vec) ) ;
        } // else
        
      } // if
      else if ( FindMap( temp->token, new_vector ) ) { 
        if ( FindMap( str, new_vector2 ) ){
          new_vector.at( 0 ).memSpace = FindMemNum( temp->token ) ; 
          msymbolMap[str] = new_vector ;
        } // if
        else {
          new_vector.at( 0 ).memSpace = FindMemNum( temp->token ) ; 
          msymbolMap.insert( pair< string,vector<EXP> >(str,new_vector) ) ;
        } // else
        
        
      } // else if
      else if ( temp->type == SYMBOL ) {
        throw UnboundException( temp ) ; 
      } // else
      else {
        ex.token = temp->token ;
        ex.type = temp->type ;
        ex.memSpace = memNum ;
        vs.push_back( ex ) ; 
        if ( FindMap( str, new_vector ) ){
          msymbolMap[str] = vs ;
        } // if
        else {
          msymbolMap.insert( pair< string,vector<EXP> >(str,vs) ) ;
        } // else
        
      } // else
      
      } // else if
      else {
        cout << "ERROR (DEFINE format)" << endl ;
      } // else
      
    } // if
    else {
      cout << "ERROR (DEFINE format)" << endl ; // pretty print
    } // else
    
    cout << str << " defined" ;


} // Functions::Define()

bool Functions:: IsSystemPrimitive( Type type ) {
  if ( type == CONS || type == LIST || type == QUOTE || type == DEFINE || type == CAR 
       || type == CDR || type == ATOM_QMARK || type == PAIR_QMARK || type == LIST_QMARK 
       || type == NULL_QMARK || type == INTEGER_QMARK || type == REAL_QMARK 
       || type == NUMBER_QMARK || type == STRING_QMARK || type == BOOLEAN_QMARK 
       || type == SYMBOL_QMARK || type == ADD || type == SUB || type == MULT || type == DIV 
       || type == NOTT || type == AND || type == OR || type == BIGGERTHAN 
       || type == BIGGEREQUAL || type == LESSTHAN || type == LESSEQUAL || type == EQUAL 
       || type == STRING_APPEND || type == STRING_BIGGER || type == STRING_LESS 
       || type == STRING_EQUAL || type == EQV_QMARK || type == EQUAL_QMARK || type == BEGIN 
       || type == IF || type == COND || type == CLEAN_ENVIRONMENT ) {
    return true ;
  } // if 
  else 
    return false ;
    
} // Functions::IsSystemPrimitive() 

void Functions::Eval() {

  bool hasError = false ;
  vector<EXP> new_vector ; // map用 
  EXP* temp = mexeNode ;

  mlevel ++ ; 
                                                                     
  if ( IsATOM(temp) && temp->type != SYMBOL  )  {
    cout << "Is just an atom but not a symbol" << endl ; 
    hasError = true ; 
    mexeNode = temp ;
    cout << temp->token ;
    // 出迴圈 
  } // else if
  else if ( temp->type == SYMBOL ) {
    cout << "Is a symbol without Paren: " ; 
    hasError = true ; 
    if ( FindMap( temp->token, new_vector) == false ) // unbound symbol 
    { 
      throw UnboundException( temp ) ; 
      // cout << "Line :  2037 >> ERROR (unbound symbol) : " << temp->token << endl ;
    } // if  
    else
    {
      PrintVec( new_vector ) ;                                                    
    } // else 

  } // else if 
  else if ( IsSystemPrimitive ( temp->type ) )
  {
    cout << temp->token << endl ; 
  } // else if 
  else 
  { // temp == emptyptr || temp == bounding symbol 

    temp = temp->listPtr; // first LEFT_PAREN
    EXP* firstArgument = temp->next ; 
    cout << "First Argument [" << firstArgument->token << "] " ;

    if ( FindMap( firstArgument->token, new_vector ) )
    {
      if ( IsSystemPrimitive( new_vector.at( 0 ).type ) )
      {
        firstArgument->token = new_vector.at( 0 ).token ; 
        firstArgument->type = new_vector.at( 0 ).type ; 
      } // if 
    } // if

    if ( firstArgument->token == "exit" )
    {
      if ( mlevel > 1 )
      {
        cout << "ERROR (level of exit)" ; 
         
      } // if 

      hasError = true ; 
    } // if 
    else if ( firstArgument->type == QUOTE ) {
      mexeNode = firstArgument ; 
    } // if
    else if ( IsNonList( temp ) ) // non list error 
    {
        hasError = true ; 
        throw NonListException( mnonListVec ) ; 
    } // if 
    else if ( IsATOM( firstArgument ) 
              && firstArgument->type != SYMBOL 
              && NOT IsSystemPrimitive( firstArgument->type ) )  
    { // non known function
      hasError = true ; 
      cout << "ERROR (attempt to apply non-function) : " << firstArgument->token << endl ;
    } // if 
    else if ( firstArgument->type == SYMBOL 
              || IsSystemPrimitive( firstArgument->type ) )  
    { 
      if ( IsSystemPrimitive( firstArgument->type ) )
      { 
        cout << "is a primitive function : " 
             << firstArgument->token << endl ;
        if ( ( firstArgument->type == DEFINE 
               || firstArgument->type == CLEAN_ENVIRONMENT ) 
               && mlevel > 1 )
                                                              
        {
          cout << "ERROR (level of " << firstArgument->token << ")" ; 
          hasError = true ; 
        } // if 
        else if ( firstArgument->type == DEFINE || firstArgument->type == COND ) // 未完成
        { 
          mexeNode = firstArgument ; 
        } // else if 
        else if ( firstArgument->type == IF )
        { 
          if ( CheckNumOfArg( 2 ) || CheckNumOfArg( 3 ) ) 
          {
            mexeNode = firstArgument ; 
          } // if 
          else
          {
            hasError = true ;
            throw IncorrectNumberException( "if" ) ;
            // cout << "ERROR (incorrect number of arguments) : if" << endl ; 
          } // else 
        } // else if 
        else if ( firstArgument->type == AND || firstArgument->type == OR )
        { 
          mexeNode = firstArgument ; 
          if ( NOT CheckNumOfArg( 1 ) && NOT CheckNumOfArg( 0 ) ) // >= 2 
          {
            cout << "bbbbbbbbbbbbb" << endl ;
            mexeNode = firstArgument ; 
          } // if 
          else
          {
            hasError = true ;
                                                                    
            cout << "ERROR (incorrect number of arguments) : " << firstArgument->token << endl ; 
          } // else

                             
        } // else if
        else  
        {
            mexeNode = firstArgument ; 
        } // else 
      } // if
      else // SYM is not the name of a known function
      {
        hasError = true ; 
        if ( FindMap( firstArgument->token, new_vector ) == false )
        {
          throw UnboundException( firstArgument ) ; 
          // cout << "ERROR (unbound symbol) : " << firstArgument->token << endl ; 
        } // if 
        else
        {
          cout << "ERROR (attempt to apply non-function) : " ; 
          PrintVec( new_vector ) ; 
        } // else 
      } // else 
    } // else if 
    else // the first argument of ( ... ) is ( 。。。 ), i.e., it is ( ( 。。。 ) ...... )
    {
      // firstArgument->type == EMPTYPTR 
      cout << "Now is : " << firstArgument->token << ", then Call Eval() " << endl ;
      mexeNode = firstArgument ; 
      Eval() ;

      
      if ( NOT firstArgument->vec.empty() 
           && firstArgument->vec.size() == 1 
           && IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) ) 
      {
        cout << "Return Eval() and argument is changed to : " << firstArgument->vec.at( 0 ).token << endl ;

        firstArgument->token = firstArgument->vec.at( 0 ).token ; 
        firstArgument->type = firstArgument->vec.at( 0 ).type ;
        mexeNode = firstArgument ; 
      } // if 
      else if ( NOT firstArgument->vec.empty() 
                && NOT IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) )
      {
        cout << "ERROR (attempt to apply non-function) : " ; 
        PrintVec( firstArgument->vec ) ; 
        hasError = true ; 
      } // else 
      else
      {
        cout << "ERROR : the vec is empty" ;
        hasError = true ; 
      } // else 

    } // else 

  } // else if

  if ( hasError == false ) {
    cout << "Execute Start : " << mexeNode->token  << endl << endl ;
    Execute() ;
  } // if 

} // Eval()

bool PrintRoot() 
{
  if ( NOT gHead->vec.empty() )
  {
    cout << "========= root: =========" << endl ;
    DeleteDotParen( gHead->vec ) ; 
    return PrintS_EXP( gHead->vec ) ; 
  } // if 
  cout << endl << "gHead->vec is EMPTY" ;  

  return false ; 
} // PrintRoot()

int main() { 
//  cin >> uTestNum ; 

  int i = 0 ;
  bool syntaxIsTrue ;
  vector<Type> myStack ; // 用來計算 左括號和右括號還有 DOT 的數量 
  vector<DotCheck> dotStack ; 
  bool hasDot = false ; 

  cout << "Welcome to OurScheme!" << endl  ;
  
  bool quit = false ; 
  bool readEXP = true ;
  
  vector<EXP> s_exp ;
  EXP nextToken ;
  EXP lastToken ; 

  Functions funcClass ; 
  funcClass.ResetMemNum() ;

  while ( NOT quit )
  {
    cout << endl << "> " ; 
    readEXP = true ; 
    gNowColumn = 0 ; 
    s_exp.clear() ; 
    myStack.clear() ;
    dotStack.clear() ;
    funcClass.ResetLevel() ;
    
    while ( readEXP == true )
    {

      try
      {

        nextToken = GetToken() ; // 有可能會丟出stringException 和 EofException

        if ( NOT dotStack.empty() && dotStack.back().isCheck == true )
        {
          // cout << "Is Check " << PrintType(nextToken.type) << endl ;
          if ( nextToken.type != RIGHT_PAREN )
          {
            // cout << "ERROR" ; 
            throw SyntaxErrorException( SYNERR_RIGHTPAREN, nextToken ) ; 
          } // if 
        
        } // if 

        if ( nextToken.type == RIGHT_PAREN && NOT s_exp.empty() && s_exp.back().type == DOT )
        {
          throw SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
        } // if 

        if ( nextToken.type == DOT && NOT s_exp.empty() && s_exp.back ( ).type == LEFT_PAREN )
        {
          throw SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
        } // if 

        s_exp.push_back( nextToken ) ; 


        if ( nextToken.type == LEFT_PAREN )
        {
          myStack.push_back( nextToken.type ) ; 
        } // if 
        else if ( nextToken.type == RIGHT_PAREN )
        {
          // cout << "Right Paren " ;  
          // cout << "dot cnt : " << dotCnt << " DOT Stack size : " << dotStack.size() << endl; 
          if ( myStack.empty( ) )
          {
            // cout << "RIGHT PAREN Exception line : " << nextToken.row  ;
            throw SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
          } // else if 
          else if ( myStack.back() == LEFT_PAREN )
          {
            // cout << "Correct" << endl ; 
            myStack.pop_back() ; 

            if ( NOT dotStack.empty() && CountInVec( myStack, DOT ) == dotStack.size( ) && 
                 dotStack.back().leftParNum == CountInVec( myStack, LEFT_PAREN ) )
            {
              dotStack.back().isCheck = true ; 
            } // if 
          } // if 
          else if ( myStack.back() == DOT )
          {
            // cout << "Debug >> pop stack : " << PrintType( myStack.top() ) << endl ;
            myStack.pop_back() ;
            // cout << "Debug >> pop stack : " << PrintType( myStack.top() ) << endl ;
            myStack.pop_back() ;

            // cout << "Pop dot stack" << endl ; 
            dotStack.pop_back() ; 

            if ( NOT dotStack.empty( ) && dotStack.back().leftParNum == CountInVec( myStack, LEFT_PAREN ) )
            {
              dotStack.back().isCheck = true ; 
            } // if  
            
          } // else if 
        } // else if
        else if ( nextToken.type == DOT )
        {
          DotCheck tt ; 
          tt.type = DOT ; 
          tt.isCheck = false ; 
          tt.leftParNum = CountInVec( myStack, LEFT_PAREN ) ; 
          dotStack.push_back( tt ) ; 

          // cout << "DOT" ; 
          if ( myStack.empty() ) 
          {
            // cout << " has Exception" << endl ; 
            throw SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
          } // if 
          else if ( myStack.back() == DOT ) 
          {
            // cout << " has Exception" << endl ; 
            throw SyntaxErrorException( SYNERR_RIGHTPAREN, nextToken ) ; 
          } // else if 
          else
          {
            // cout << " Correct" << endl ; 
            myStack.push_back( nextToken.type ) ; 
          } // else 
 
          
        } // else if  
        else
        {
          if ( NOT dotStack.empty() && CountInVec( myStack, DOT ) == dotStack.size( ) && 
               dotStack.back().leftParNum == CountInVec( myStack, LEFT_PAREN ) )
          {
            dotStack.back().isCheck = true ; 
          } // if
        } // else 
        // cout << " NO Exception" << endl ; 

        if ( myStack.empty() )
        {  
          if ( nextToken.type == QUOTE )
          {
            readEXP = true ; 
          } // if 
          else // 這條指令結束了 expend
          {
            
            
            FixQuote( s_exp ) ; 
            FixToken( s_exp ) ; // 更正一些token t, (), nil, function  call ...
            DeleteDotParen( s_exp ) ;

            // cout << "Pretty Print S_EXP: " << endl << PrettyString( s_exp ) << endl ;  
            
            // 建立樹結構 
            delete gRoot ;  
            gRoot = NULL ;
            i = 0 ;
            BuildTree( s_exp, i ) ;
            gHead = gRoot ; // new


            // 判斷文法 
            gnum = 0 ;
            S_EXP( gHead ) ; 
            // 可能會丟出 syntax execepiton  
            // preOrderTraversal(gHead) ; 
            
            
            funcClass.SetRoot() ;
            funcClass.Eval() ;
            funcClass.ResetLevel() ; 
            //  funcClass.PrintMap() ; // test
            
            PrintRoot() ;
            // 一些印出指令前的處裡 
            gLastRow = s_exp.at( s_exp.size() - 1 ).nowRow ;
             
            readEXP = false ;

          } // else if 
        } // if 
        else if ( myStack.back() == DOT && myStack.size() == 1 )
        {
          throw SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
        } // else if 

      } // try 
      catch ( NotAStringException ex ) // has no closing quote exception 
      {
        cout << ex.What() << endl ; 

        // After print an String Error Message
        // we should break the while( readEXP ) 
        readEXP = false ; 

        gLastRow = gNowRow ;
      } // catch  the No Closing Quote Exception 
      catch ( SyntaxErrorException ex ) // SyntaxError 
      {
        

        cout << ex.What() << endl ; 
        // After print an Syntax Error Message
        // we should break the while( readEXP )
        readEXP = false ; 

        // and we should read char to end line ; 
        char ch = '\0' ;
        do
        {
          ch = getchar() ; 
        } while ( ch != '\n' && ch != EOF && ch != -1 ) ; 

        // and gNowRow plus one, gNowColumn clear 
        
        gNowRow ++ ;
        gLastRow = gNowRow ; 

        // cout << endl << "  ERROR ADD LINE  " << gNowRow ; 
        gNowColumn = 0 ; 

      } // catch Syntax Exception
      catch ( EofException ex )
      {
        cout << ex.What() ; 
        readEXP = false ; 
        quit = true ; 
      } // catch 
      catch ( UnboundException ex )
      {
        funcClass.ResetLevel() ; 
        cout << ex.What() ; 
        readEXP = false ; 
      } // catch 
      catch ( NonListException ex )
      {
        funcClass.ResetLevel() ; 
        cout << ex.What() ; 
        readEXP = false ; 
      } // catch 
      catch ( IncorrectArgumentException ex )
      {
        funcClass.ResetLevel() ; 
        cout << ex.What() ; 
        readEXP = false ;
      } // catch 
      catch ( IncorrectNumberException ex )
      {
        funcClass.ResetLevel() ; 
        cout << ex.What() ; 
        readEXP = false ;
      } // catch 


    } // while ( readEXP )

  } // while ( NOT quit )  
  
  printf( "\nThanks for using OurScheme!" ) ;

  return 0;
    
} // main()
