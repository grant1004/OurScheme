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
  LEFT_PAREN, // '('

  RIGHT_PAREN, // ')'
  
  SYMBOL, // other token

  INT, // '123', '+123', '-123'
  
  FLOAT, // '123.567', '123.', '.567', '+123.4', '-.123'

  STRING, // "string's (example)." (strings do not extend across lines)

  NIL, // 'nil' or '#f'

  T, // 't' or '#t'

  DOT, // '.' 

  QUOTE, // '\'' ��޸�

  ADD, // '+'

  SUB, // '-'

  MULT, // '*'

  DIV, // '/'
  
  EMPTYPTR, // BuildTree() �Ū�   

  NONE, // EOF
  
  ERROR // ERROR
}; // Type 


struct EXP {
  string token ;
  int column ;
  int row ; 
  int nowRow ; // �������Ħr�����
  Type type ; // SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN | RIGHT-PAREN
  EXP* next ;
  EXP* pre_next ;
  EXP* listPtr ;
  EXP* pre_listPtr ;
  int dotCnt ;

}; // struct EXP 

  /*
  NOTHING,           // �S��ERROR 
  
  SYNERR_ATOM_PAR,   
  // �y�k���~ ( �U�@��token���ӭn�� ATOM �άO '(' )   
  // ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 1 is >>)<<  
  
  SYNERR_RIGHTPAREN, 
  // �y�k���~ (  ���ӭn���k�A���o�S���k�A��  ) 
  // ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<< 
  
  STRERR,            
  // String Error �֤@�����޸� '\"' 
  // ERROR (no closing quote) : END-OF-LINE encountered at line 1, column 19   
  */

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
    if ( ErrType == SYNERR_ATOM_PAR ) // ( �U�@��token���ӭn�� ATOM �άO '(' )
    {
      ss << "ERROR (unexpected token) : "
        << "atom or '(' expected when token at "
        << "Line " << token.row  
        << " Column " << token.column << " is >>" << token.token << "<<" ;  
      mErrMsg = ss.str() ;
    } // if 
    else if ( ErrType == SYNERR_RIGHTPAREN ) // (  ���ӭn���k�A���o�S���k�A��  )
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

EXP * gRoot = NULL ;
EXP * gHead = NULL ;

/* 
  PrintType( Type type ) 
  �̳y�Ƕi�h�� type �L�X������ type �W�� 
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
  * �ˬd ch �O�_�� white space 
*/

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

/* 
  CheckDelimiter ( char ch )
  * �ˬd ch �O�_�� Delimiter 
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
  * �ˬd�O�_�� '\"'  
*/

bool IsStringStart( char ch )
{
  if ( ch == '\"' )
    return true ; 
    
  return false ; 
} // IsStringStart()

/* 
  IsComment ( char ch )
  *  �ˬd�O�_�� ';' 
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

bool IsInt( string token ) // �}�Y�i�H�O0��???
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

bool IsFloat( string token ) // �}�Y�i�H�O0��??
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
*  �u�n���O INT || FLOAT || '(' || ')' || DOT || STRING || NIL || T || QUOTE 
*  �ѤU����@SYMBOL
*/

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

/* 
  IsString ( string token ) 
*  �}�Y�O�_�O "  "
*/

bool IsString( string token )
{
  if ( token[0] == '\"' ) 
    return true ; 
  return false ; 
} // IsString()


/* 
  IsDelimiter( string token, Type & type ) 
*  �ˬd�ODelimiter �ó]�w�L��type 
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
*  �O�_�� EOF 
*/

bool IsEOF( char ch )
{
  if ( ch == -1 || ch == EOF )
    return true ;
  return false ; 
} // IsEOF()

/* 
  IdentifyType ( string token ) 
*  ���� token �O���� type 
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
*  ���� string Ū�X�� ���� ('\n') �άO�t�@�� ('\"')
*  �p�GŪ�� '\n' �S��Ū�� '\"' ���N�O error  
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
    if ( ch == '\\' ) // ����r�� \"
    {
      // �J�����r���n�� \ �R���A�ïd�U�U�@�Ӧr�� 
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
* �� ; �᭱������Ū�� 
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
*  ���L�ť�Ū��Ĥ@�Ӧr��
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
* ���Utoken �çP�_�e�� type  
*/ 

