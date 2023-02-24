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

int gNowColumn = 1 ;
int gNowRow = 1 ; 
 
#define NOT ! 

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

  ADD, // '+'
 
  SUB, // '-'
  
  MULT, // '*'
  
  DIV, // '/'

  DOT, // '.' 

  QUOTE, // '\'' 單引號

  NONE 
}; // Type 

struct EXP {
  string token ;
  int column ;
  int row ; 
  Type type ; // SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN | RIGHT-PAREN
  EXP* next ;

}; // struct EXP 

bool IsWhiteSpace(char ch) // 判斷是否為white space, 如果是 return true, 不是 return false  
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

bool IsEOF(char ch)
{
  if ( ch == -1 )
  {
    return true ; 
  } // if 

  return false ; 
} // IsEOF 

bool IsInt( string str ) // 開頭可以是0嗎???
{
  bool cont = false ; // continue ; 
  for ( int i = 0 ; i < str.size() ; i ++ ) 
  {
    if ( i == 0 && ( str[i] == '+' || str[i] == '-' ) )
    {
      cont = true ; 
    } // if 
    else if ( str[i] == '0' || str[i] == '1' || str[i] == '2' || str[i] == '3' || str[i] == '4' || 
              str[i] == '5' || str[i] == '6' || str[i] == '7' || str[i] == '8' || str[i] == '9' )
    {
      cont = true ; 
    } // if
    else  
    {
      return false ; 
    } // else 

  } // for 

  if (str.empty())
    return false ;

  return true ; 
} // IsInt(string str) 

