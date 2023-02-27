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
int gNowColumn = 1 ;
int gNowRow = 1 ; 
int gNumOfParen = 0 ;


#define NOT ! 

enum Type 
{
  LR_PAREN , // '()'  

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

  NONE 
}; // Type 

struct EXP {
  string token ;
  int column ;
  int row ; 
  Type type ; // SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN | RIGHT-PAREN
  EXP* next ;
  EXP* listPtr ;

}; // struct EXP 

// 2023/02/25 超級大肥肥新增這些程式碼

EXP * head ;

/* PrintType( Type type ) 
* 依造傳進去的 type 印出對應的 type 名稱 
*/ 
string PrintType ( Type type )
{
  if ( type == LR_PAREN ) 
    return "LP_PAREN";  // '()'  
  else if ( type == LEFT_PAREN ) 
    return "(" ; // '('
  else if ( type == RIGHT_PAREN ) 
    return ")" ; // ')'
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
    if (ch == '\n')
    {
      gNowRow++; 
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
  if ( CheckWhiteSpace ( ch ) )
  {
    return true ; 
  } // if 
  else if ( ch == '(' || ch == ')' || ch == '\'' ) 
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
    //cout << ch ; 
    if ( ch == '\\' ) // 跳脫字元 \"
    {
      // 遇到跳脫字元要把 \ 刪掉，並留下下一個字元 
      // EX:  '\"' --> '"', '\\"' --> '\"' 
      char peek = cin.peek() ; 
      ch = getchar() ; 
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

} // SkipCommnet() 

/* GetFirstChar ( ) 
*  跳過空白讀到第一個字元
*/ 
char GetFirstChar ( ) // skip white space to get First char 
{
  char ch = getchar() ; 
  while ( CheckWhiteSpace(ch) == true )
  {
    ch = getchar() ;  
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
EXP * GetToken ( ) {
  // (1 . (2 . (3 . nil)))
  // 切割遇到的第一個token 並判斷牠的type 

  EXP * gg = new EXP() ;
  gg->token = "\0" ; 
  gg->column = 0 ; 
  gg->row = 0 ; 
  gg->type = NONE ;
  gg->next = NULL ; 
  gg->listPtr = NULL ;

  char ch = GetFirstChar() ;
  char peek = '\0' ;  
  bool valid = true ;

  if ( IsComment ( ch ) )
  {
    SkipComment() ; 
    ch = GetFirstChar() ;
    valid = true ; 
  } // if 

  if ( CheckDelimiter ( ch ) )
  {
    gg->token += ch ; 
    valid = false ; 
  } // if 
  else if ( IsStringStart ( ch ) )
  {
    gg->token = GetString() ; 
    valid = false ; 
  } // else if 
  else if ( IsEOF ( ch ) )
  {
    gg->token = "\0" ;
    valid = false ; 
  } // else if 

  while ( valid )  //(1 . (2 . (3 . nil)))
  { 
    gg->token += ch ; 
    peek = cin.peek() ; 
    gNowColumn ++ ;  
    if ( CheckDelimiter ( peek ) )
    {
      valid = false ; 
    } // if
    
    if ( valid )
      ch = getchar() ; 


  } // while 


  gg->type = IdentifyType ( gg -> token ) ; 

  if ( gg->token == "\0" && gg->type == NONE ) // EOF 
  {
    gIsEOF = true ;  
  } // if 

  return gg ; 
  
} // getToken() 

//==================================================

bool IsATOM( EXP * temp ){ 
  if ( temp->type == SYMBOL || temp->type == INT  || temp->type == FLOAT  || temp->type == STRING  || temp->type == NIL || temp->type == T )
    return true ;
  else if ( temp->type == LEFT_PAREN && temp->next != NULL && temp->next->type == RIGHT_PAREN )
    return true ;
  else
    return false ;
} // IsATOM()

bool S_EXP( EXP * &temp ) { 
/*
<S-exp> ::= <ATOM> 
            | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
            | QUOTE <S-exp>
            
<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
            | NIL | T | LEFT-PAREN RIGHT-PAREN
*/

// (12 (    . 3))
// (1 . (2 . (3 . 4)))
// (1 2 3)

  cout << endl << "temp->token: " << temp->token << endl ;
  system("pause") ;
  
  if ( temp == NULL ) {
    cout << "aa " ; 
    return false ;
  } // if 
  else if ( IsATOM( temp ) == true ) {
    cout << "bb " ;
    return true ;
  } // else if
  else if ( temp->type == LEFT_PAREN ) { // LEFT-PAREN
    cout << "cc " ;
    temp = temp->next ;
    if ( S_EXP( temp ) == true ) { // <S-exp>
      cout << "dd " ;
      temp = temp->next ; // { <S-exp> } or [ DOT <S-exp> ] or RIGHT-PAREN

      while ( S_EXP( temp ) == true ) { // 零或多個 S-exp { <S-exp> }
        cout << "ee " ;
        temp = temp->next ;
      } // while
      if ( temp->type == DOT ) {
        cout << "ff " ;
        
        temp = temp->next ; // (
        if ( S_EXP( temp ) == true ) {
          cout << "gg " ;
//          cout << endl << "temp->tokenrrrrr: " << temp->token << endl ;
          temp = temp->next ;
          if ( temp->type == RIGHT_PAREN ) {
            cout << "hh " ;
            return true ;
          } // if
          else {
            cout << "ii " ;
            return false ;
          } // else 
        } // if
        else {
          cout << "zz " ;
          return false ;
        }

      } // if
      else if ( temp->type == RIGHT_PAREN ) {
        cout << "jj " ;
        return true ;
      } // else if 
      else {
        cout << "kk " ;  
        return false ;
      } // else       
      
    } // if
    
    else{
      cout << "mm " ; 
      return false ;
    } // else 
  } // else if
  else if ( temp->type == QUOTE ) {
    cout << "nn " ;
    temp = temp->next ; 
    if ( S_EXP( temp ) == true ) {
      cout << "oo " ;
      return true ;
    } // if
    else {
      cout << "pp " ; 
      return false ;
    }// else
  } // else if
  else{
    cout << "qq " ; 
    return false ;
  } // else
    
} // S_EXP()

void test() {
  EXP *temp = head->next ;
  while ( temp != NULL ) {
    cout << "temp->token: " << temp->token << endl ;
    temp = temp->next ;
  }
  
}

void PrintS_EXP( EXP * sExp ) 
{
  EXP * now = sExp ;    
  while( now != NULL ) 
  {
    cout << now->token << " " ; 
    now = now -> next ; 
  } // while 
} // PrintS_EXP( EXP * sExp ) 



int main() { // exit未完成 
  bool first = false ;
  bool readEXP = true ;
  bool start = true ; 
  int parnum = 0 ; 
  cout << "Welcome to OurScheme!" << endl ;
  
  bool ALL_EXP_DONE = false ; 
  
  EXP * list = NULL ;
  EXP * temp = NULL ;
  
  while ( NOT ALL_EXP_DONE ) 
  { 

    readEXP = true ;
    start = true ; 
    head = new EXP() ; 
    list = head ; 
    temp = head ; 
    parnum = 0 ; 
    
    while ( readEXP )
    {
      
      nextToken = GetToken() ; 
      if ( start ) {
        start = false ;
        temp -> token = nextToken -> token ; 
        temp -> type = nextToken -> type ;
        temp -> column = nextToken -> column ;
        temp -> row = nextToken -> row ;  
        temp->next = NULL ;
        temp -> listPtr = NULL ;
        
      } // if
      else if ( nextToken->type == RIGHT_PAREN ) { // list開始 
        temp -> next = new EXP() ;  
        temp = temp -> next ;
        list = temp ;
        temp->listPtr = nextToken ;
        temp = temp -> listPtr ;        
        temp->next = NULL ;
        temp -> listPtr = NULL ;
      } // else if
      else if ( nextToken->type == LEFT_PAREN ) { // list結尾 
        temp -> next = nextToken ; 
        temp = temp -> next ;
        temp->next = NULL ;
        temp -> listPtr = NULL ;
        temp = list ;     
      }
      else {
        temp -> next = nextToken ; 
        temp = temp -> next ;
        temp->next = NULL ;
        temp -> listPtr = NULL ;
      } // else 

      cout << nextToken -> token << " --> " << PrintType(nextToken -> type) << endl ; 
      
      if ( nextToken->type == LEFT_PAREN ) 
      { 
        parnum ++ ; 
        // cout << "Left Paren :" << parnum << endl ; 
      } // if       
      else if ( nextToken->type == RIGHT_PAREN ) 
      {
        parnum -- ; 
        // cout << "Right Paren :" << parnum << endl ; 
      } // else if 
      
      if ( parnum == 0 ) 
      {
        if ( s_exp -> type != NONE ) 
        {
          cout << ">>> Exp Done. s_Exp = " ; 
          PrintS_EXP( s_exp ) ;
          cout << " Syntax : " ;
          if ( S_EXP( s_exp) )
          {
            cout << "Correct ! " ; 
          } // if             
          else 
          {
            cout << "NOT Correct ! " ; 
          } // if             
          cout << endl << endl ; 
        } // if 
        else 
        {
          cout << ">>> ALL s_exp Read Done " ; 
        } // else 
        readEXP = false ; 
      } // if     
      
      
    } // while ( readEXP )  
    
    if ( s_exp -> type == NONE ) 
      ALL_EXP_DONE = true ;
    
  } // while ( ALL_EXP ) 
  
  
} // main()     

