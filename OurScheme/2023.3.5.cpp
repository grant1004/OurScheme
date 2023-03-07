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

  LEFT_RIGHT_PAREN, // '()' -> NIL 
  
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
  Type type ; // SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN | RIGHT-PAREN
  EXP* next ;
  EXP* pre_next ;
  EXP* listPtr ;
  EXP* pre_listPtr ;
  int dotCnt ;
  int quoteCnt ;

}; // struct EXP 




enum ExceptionType 
{ 
  NOTHING,           // �S��ERROR 
  SYNERR_ATOM_PAR,   // �y�k���~ ( �U�@��token���ӭn�� ATOM �άO '(' )   ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 1 is >>)<<  
  SYNERR_RIGHTPAREN, // �y�k���~ (  ���ӭn���k�A���o�S���k�A��  ) ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<< 
  STRERR,            // String Error �֤@�����޸� '\"' ERROR (no closing quote) : END-OF-LINE encountered at line 1, column 19   
};

class MyException
{  
  string ErrMsg  ;
   
  public : 
    const char* what() const throw()
    {
      return ErrMsg.c_str() ;       
    } // what() 
    
    MyException()
    {
      ErrMsg = "This is an empty Exception.\n" ; 
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
        ErrMsg = ss.str() ;   
      } // if 
    } // MyException() 
    
    MyException( ExceptionType ErrType, EXP token) 
    {
      stringstream ss ;
      if ( ErrType == SYNERR_ATOM_PAR ) // ( �U�@��token���ӭn�� ATOM �άO '(' )
      {
        ss << "ERROR (unexpected token) : "
           << "atom or '(' expected when token at "
           << "Line " << token.row  
           << " Column " << token.column << " is >>" << token.token << "<<" ;  
        ErrMsg = ss.str() ;
      } // else 
      else if ( ErrType == SYNERR_RIGHTPAREN ) // (  ���ӭn���k�A���o�S���k�A��  )
      {
        ss << "ERROR (unexpected token) : "
           << "')' expected when token at "
           << "Line " << token.row  
           << " Column " << token.column << " is >>" << token.token << "<<" ;  
        ErrMsg = ss.str() ;
      } // else 
    } // MyException()
} ; 



// 2023/02/25 �W�Ťj�Ϊηs�W�o�ǵ{���X

EXP * root = NULL ;

/* PrintType( Type type ) 
* �̳y�Ƕi�h�� type �L�X������ type �W�� 
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
* �ˬd ch �O�_�� white space 
*/
bool CheckWhiteSpace(char ch)  
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
* �ˬd ch �O�_�� Delimiter 
* Delimiter : '('  ')'  '\''  WhiteSpace  
*/
bool CheckDelimiter ( char ch )
{
  if ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || 
       ch == '(' || ch == ')'  || ch == '\'' || ch == ';' || ch == '\"') 
  {
    return true ; 
  } // else if 

  return false ; 
} // IsDelimiter ( char ch )

/* IsStringStart ( char ch )
* �ˬd�O�_�� '\"'  
*/
bool IsStringStart ( char ch )
{
  if ( ch == '\"' )
    return true ; 
  return false ; 
} // IsStringStart ( char ch )

/* IsComment ( char ch )
*  �ˬd�O�_�� ';' 
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
bool IsFloat( string token ) // �}�Y�i�H�O0��??
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
*  �u�n���O INT || FLOAT || '(' || ')' || DOT || STRING || NIL || T || QUOTE 
*  �ѤU����@SYMBOL
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
*  �}�Y�O�_�O "  
*/
bool IsString ( string token )
{
  if ( token[0] == '\"' ) 
    return true ; 
  return false ; 
} // IsString 


/* IsDelimiter( string token, Type & type ) 
*  �ˬd�ODelimiter �ó]�w�L��type 
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
*  �O�_�� EOF 
*/
bool IsEOF ( char ch )
{
  if ( ch == -1 || ch == EOF )
    return true ;
  return false ; 
} // IsEOF

/* IdentifyType ( string token ) 
*  ���� token �O���� type 
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
*  ���� string Ū�X�ӡA���� ('\n') �άO�t�@�� ('\"')
*  �p�GŪ�� '\n' �S��Ū�� '\"' ���N�O error  
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
    if ( ch == '\\' ) // ����r�� \"
    {
      // �J�����r���n�� \ �R���A�ïd�U�U�@�Ӧr�� 
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
    else if ( ch == -1 )
    {
      valid = false ; 
    }
    str += ch ;
  } // while 
  
  if ( ch == '\n' || ch == -1 )
  {
    throw MyException( STRERR, gNowRow, gNowColumn ) ;   
  } // if  

  return str ; 
} // GetString ()

/* SkipComment ( ) 
* �� ; �᭱������Ū�� 
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
*  ���L�ť�Ū��Ĥ@�Ӧr��
*/ 
char GetFirstChar ( ) // skip white space to get First char 
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
  
  while ( CheckWhiteSpace(ch) == true )
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
} // GetNextChar()
 
