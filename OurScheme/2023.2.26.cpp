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

  QUOTE, // '\'' ��޸�

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

}; // struct EXP 

// 2023/02/25 �W�Ťj�Ϊηs�W�o�ǵ{���X

EXP * head = NULL ;

/* PrintType( Type type ) 
* �̳y�Ƕi�h�� type �L�X������ type �W�� 
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

  return "ERROR TYPE" ; 
} // PrintType ( Type type ) 


/* CheckWhiteSpace(char ch) 
* �ˬd ch �O�_�� white space 
*/
bool CheckWhiteSpace(char ch) // �P�_�O�_��white space, �p�G�O return true, ���O return false  
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
* �ˬd ch �O�_�� Delimiter 
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
*  �p�GŪ�� '\n' �S��Ū�� '\"' ���N�O error  <----------------------------------------- �o���٨S������ 
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
    if ( ch == '\\' ) // ����r�� \"
    {
      // �J�����r���n�� \ �R���A�ïd�U�U�@�Ӧr�� 
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
* �� ; �᭱������Ū�� 
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
*  ���L�ť�Ū��Ĥ@�Ӧr��
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
* ���Utoken �çP�_�e�� type  
*/ 
EXP * GetToken ( ) {
  // (1 . (2 . (3 . nil)))
  // ���ιJ�쪺�Ĥ@��token �çP�_�e��type 

  EXP * gg = new EXP() ;
  gg->token = "\0" ; 
  gg->column = 0 ; 
  gg->row = 0 ; 
  gg->type = NONE ;
  gg->next = NULL ; 

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

bool IsATOM( EXP * temp ){ /////////////////��L��/////////////////////////////////  
  if ( temp->type == SYMBOL || temp->type == INT  || temp->type == FLOAT  || temp->type == STRING  || temp->type == NIL || temp->type == T )
    return true ;
  else if ( temp->type == LEFT_PAREN && temp->next != NULL && temp->next->type == RIGHT_PAREN )
    return true ;
  else
    return false ;
} // IsATOM()


bool S_EXP( EXP * temp ) { 
/*
<S-exp> ::= <ATOM> 
            | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
            | QUOTE <S-exp>
            
<ATOM>  ::= SYMBOL | INT | FLOAT | STRING 
            | NIL | T | LEFT-PAREN RIGHT-PAREN
*/

// (1 2.(3 4))
// (1 2 3)
// (1 . (2 . (3 . 4)))
// (t . nil . (1 2 3))
  cout << temp->token << endl ;
  if ( temp == NULL ) {
    return false ;
  }
  else if ( IsATOM( temp ) == true ) {
    return true ;
  } 
  else if ( temp->type == LEFT_PAREN ) { // LEFT-PAREN
    temp = temp->next ;
    if ( S_EXP( temp ) == true ) { // <S-exp>
      temp = temp->next ; // { <S-exp> } or [ DOT <S-exp> ] or RIGHT-PAREN

      while ( S_EXP( temp ) == true ) { // �s�Φh�� S-exp { <S-exp> }
        temp = temp->next ;
      }
      if ( temp->type == DOT ) {
        temp = temp->next ; // (
        if ( S_EXP( temp ) == true ) {
          temp = temp->next ;
          if ( temp->type == RIGHT_PAREN ) {
            return true ;
          }
          else {
            return false ;
          }
        }

      } // if
      else if ( temp->type == RIGHT_PAREN ) {
        return true ;
      } // else if 
      else {
        return false ;
      } // else       
      
    }
    
    else{
      return false ;
    }
  } 
  else if ( temp->type == QUOTE ) {
    temp = temp->next ; 
    if ( S_EXP( temp ) == true ) {
      return true ;
    }
    else {
      return false ;
    }
  } 
  else{
    return false ;
  }
    
} // S_EXP()

int main() {

  EXP *input ;
  EXP *temp = NULL ;

  cout << "Welcome to OurScheme!" << endl ;

  input = GetToken() ; // Ū���Ĥ@�� token 

  while ( NOT gExit && NOT gIsEOF ) 
  {
//    cout << "aaa" << endl ;
    if ( gNumOfParen == 0 ) { // statement �����F 
      delete head ;
      head = new EXP() ;
      temp = head ;
    }

    temp = input ;
    
    temp->next = new EXP() ;
    temp = temp->next ;
    if ( input->token == "(" ) 
       gNumOfParen++ ;
    else if ( input->token == ")" ) 
       gNumOfParen-- ;
       
       
    cout << "> " << input->token << endl ;
    input = GetToken() ; // Ū���U�@�� token    
  } // while 
  
  S_EXP(head) ;
  
  if ( gIsEOF == true ) {
    cout << "ERROR(no more input:END-OF-FILE encountered)" ;
  } // if  ( gIsEOF == true ) 
  
  cout << endl << "Thanks for using OurScheme!" ;


  
} // main() 