EXP GetToken() 
{
  // (1 . (2 . (3 . nil)))
  // ���ιJ�쪺�Ĥ@��token �çP�_�e��type 

  EXP gg ;

  char ch = GetFirstChar() ;
  
  char peek = '\0' ;  
  bool valid = true ; // true : ���O delimiter string EOF�A false : �N��i��O delimiter string EOF 
  
  bool skipComment = false ; 
  
  while ( NOT skipComment ) // ���٨S�����������ѡA�N�i�hwhile�A�p�G�������L�F�~���U��
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
    while ( temp->type != LEFT_PAREN ) { // ���^�h 
      temp = temp->pre_next ; 
    } // while
    
    temp = temp->pre_listPtr->next ;
    S_EXP( temp ) ;
    
  } // else if 
  else if ( temp->type == RIGHT_PAREN ) {
    //    cout << "cc" << endl ;
    gnum = -1 ;
    throw SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ; 
    return false ; // temp ���ӬOs_EXP 
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
      return false ; // ���ӬO�k�A�� ex: . 3 3 
    } // if
    else {
      if ( gnum == 5 ) { // �e���ODOT 
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
      return false ; // ���ӬO�k�A�� ex: . (1) (1)  
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
      return false ; // ex: (1 . 3 . 3 ) // �X�{�ĤG��.�F 
    } // else 

  } // else if
  else if ( temp->type == DOT ) {  
    //    cout << "ww" << endl ;
    gnum = -1 ;
    throw SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
    return false ; // �ѰO�o�O�ƻ�ERROR�F����� 
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
    return false ; // �S���o�تF�� 
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
  cout << "VEC : " ; 
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
      // cout << "Line : " << s_exp.at( i ).row << " " ; 
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

string Rounding( string str ) { // �p���I��|��+�|�ˤ��J 

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
        
        if ( parnum == 0 ) { // �J�쥿�T���k�A�� 
          temp.token = ")" ;
          temp.type = RIGHT_PAREN ;
          s_exp.insert( s_exp.begin()+k+1, temp ) ;
          k = s_exp.size() ; // �X�j�� 
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

int main() { 
  cin >> uTestNum ; 

  int i = 0 ;
  bool syntaxIsTrue ;
  vector<Type> myStack ; // �Ψӭp�� ���A���M�k�A���٦� DOT ���ƶq 
  vector<DotCheck> dotStack ; 
  bool hasDot = false ; 

  cout << "Welcome to OurScheme!" << endl  ;
  
  bool quit = false ; 
  bool readEXP = true ;
  
  vector<EXP> s_exp ;
  EXP nextToken ;

  while ( NOT quit )
  {
    cout << endl << "> " ; 
    readEXP = true ; 
    gNowColumn = 0 ; 
    s_exp.clear() ; 
    myStack.clear() ;
    dotStack.clear() ;

    while ( readEXP == true )
    {
      try
      {
        
        nextToken = GetToken() ; // ���i��|��XstringException �M EofException

        if ( NOT dotStack.empty() && dotStack.back().isCheck == true )
        {
          // cout << "Is Check " << PrintType(nextToken.type) << endl ;
          if ( nextToken.type != RIGHT_PAREN )
          {
            // cout << "ERROR" ; 
            throw SyntaxErrorException( SYNERR_RIGHTPAREN, nextToken ) ; 
          } // if 
        
        } // if 

        if ( nextToken.type == RIGHT_PAREN && s_exp.back ( ).type == DOT )
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
          else // �o�����O�����F
          {
            
            FixToken( s_exp ) ; // �󥿤@��token t, (), nil...   

            // �إ߾𵲺c 
            delete gRoot ;  
            gRoot = NULL ;
            i = 0 ;
            BuildTree( s_exp, i ) ;
            gHead = gRoot ; // new


            // �P�_��k 
            gnum = 0 ;
            S_EXP( gHead ) ; // �i��|��X syntax execepiton 

            // �@�ǦL�X���O�e���B�� 
            gLastRow = s_exp.at( s_exp.size() - 1 ).nowRow ;
            FixQuote( s_exp ) ; 
            DeleteDotParen( s_exp ) ;
            quit = PrintS_EXP( s_exp ) ; 
            readEXP = false ;

            // cout << endl << "Debug >> �|�]��o�̥N��S������Exception�ӥB���O�����F " << endl ;
            // cout << endl << "=======================================================" << endl ;
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
        gLastRow = gNowRow ; 
        gNowRow ++ ;
        // cout << endl << "  ERROR ADD LINE  " << gNowRow ; 
        gNowColumn = 0 ; 

      } // catch Syntax Exception
      catch ( EofException ex )
      {
        cout << ex.What() ; 
        readEXP = false ; 
        quit = true ; 
      } // catch 
        
    } // while ( readEXP )

  } // while ( NOT quit )  
  
  printf( "\nThanks for using OurScheme!" ) ;

  return 0;
    
} // main()  