/* GetToken ( ) 
* ���Utoken �çP�_�e�� type  
*/ 
EXP GetToken ( ) 
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

  
  if ( IsStringStart ( ch ) )
  {
    gg.token = GetString() ; 
    valid = false ; 
  } // else if 
  else if ( CheckDelimiter ( ch ) == true )
  {
    gg.token += ch ; 
    valid = false ; 
  } // if 
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
int gAfterDotCnt = 0 ;
int gAfterQuoteCnt = 0 ;  
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
  int dotCnt ;
  int quoteCnt ;
(1(a.b)()(123).(234 . 678) .( 123 ) ) ERROR
( 1 ( a . b . c ) ( 123 . 456 .678 ) . ( abc a .c ) ) ERROR
( 1 . 2 4 5 6 ) 
*/


// �C�� return false �O����N��O ??  

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
    gnum = 2 ; // list
    while( temp->type != LEFT_PAREN ) { // ���^�h 
      temp = temp->pre_next ; 
    }
    temp = temp->pre_listPtr->next ;
    S_EXP( temp ) ;
    
  }
  else if ( temp->type == RIGHT_PAREN ) {
    cout << "cc" << endl ;
    gnum = -1 ;
    throw MyException( SYNERR_ATOM_PAR, *temp ) ; 
    return false ;
  }
  else if ( IsATOM(temp) == true && gnum == 0 ) {
    cout << "dd" << endl ;
    gnum = 1 ;
    return true ;
  }
  else if ( IsATOM(temp) == true ) { ////////
    cout << "gg" << endl ;
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
      cout << "ee" << endl ;
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      throw MyException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ;
    }
    else if ( temp->pre_next != NULL && temp->pre_next->quoteCnt != 0 ) {
      cout << "ff" << endl ;
      temp->quoteCnt = temp->pre_next->quoteCnt+1 ;
      throw MyException( SYNERR_ATOM_PAR, *temp ) ;
      return false ;
    }
    else {
      if ( gnum == 5 ) { // �e���ODOT 
        cout << "hh" << endl ;
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
      }
      else if ( gnum == 4 ) { // QUOTE
        cout << "ii" << endl ;
        gAfterQuoteCnt++ ;
        temp->quoteCnt = gAfterQuoteCnt ;
      }
      cout << "jj" << endl ;
      gnum = 1 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    }

  }
  else if ( temp->type == EMPTYPTR ) { ////////
    cout << "kk" << endl ;
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
      cout << "mm" << endl ;
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      throw MyException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ;
    }
    else if ( temp->pre_next != NULL && temp->pre_next->quoteCnt != 0 ) {
      cout << "nn" << endl ;
      temp->quoteCnt = temp->pre_next->quoteCnt+1 ;
      throw MyException( SYNERR_ATOM_PAR, *temp ) ;
      return false ;
    }
    else {
      if ( gnum == 5 ) { // DOT
        cout << "pp" << endl ;
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
        
      }
      else if ( gnum == 4 ) { // QUOTE
        cout << "qq" << endl ;
        gAfterQuoteCnt++ ;
        temp->quoteCnt = gAfterQuoteCnt ;
      }
      cout << "oo" << endl ;
      gnum = 2 ;
      gAfterDotCnt = 0 ;
      gAfterQuoteCnt = 0 ;
      temp = temp->listPtr->next ;
      S_EXP( temp ) ;
    } // else

  }
  else if ( temp->type == DOT &&  ( temp->pre_next->type == EMPTYPTR || gnum == 1 ) && gnum != 5 ) {  ///
    cout << "rr" << endl ;
    EXP * forward = temp->pre_next ;
    while( forward != NULL && forward->type != DOT ){
      forward = forward->pre_next ;
    }
    if ( forward == NULL ) {
      cout << "ss" << endl ;
      gnum = 5 ;
      gAfterDotCnt = 0 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    }
    else {
      cout << "tt" << endl ;
      throw MyException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ;
    }

  }
  else if ( temp->type == DOT ) {
    cout << "uu" << endl ;
    gnum = -1 ;
    throw MyException( SYNERR_ATOM_PAR, *temp ) ;
    return false ;
  }
  else if ( temp->type == QUOTE ) { ////////////

    cout << "vv" << endl ;
    gAfterQuoteCnt = 0 ;
    gnum = 4 ;
    temp = temp->next ;
    S_EXP( temp ) ;
  }
  else {
    cout << "ww" << endl ;
    gnum = -1 ;
    throw MyException( SYNERR_ATOM_PAR, *temp ) ;
    return false ;
  }
    
} // S_EXP()

void printTab( int numOfTab ) 
{
  for ( int i = 0 ; i < numOfTab ; i ++ ) 
    cout << " " ;  
} // printTab( int numOfTab ) 