bool IsFloat( string str ) // 開頭可以是0嗎??
{
  bool cont = false ; // continue ; 
  int numOfDot = 0 ; 

  for ( int i = 0 ; i < str.size() ; i ++ ) 
  {
    if (str[i] == '.')
    {
      numOfDot ++ ; 
    } // if 

    if ( ( str[0] == '+' || str[0] == '-' || str[0] == '.' ) && i == 0 ) 
    {
      cont = true ; 
    } // if 
    else if ( str[i] == '.' || str[i] == '0' || str[i] == '1' || str[i] == '2' || str[i] == '3' || str[i] == '4' ||
              str[i] == '5' || str[i] == '6' || str[i] == '7' || str[i] == '8' || str[i] == '9' )
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

  if (str.empty())
    return false ; 

  return true ; 
} // IsFloat

bool IsSymbol( string str )
{
  if ( str == " " ) 
  {
    cout << "Is White Space" ; 
    return false ; 
  } // if 
  else if ( str == "\0" ) 
  {
    cout << "Is NULL" ; 
    return false ; 
  } // else if
  else if (str.empty())
  {
    return false ; 
  } // else if 
  else if ( NOT IsInt(str) )
  {
    return true ; 
  } // else if 
  else if ( NOT IsFloat(str))
  {
    return true ;
  } // else if 

  return false ; 
} // IsSymbol 

bool IsSeparators(char ch)
{
  if ('(' || ')')
  {
    return true ; 
  } // if 

  return false ;
} // IsSeparators

char GetNextChar() // skip white space to get char 
{
  char ch = getchar() ; 
  while (IsWhiteSpace(ch))
  {
    ch = getchar() ; 
  } // while 

  if ( ch == EOF ) 

  return EOF ; 
} // GetNextChar()

EXP * GetToken() {

  /*
  *  separators : whitespace, '(', ')', '\'', '\"' 
  *  判斷 token TYPE
  *  
  */

  EXP *gg = new EXP() ;
  gg->token = "\0" ; 
  gg->column = 0 ; 
  gg->row = 0 ; 
  gg->type = NONE ;
  gg->next = NULL ; 

  char ch = getchar(); 
  char peek = '\0'; 
   
  bool b = false ;  // break 
  while ( NOT b && NOT IsWhiteSpace( ch ) && NOT IsEOF( ch )  ) // 不是空白就一直讀
  {
    peek = cin.peek() ; 
    
    if (IsSeparators(peek) && gg->token != "\0")
    {
      b = true ; 
    } // if 
    else if ( ch == '(' && peek == ')' ) // nil 
    {
      gg->token = "nil" ; 
      gg->type = NIL ; 
      gg->row = gNowRow ;  
      gg->column = gNowColumn ; 
      gg->next = NULL ;
      return gg ; 
    } // else if  
    else if ( ch == '(' && IsWhiteSpace( peek ) && gg->token == "\0" ) // 左括號
    {
      gg->token = ch ; 
      gg->type = LEFT_PAREN ;
      gg->row = gNowRow ; 
      gg->column = gNowColumn ; 
      gg->next = NULL ; 
      return gg ; 
    } // else if 
    else if ( ch == ')' && IsWhiteSpace( peek ) && gg->token == "\0" ) // 右括號
    {
      gg->token = ch ;
      gg->type = RIGHT_PAREN ; 
      gg->row = gNowRow ;
      gg->column = gNowColumn ;
      gg->next = NULL ;
      return gg ; 
    } // else if
    else if ( ch == '+' && IsWhiteSpace( peek ) ) // ADD
    {
      gg->token = ch ;
      gg->type = ADD ;
      gg->column = gNowColumn ;
      gg->row = gNowRow ;
      gg->next = NULL ;
      return gg ; 
    } // else if
    else if ( ch == '-' && IsWhiteSpace(peek) ) // SUB
    {
      gg->token = ch ;
      gg->type = SUB ;
      gg->row = gNowRow ;
      gg->column = gNowColumn ;
      gg->next = NULL ;
      return gg ; 
    } // else if
    else if ( ch == '*' ) // MULT
    {
      gg->token = ch ;
      gg->type = MULT ;
      gg->column = gNowColumn ;
      gg->row = gNowRow ;
      gg->next = NULL ;
      return gg ; 
    } // else if
    else if ( ch == '/' ) // DIV
    {
      gg->token = ch ;
      gg->type = DIV ;
      gg->column = gNowColumn ;
      gg->row = gNowRow ;
      gg->next = NULL ;
      return gg ; 
    } // else if
    else if ( ch == '.' && IsWhiteSpace(peek) && gg->token == "\0" ) // DOT
    {
      gg->token = ch;
      gg->type = DOT;
      gg->column = gNowColumn;
      gg->row = gNowRow ;
      gg->next = NULL;
      return gg ; 
    } // else if
    else if ( ch == ';' ) // comment 
    {
      while ( ch != '\n' ) // skip comment 
      {
        ch = getchar(); 
      } // while 
      gNowRow++; 
      // now ch == '\n' 
    } // else if 
    else if ( ch == '\'' )
    {
      gg->token = "quote";
      gg->type = QUOTE;
      gg->column = gNowColumn;
      gg->row = gNowRow ;
      gg->next = NULL;
      return gg ; 
    } // else if 
    else if (ch == '\"') // STRING 
    {
      // 讀到換行或是下一個 '\"' 
      bool getString = false ;
      bool cont = false ; // continue 
      while ( ch != '\n' && NOT getString ) 
      {
        cont = false ; 
        if ( ch == '\\' ) // 跳脫字元 \n \t \' \" 
        { 
          peek = cin.peek() ;
          cout << "Peek : " << peek; 
          if ( peek == '\'' || peek == '\"' || peek == '\\' )
          {
            ch = getchar() ; // get ' " \ 
            gg->token += ch ; 
            ch = getchar() ; // get next char 
            cont = true ; 
          } // if
          else if (peek == 'n')
          {
            ch = getchar() ; // get n 
            gg->token += '\n' ; 
            ch = getchar() ; // get next char 
            cont = true ; 
          } // else if 
          else if (peek == 't')
          {
            ch = getchar() ; // get t
            gg->token += '\t' ; 
            ch = getchar() ; // get next char 
            cont = true ; 
          } // else if 

        } // if 

        if ( cont == false )
        {
          gg->token += ch ; 
          
          ch = getchar() ; 

          if ( ch == '\"' )
          {
            gg->token += ch ; 
            getString = true ; 
          } // if  
        } // if (cont == false)
      } // while ( ch != '\n' && NOT getString ) 

      if ( getString )
      {
        gg->column = gNowColumn ; 
        gg->row = gNowRow ; 
        gg->type = STRING ; 
        gg->next = NULL ; 
        return gg ; 
      } // if 
      else if ( NOT getString )
      {
        gg->token = "ERROR Not a String" ; 
        gg->row = gNowRow ; 
        gg->column = gNowColumn ; 
        gg->type = NONE ; 
        gg->next = NULL ; 
        return gg ; 
      } // else if
    } // else  if (ch == '\"') // STRING 
    else // other char 
    {
      gg->token += ch ; 
    } // else other char 


    if ( NOT b )
    {
      ch = getchar(); 
      gNowColumn ++ ; 
    } // if 


  } // while 

  if ( gg->token == "#f" || gg->token == "nil" ) // NIL 
  {
    gg->token = "nil" ; 
    gg->type = NIL ; 
    gg->row = gNowRow ;  
    gg->column = gNowColumn ; 
    gg->next = NULL ;
    return gg ; 
  } // if 
  else if ( gg->token == "#t" || gg->token == "t" ) // T
  {
    gg->token = "#t" ;
    gg->type = T ;
    gg->row = gNowRow ;
    gg->column = gNowColumn ;
    gg->next = NULL ;
    return gg ;
  } // else if 
  else if ( IsInt( gg->token ) )
  {
    gg->type = INT ;
    gg->row = gNowRow ;
    gg->column = gNowColumn ;
    gg->next = NULL ;
    return gg ;

  } // else if 
  else if ( IsFloat( gg->token ) )
  {
    gg->type = FLOAT ;
    gg->row = gNowRow ;
    gg->column = gNowColumn ;
    gg->next = NULL ;
    return gg ;

  } // else if 
  else if ( IsSymbol( gg->token ) )
  {
    gg->type = SYMBOL ; 
    gg->row = gNowRow ;
    gg->column = gNowColumn ;
    gg->next = NULL ;
    return gg ;

  } // else if 

  cout << "Get token Lose Something Type : " << gg->token ; 

  return gg ; 
  
} // getToken() 

int main() {


  EXP * instruction = new EXP() ;
  EXP * head = instruction; 
  EXP * temp = instruction;

  EXP *input ;

  cout << "Welcome to OurScheme!" << endl ;

  input = GetToken() ; // 讀取第一個 token 

  while ( input->token != "(exit)" || gIsEOF == false ) {

    if (input->token == "\0")
    {
      cout << "NULL" ;
    } // if 
    cout << "> " << input->token << ", Type :" << input->type << endl << endl;

    input = GetToken() ; // 讀取下一個 token  
    
  } // while 
  
  if ( gIsEOF == true ) {
    cout << "ERROR(no more input:END-OF-FILE encountered)" ;
  } // if  ( gIsEOF == true ) 
  
  cout << endl << "Thanks for using OurScheme!" ;
  
} // main() 