void PrintS_EXP( vector<EXP> s_exp ) 
{
  int tab = 0 ; 
  for ( int i = 0 ; i < s_exp.size ( ) ; i++ )
  {
    if ( s_exp.at ( i ).type == LEFT_PAREN )
    {
      try
      {
        if ( s_exp.at ( i - 1 ).type == LEFT_PAREN )
        {
          printTab( 0 ); 
        } // if 
        else
        {
          printTab( tab ); 
        } // else  
      } // try 
      catch ( exception ex )
      {
        printTab( 0 ) ; 
      } // catch 
      

      cout << "(" << " " ;
      tab += 2 ; 
    } // if 
    else if ( s_exp.at ( i ).type == QUOTE )
    {
      printTab( tab ) ;
      cout << "quote" << endl ; 
    } // else if 
    else if ( s_exp.at ( i ).type == RIGHT_PAREN )
    {
      tab -= 2 ; 
      printTab( tab ) ;
      cout << ")" << endl ; 
    } // else if 
    else
    {
      try
      {
        if ( s_exp.at ( i - 1 ).type == LEFT_PAREN )
        {
          printTab( 0 ); 
        } // if 
        else
        {
          printTab( tab ); 
        } // else 
      } // try 
      catch ( exception ex ) 
      {
        printTab( 0 );
      } // catch 
      
      
      cout << s_exp.at( i ).token << endl ; 
    } // else 
  } // for 

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
  ptr -> quoteCnt = 0 ;
  ptr -> dotCnt = 0 ;
  i++ ;
  return ptr ;
  
} // getValue()

void preOrderTraversal(EXP* focusNode) {
  if (focusNode != NULL) {
    std::cout << focusNode->token << " ";
    preOrderTraversal(focusNode->listPtr);
    preOrderTraversal(focusNode->next);
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

string rounding( string str ) { // �p���I��|��+�|�ˤ��J 

  stringstream ss ;
  ss << fixed << setprecision( 3 ) << atof( str.c_str() ) ;
  
  return ss.str() ;
  
} // rounding()

void fixToken( vector<EXP> & s_exp ) { // () �S�B�z 
  /*
  float�p���I�T��
  �|�ˤ��J
  t or #t �t�ΦL�X�Ӫ��O #t
  nil or () or #f �t�ΦL�X�Ӫ��Onil
  ()
  */ 
  int i = 0 ;
  while ( i < s_exp.size() ) {
    if ( s_exp.at(i).type == FLOAT ) {
      s_exp.at(i).token = rounding( s_exp.at(i).token ) ;
    } // if 
    else if ( s_exp.at(i).token == "t" ) {
      s_exp.at(i).token = "#t" ;
    } // else if 
    else if ( s_exp.at(i).token == "#f" ) {
      s_exp.at(i).token = "nil" ;
    } // else if 
    else if ( s_exp.at(i).type == LEFT_PAREN && i+1 < s_exp.size() && s_exp.at(i+1).type == RIGHT_PAREN ) {
      s_exp.at(i).token = "nil" ;
      s_exp.at(i).type = LEFT_RIGHT_PAREN ;
      s_exp.erase( s_exp.begin()+i+1 ) ;
    } // else if 

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
    gEndLine = false ; // false : ���n�p�⴫�� ; true : �}�l�p�⴫�� 
    
    cout << "> " ; 
    while ( readEXP )
    {
      
      try 
      {
        nextToken = GetToken() ; 

      } // try 
      catch ( MyException exp ) // string Error  
      {
        cout << exp.what() << endl ; 
        readEXP = false ; 
        nextToken.token = "ERROR" ;
        nextToken.type = ERROR ; 
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
      
      if ( parnum == 0 ) // �o�����O���� 
      {
        if ( nextToken.type == QUOTE )
        {
//          cout << "This is QUOTE" << endl ; 
          readEXP = true ;
        } // if 
        else if ( nextToken.type != NONE && nextToken.type != ERROR) // �٨SŪ�� EOF�A�٦���L���O�٨SŪ
        {
          
          fixToken(s_exp) ; // ��token 
          PrintS_EXP( s_exp ) ;     
//          test(s_exp) ;
//          system("pause") ;
          delete root ;  
          root = NULL ;
          i = 0 ;
          buildTree( s_exp, i ) ;
          cout << endl << "============ Tree ========" << endl ;
          preOrderTraversal(root) ;
          cout << endl << "check syntax START" << endl ;
          
          try 
          {
            gnum = 0 ;
            S_EXP( root ) ;

          } // try 
          catch ( MyException exp )
          {
            cout << exp.what() << endl ; 
          } // catch 
          
          /////IRIS
          
          cout << endl << "================================EXP DONE================================" << endl ; 
          readEXP = false ;
        } // if 
        else if ( nextToken.type == NONE ) // nextToken == NONE �N��Ū�� EOF �F�A�S��������O�F 
        {
          cout << ">>> ALL s_exp Read Done " << endl ; 
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
          cout << exp.what() << endl ; 
        } // catch 
        
      } // else if 
      
      
      
    } // while ( readEXP )

         
    if ( nextToken.type == NONE ) // Ū�� EOF
    {
      ALL_EXP_DONE = true ;
    } // if �S�� (exit) 
    else if ( s_exp.at( 0 ).token == "("  && s_exp.at( 1 ).token == "exit" && s_exp.at( 2 ).token == ")" )
    {
      ALL_EXP_DONE = true ;
    } // else if 
    
  } // while ( ALL_EXP ) 
  
  
  printf( "\nThanks for using OurScheme!" );
  return 0;
    
} // main()     
