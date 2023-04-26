# include <map>
# include <cstdio>
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
  
  EXIT, EMPTYPTR, NONE, ERROR,
  
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
  
  IF, COND, CLEAN_ENVIRONMENT,
  
  LET, LAMBDA, LAMBDA_FUNC
   
}; // Type 
 
enum FuncType
{
  TYPEDEFINE, // procedure ...
  TYPELAMBDA, // procedure lambda 
  TYPENONE // symbol  
};

struct EXP {
  string token ;
  int column ;
  int row ; 
  int nowRow ; 
  Type type ; 
  EXP* next ;
  EXP* pre_next ;
  EXP* listPtr ;
  EXP* pre_listPtr ;
  int dotCnt ;
  vector<EXP> vec ; 
  int memSpace ;

}; // struct EXP 

EXP * gRoot = NULL ;
EXP * gHead = NULL ;

struct MAP {
  string str ; // symbol or function name
  vector<string> par_name ; 
  vector<EXP> vec ;
  FuncType funcType ;
};

void InitExp( EXP & ex )
{
  ex.token = "\0";
  ex.column = 0 ;
  ex.row = 0 ; 
  ex.nowRow = 0 ; // �������Ħr�����
  ex.type = NONE ; 
  ex.next = NULL ;
  ex.pre_next = NULL ;
  ex.listPtr = NULL ;
  ex.pre_listPtr = NULL ;
  ex.dotCnt = 0 ;
  ex.memSpace = 0 ;

} // InitExp() 

string PrettyString( vector<EXP> exp ) ; 

void FixSystemPrimitiveAndNil( vector<EXP> &s_exp ) ; 

string ToUppder( string str )
{
  for ( int i = 0 ; i < str.length() ; ++i ) {

    if ( str.at( i ) >= 'a' && str.at( i ) <= 'z' ) {

      str.at( i ) = str.at( i ) - 'a' + 'A';

    } // if 

  } // for 


  return str ; 
} // ToUppder() 

enum ExceptionType 
{ 
  NOTHING,  
  
  SYNERR_ATOM_PAR,   
  
  SYNERR_RIGHTPAREN   
  
}; // Enum ExceptionType 

class SyntaxErrorException
{
  string mErrMsg ;

  public : 
  string What()
  {
    return mErrMsg ;       
  } // What() 

  SyntaxErrorException( ExceptionType ErrType, EXP token ) 
  {
    mErrMsg = "" ;
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
  string mErrMsg ;
   
  public : 
  string What()
  {
    return mErrMsg ;       
  } // What() 
  
  NotAStringException( int line, int column ) 
  {
    mErrMsg = "" ;
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

} ; // NotAStringException

class EofException // read EOF 
{
  string mErrMsg ;

  public : 
  string What()
  {
    return mErrMsg ;       
  } // What() 

  EofException() 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (no more input) : END-OF-FILE encountered" ;
    mErrMsg = ss.str() ;   
  } // EofException() 

}; // EofException


/*
  Error : 
  ! ERROR (unbound symbol) : 
  ! ERROR (non-list) : 
  ! ERROR (incorrect number of arguments) : cons
  ! ERROR (car with incorrect argument type) : 3
  ! ERROR (attempt to apply non-function) : 3
  ! ERROR (division by zero) : / 
  ERROR (DEFINE format) : 
  ERROR (no return value) : 
  ERROR (COND format) : 
*/

class UnboundException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  UnboundException( EXP * token )  
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (unbound symbol) : " << token->token << endl ;
    mErrMsg = ss.str() ;   
  } // UnboundException() 

}; // UnboundException

class NonListException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  NonListException( vector<EXP> exp )  
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (non-list) : " << PrettyString( exp ) ;
    mErrMsg = ss.str() ;   
  } // NonListException() 
}; // NonListException 

class IncorrectArgumentException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  IncorrectArgumentException( string func, EXP exp )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (" << func << " with incorrect argument type) : " << exp.token << endl ;
    mErrMsg = ss.str() ;   
  } // IncorrectArgumentException() 

  IncorrectArgumentException( string func2, vector<EXP> exp2 )  // vector exp2
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (" << func2 << " with incorrect argument type) : " << PrettyString( exp2 ) ;
    mErrMsg = ss.str() ;   
  } // IncorrectArgumentException()

}; // IncorrectArgumentException

class IncorrectNumberException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  IncorrectNumberException( string func )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (incorrect number of arguments) : " << func << endl ;
    mErrMsg = ss.str() ;   
  } // IncorrectNumberException() 

}; // IncorrectNumberException

class NonFunctionException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  NonFunctionException( string token )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (attempt to apply non-function) : " << token << endl ;
    mErrMsg = ss.str() ;   
  } // NonFunctionException() 

  NonFunctionException( vector<EXP> exp )  // EXP exp 
  {
    mErrMsg = "" ;
    FixSystemPrimitiveAndNil( exp ) ; 
    stringstream ss ;
    ss << "ERROR (attempt to apply non-function) : " << PrettyString( exp ) ;
    mErrMsg = ss.str() ;   
  } // NonFunctionException() 

}; // NonFunctionException

class DivByZeroException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  DivByZeroException()  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (division by zero) : /" << endl ;
    mErrMsg = ss.str() ;   
  } // DivByZeroException()

}; // DivByZeroException

class DefineFormatException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  DefineFormatException( vector<EXP> exp )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (DEFINE format) : " << PrettyString( exp ) ; 
    mErrMsg = ss.str() ;   
  } // DefineFormatException()

}; // DefineFormatException

class LambdaFormatException
{
  string mErrMsg ;
public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  LambdaFormatException( vector<EXP> exp )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (LAMBDA format) : " << PrettyString( exp ) ; 
    mErrMsg = ss.str() ;   
  } // LambdaFormatException()

}; // LambdaFormatException

class CondFormatException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  CondFormatException( vector<EXP> exp )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (COND format) : " << PrettyString( exp ) ; 
    mErrMsg = ss.str() ;   
  } // CondFormatException()

}; // CondFormatException

class ErrorLevelException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  ErrorLevelException( string token )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (level of " << ToUppder( token ) << ")" << endl ;  
    mErrMsg = ss.str() ;   
  } // ErrorLevelException()

}; // ErrorLevelException

class NoReturnException
{ 
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  NoReturnException( vector<EXP> exp )  // EXP exp 
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "ERROR (no return value) : " << PrettyString( exp ) ; 
    mErrMsg = ss.str() ;   
  } // NoReturnException()

}; // NoReturnException

class ExitException
{
  string mErrMsg ;
  public:
  string What()
  {
    return mErrMsg ;       
  } // What() 

  ExitException( string token )  
  {
    mErrMsg = "" ;
    stringstream ss ;
    ss << "EXIT" ;  
    mErrMsg = ss.str() ;   
  } // ExitException()


}; // ExitException

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
  else if ( type == EMPTYPTR )
    return "EMPTY" ; 

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

Type IdentifyType( string token )
{
  Type type = NONE ; 

  if ( token == "\0" )
  {
    return NONE ; 
  } // if 
  else if ( token == "exit" )
  {
    return EXIT ; 
  } // else if 
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

string GetString()
{
  string str = "" ; 
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

    throw new NotAStringException( line, gNowColumn ) ;
      
  } // if  
  else if ( ch == -1 )
  {
    throw new NotAStringException( line, gNowColumn ) ; 

  } // else if   

  return str ; 
} // GetString()


void SkipComment()
{
  char ch = '\0' ;
  do
  {
    ch = getchar() ;
  } while ( ch != '\n' && ch != -1 && ch != EOF ) ; 

  if ( ch == -1 || ch == EOF )
  {
    throw new EofException() ; 
  } // if 

  gNowRow ++ ; 
  // cout << endl << "  COMMENT ADD LINE  " << gNowRow ;  
  gNowColumn = 0 ; 
} // SkipComment() 


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
 

EXP GetToken() 
{
  // (1 . (2 . (3 . nil)))
  // ���ιJ�쪺�Ĥ@��token �çP�_�e��type 

  EXP gg ; 
  InitExp( gg ) ; 

  char ch = GetFirstChar() ;
  
  char peek = '\0' ;  
  bool valid = true ; // true : ���O delimiter string EOF�A false : �N���i��O delimiter string EOF 
  
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
    throw new EofException() ; 
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
  else if ( ex.type == EXIT || ex.type == CONS || ex.type == LIST || ex.type == QUOTE || 
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
            ex.type == CLEAN_ENVIRONMENT || 
            ex.type == LET || ex.type == LAMBDA ) {
              
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
  else if ( temp->type == EXIT || temp->type == CONS || temp->type == LIST || temp->type == QUOTE || 
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
            temp->type == CLEAN_ENVIRONMENT || 
            temp->type == LET || temp->type == LAMBDA ) {
              
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
  
  if ( temp == NULL && gnum == 2 ) {
                                  
    gnum = 8888 ;
    return true ;
  } // if
  else if ( temp == NULL && gRoot->type == QUOTE ) { 
                                  
    return true ;
  } // else if
  else if ( temp != NULL && temp->type == RIGHT_PAREN && ( gnum == 1 || gnum == 2 ) ) { 
                                  
    gnum = 2 ; // list
    while ( temp->type != LEFT_PAREN ) { 
      temp = temp->pre_next ; 
    } // while
    
    temp = temp->pre_listPtr->next ;
    S_EXP( temp ) ;
    
  } // else if 
  else if ( temp->type == RIGHT_PAREN ) {
                              
    gnum = -1 ;
    throw new SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ; 
    return false ; 
  } // else if
  else if ( IsATOM( temp ) == true && gnum == 0 && temp->next == NULL && temp->listPtr == NULL ) {
                                
    gnum = 1 ;
    return true ;
  } // else if
  else if ( IsATOM( temp ) == true ) { 
                                 
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
                                  
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      throw new SyntaxErrorException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // ex: . 3 3 
    } // if
    else {
      if ( gnum == 5 ) { // DOT 
                                        
        gAfterDotCnt++ ;
        temp->dotCnt = gAfterDotCnt ;
      } // if
      
      gnum = 1 ;
      temp = temp->next ;
      S_EXP( temp ) ;
    } // else 

  } // else if
  else if ( temp->type == EMPTYPTR ) { 
                                
    if ( temp->pre_next != NULL && temp->pre_next->dotCnt != 0 ) {
                                  
      temp->dotCnt = temp->pre_next->dotCnt+1 ;
      temp = temp->listPtr ; 
      throw new SyntaxErrorException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // ex: . (1) (1)  
    } // if 
    else {
                                    
      if ( gnum == 5 ) { // DOT
                                        
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
                              
    throw new SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
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
                                  
      throw new SyntaxErrorException( SYNERR_RIGHTPAREN, *temp ) ;
      return false ; // ex: (1 . 3 . 3 ) 
    } // else 

  } // else if
  else if ( temp->type == DOT ) {  
                              
    gnum = -1 ;
    throw new SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
    return false ;
  } // else if
  else if ( temp->type == QUOTE ) { 
    //    cout << "xx" << endl ;
    gnum = 4 ;
    temp = temp->next ;
    S_EXP( temp ) ;
  } // else if
  else {
                              
    gnum = -1 ;
    throw new SyntaxErrorException( SYNERR_ATOM_PAR, *temp ) ;
    return false ;
  } // else 

  return true ;     
} // S_EXP()

 
void PrintTab( int numOfTab ) 
{
  for ( int i = 0 ; i < numOfTab ; i ++ ) 
    cout << " " ;  
} // PrintTab() 

void DeleteDotParen( vector<EXP> & s_exp )
{
  int parnum = 0 ;
  int nowRow = s_exp.at( s_exp.size() - 1 ).nowRow ; 
  int row = s_exp.at( s_exp.size() - 1 ).row ;
  int col = s_exp.at( s_exp.size() - 1 ).column ; 
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


  s_exp.at( s_exp.size() - 1 ).nowRow = nowRow ; 
  s_exp.at( s_exp.size() - 1 ).row = row ; 
  s_exp.at( s_exp.size() - 1 ).column = col ; 
} // DeleteDotParen()

bool PrintS_EXP( vector<EXP> s_exp ) 
{
  int tab = 0 ; 
  int parnum = 0 ;  
  // (1 . (2 . (3 . 4))) --> ( 1 2 3 . 4 ) 
  for ( int i = 0 ; i < s_exp.size() ; i++ )
  {
    // cout << "Line :" << s_exp.at( i ).nowRow << " " ;
    if ( s_exp.at( i ).type == LEFT_PAREN )
    {
      parnum ++ ; 

      if ( i >= 1 && s_exp.at( i - 1 ).type == LEFT_PAREN )
      {
        PrintTab( 0 );  
      } // if 
      else if ( i == 0 )
      {
        PrintTab( 0 );  
      } // else if 
      else
      {
        PrintTab( tab );
      } // else 
  
      cout << "(" << " " ;
      tab += 2 ; 
    } // if 
    else if ( s_exp.at( i ).type == QUOTE )
    {
      if ( i >= 1 && s_exp.at( i - 1 ).type == LEFT_PAREN )
      {
        PrintTab( 0 );  
      } // if 
      else if ( i == 0 )
      {
        PrintTab( 0 );  
      } // else if 
      else
      {
        PrintTab( tab );
      } // else 

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
      if ( i >= 1 && s_exp.at( i - 1 ).type == LEFT_PAREN )
      {
        PrintTab( 0 );  
      } // if 
      else if ( i == 0 )
      {
        PrintTab( 0 );  
      } // else if 
      else
      {
        PrintTab( tab );
      } // else 

        
      cout << s_exp.at( i ).token << endl ; 
    } // else 
  } // for 
  
  return true ;
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
  DeleteDotParen( s_exp ) ;

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

      if ( i >= 1 && s_exp.at( i - 1 ).type == LEFT_PAREN )
      {
        str += AddTab( 0 ); 
      } // if 
      else if ( i == 0 )
      {
        str += AddTab( 0 ); 
      } // else if 
      else
      {
        str += AddTab( tab ); 
      } // else


      str += "( " ;  
      tab += 2 ; 
    } // if 
    else if ( s_exp.at( i ).type == QUOTE )
    {
      if ( i >= 1 && s_exp.at( i - 1 ).type == LEFT_PAREN )
      {
        str += AddTab( 0 ); 
      } // if 
      else if ( i == 0 )
      {
        str += AddTab( 0 ); 
      } // else if 
      else
      {
        str += AddTab( tab ); 
      } // else

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
      if ( i >= 1 && s_exp.at( i - 1 ).type == LEFT_PAREN )
      {
        str += AddTab( 0 ); 
      } // if 
      else if ( i == 0 )
      {
        str += AddTab( 0 ); 
      } // else if 
      else
      {
        str += AddTab( tab ); 
      } // else

      str += s_exp.at( i ).token ; 
      str += "\n" ; 
    } // else 
  } // for 

  return str ; 
} // PrettyString()

EXP *GetValue( vector<EXP> vec, int &i ) {
  EXP * ptr = new EXP() ;
  InitExp( *ptr ) ; 
  ptr -> token = vec.at( i ).token ; 
  ptr -> type = vec.at( i ).type ;
  ptr -> column = vec.at( i ).column ;
  ptr -> row = vec.at( i ).row ;
  ptr -> next = NULL ;
  ptr -> pre_next = NULL ;
  ptr -> listPtr = NULL ;
  ptr -> pre_listPtr = NULL ;
  ptr -> dotCnt = 0 ;
  ptr -> nowRow = 0 ; 
  ptr -> memSpace = 0 ; 
  i++ ;
  return ptr ;
  
} // GetValue()

void PreOrderTraversal( EXP* focusNode ) {

  if ( focusNode != NULL ) {
    
    if ( focusNode->type == EMPTYPTR ) {
      if ( NOT focusNode->vec.empty() )
      {
        cout << " [ " ; 
        for ( int i = 0; i < focusNode->vec.size() ; i++ ) {
          cout << focusNode->vec.at( i ).token << " ";
        } // for
        
        cout << "] " ;  
      } // if
      else 
      {
        cout << "EMPTY " ;  
      } // else
      
    } // if
    else 
    {
      cout << focusNode->token << " ";
    } // else 

    PreOrderTraversal( focusNode->listPtr ) ;
    PreOrderTraversal( focusNode->next ) ;
  } // if 
  
} // PreOrderTraversal()

void GetPreOrderTraversal( EXP* focusNode, vector<EXP> &vec ) {

  if ( focusNode != NULL ) {
    vec.push_back( *focusNode ) ;
    GetPreOrderTraversal( focusNode->listPtr, vec ) ;
    GetPreOrderTraversal( focusNode->next, vec ) ;
  } // if 

  
} // GetPreOrderTraversal()

void GetPreOrderTraversalWithNoEmpty( EXP* focusNode, vector<EXP> &vec ) {

  if ( focusNode != NULL ) {
    if ( focusNode->type != EMPTYPTR )
      vec.push_back( *focusNode ) ;
    GetPreOrderTraversalWithNoEmpty( focusNode->listPtr, vec ) ;
    GetPreOrderTraversalWithNoEmpty( focusNode->next, vec ) ;
  } // if 


} // GetPreOrderTraversalWithNoEmpty()

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

EXP * DynamicBuildTree( vector<EXP> s_exp, int &i ) { // project3 
  EXP * temp = NULL ;
  EXP * root = NULL ;
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
      if ( root == NULL ) {
        root = new EXP() ;
        root->type = EMPTYPTR ;
        root->token = "XXEMPTYXX" ;
        root->next = NULL ;
        root->listPtr = GetValue( s_exp, i ) ;
        root->listPtr->pre_listPtr = root ; 
        temp = root->listPtr ;
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
      if ( root == NULL ) {
        root = GetValue( s_exp, i ) ; 
        temp = root ;
      } // if
      else {
        temp->next = GetValue( s_exp, i ) ; 
        temp->next->pre_next = temp ;
        temp = temp->next ;        
      } // else

    } // else 
    

  } // while
  
  return root ;
    
  
} // DynamicBuildTree()
  

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

string FloatToString( float value ) {
  stringstream ss ;
  ss << value ;
  return ss.str() ;
} // FloatToString()

void FixToken( vector<EXP> & s_exp ) {

  int i = 0 ;

  int nowRow = s_exp.at( s_exp.size() - 1 ).nowRow ; 
  int row = s_exp.at( s_exp.size() - 1 ).row ; 
  int col = s_exp.at( s_exp.size() - 1 ).column ; 
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
    else if ( s_exp.at( i ).token == "define" ) 
      s_exp.at( i ).type = DEFINE ;
    else if ( s_exp.at( i ).token == "cons" ) 
      s_exp.at( i ).type = CONS ;
    else if ( s_exp.at( i ).token == "list" ) 
      s_exp.at( i ).type = LIST ;
    else if ( s_exp.at( i ).token == "\'" || s_exp.at( i ).token == "quote" ) 
      s_exp.at( i ).type = QUOTE ; 
    else if ( s_exp.at( i ).token == "car" ) 
      s_exp.at( i ).type = CAR ;
    else if ( s_exp.at( i ).token == "cdr" ) 
      s_exp.at( i ).type = CDR ;
    else if ( s_exp.at( i ).token == "atom?" ) 
      s_exp.at( i ).type = ATOM_QMARK ;
    else if ( s_exp.at( i ).token == "pair?" ) 
      s_exp.at( i ).type = PAIR_QMARK ;
    else if ( s_exp.at( i ).token == "list?" ) 
      s_exp.at( i ).type = LIST_QMARK ;
    else if ( s_exp.at( i ).token == "null?" ) 
      s_exp.at( i ).type = NULL_QMARK ;
    else if ( s_exp.at( i ).token == "integer?" ) 
      s_exp.at( i ).type = INTEGER_QMARK ;
    else if ( s_exp.at( i ).token == "real?" ) 
      s_exp.at( i ).type = REAL_QMARK ;
    else if ( s_exp.at( i ).token == "number?" ) 
      s_exp.at( i ).type = NUMBER_QMARK ;
    else if ( s_exp.at( i ).token == "string?" ) 
      s_exp.at( i ).type = STRING_QMARK ;
    else if ( s_exp.at( i ).token == "boolean?" ) 
      s_exp.at( i ).type = BOOLEAN_QMARK ;
    else if ( s_exp.at( i ).token == "symbol?" ) 
      s_exp.at( i ).type = SYMBOL_QMARK ; 
    else if ( s_exp.at( i ).token == "+" ) 
      s_exp.at( i ).type = ADD ;
    else if ( s_exp.at( i ).token == "-" ) 
      s_exp.at( i ).type = SUB ; 
    else if ( s_exp.at( i ).token == "*" ) 
      s_exp.at( i ).type = MULT ;
    else if ( s_exp.at( i ).token == "/" ) 
      s_exp.at( i ).type = DIV ;
    else if ( s_exp.at( i ).token == "not" ) 
      s_exp.at( i ).type = NOTT ; 
    else if ( s_exp.at( i ).token == "and" ) 
      s_exp.at( i ).type = AND ;
    else if ( s_exp.at( i ).token == "or" ) 
      s_exp.at( i ).type = OR ;
    else if ( s_exp.at( i ).token == ">" ) 
      s_exp.at( i ).type = BIGGERTHAN ;
    else if ( s_exp.at( i ).token == ">=" ) 
      s_exp.at( i ).type = BIGGEREQUAL ;
    else if ( s_exp.at( i ).token == "<" ) 
      s_exp.at( i ).type = LESSTHAN ;
    else if ( s_exp.at( i ).token == "<=" ) 
      s_exp.at( i ).type = LESSEQUAL ;
    else if ( s_exp.at( i ).token == "=" ) 
      s_exp.at( i ).type = EQUAL ;
    else if ( s_exp.at( i ).token == "string-append" ) 
      s_exp.at( i ).type = STRING_APPEND ;
    else if ( s_exp.at( i ).token == "string>?" ) 
      s_exp.at( i ).type = STRING_BIGGER ;
    else if ( s_exp.at( i ).token == "string<?" ) 
      s_exp.at( i ).type = STRING_LESS ;
    else if ( s_exp.at( i ).token == "string=?" ) 
      s_exp.at( i ).type = STRING_EQUAL ;
    else if ( s_exp.at( i ).token == "eqv?" ) 
      s_exp.at( i ).type = EQV_QMARK ; 
    else if ( s_exp.at( i ).token == "equal?" ) 
      s_exp.at( i ).type = EQUAL_QMARK ;
    else if ( s_exp.at( i ).token == "begin" ) 
      s_exp.at( i ).type = BEGIN ;
    else if ( s_exp.at( i ).token == "if" ) 
      s_exp.at( i ).type = IF ;
    else if ( s_exp.at( i ).token == "cond" ) 
      s_exp.at( i ).type = COND ;
    else if ( s_exp.at( i ).token == "clean-environment" ) 
      s_exp.at( i ).type = CLEAN_ENVIRONMENT ;
    else if ( s_exp.at( i ).token == "exit" ) 
      s_exp.at( i ).type = EXIT ;
    else if ( s_exp.at( i ).token == "let" )
      s_exp.at( i ).type = LET ;
    else if ( s_exp.at( i ).token == "lambda" )
      s_exp.at( i ).type = LAMBDA ;

    i++ ;
  } // while
  

  s_exp.at( s_exp.size() - 1 ).nowRow = nowRow ; 
  s_exp.at( s_exp.size() - 1 ).row = row ; 
  s_exp.at( s_exp.size() - 1 ).column = col ; 
} // FixToken()

void FixQuote( vector<EXP> & s_exp ) { // '(1 '4) , '(1), (1 '2 4 5)
  int i = 0 ;
  EXP temp ;
  InitExp( temp ) ; 
  int parnum = 0 ;

  int nowRow = s_exp.at( s_exp.size() - 1 ).nowRow ; 
  int row = s_exp.at( s_exp.size() - 1 ).row ; 
  int col = s_exp.at( s_exp.size() - 1 ).column ; 
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


  s_exp.at( s_exp.size() - 1 ).nowRow = nowRow ; 
  s_exp.at( s_exp.size() - 1 ).row = row ; 
  s_exp.at( s_exp.size() - 1 ).column = col ; 
} // FixQuote()

struct DotCheck 
{
  Type type ; 
  bool isCheck ; 
  int leftParNum ; 
} ; // DotCheck 

void InitDotCheck( DotCheck & in )
{
  in.type = NONE ; 
  in.isCheck = false ; 
  in.leftParNum = 0 ; 
} // InitDotCheck()

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

void FixSystemPrimitiveAndNil( vector<EXP> &s_exp ) {
  int i = 0 ;
  while ( i < s_exp.size() ) {
    if ( s_exp.at( i ).type == DEFINE ) 
      s_exp.at( i ).token = "#<procedure define>" ;
    else if ( s_exp.at( i ).type == CONS ) 
      s_exp.at( i ).token = "#<procedure cons>" ;
    else if ( s_exp.at( i ).type == LIST ) 
      s_exp.at( i ).token = "#<procedure list>" ;
    else if ( s_exp.at( i ).type == QUOTE ) 
      s_exp.at( i ).token = "#<procedure quote>" ;
    else if ( s_exp.at( i ).type == CAR ) 
      s_exp.at( i ).token = "#<procedure car>" ;
    else if ( s_exp.at( i ).type == CDR ) 
      s_exp.at( i ).token = "#<procedure cdr>" ;
    else if ( s_exp.at( i ).type == ATOM_QMARK ) 
      s_exp.at( i ).token = "#<procedure atom?>" ;
    else if ( s_exp.at( i ).type == PAIR_QMARK ) 
      s_exp.at( i ).token = "#<procedure pair?>" ;
    else if ( s_exp.at( i ).type == LIST_QMARK ) 
      s_exp.at( i ).token = "#<procedure list?>" ;
    else if ( s_exp.at( i ).type == NULL_QMARK ) 
      s_exp.at( i ).token = "#<procedure null?>" ;
    else if ( s_exp.at( i ).type == INTEGER_QMARK ) 
      s_exp.at( i ).token = "#<procedure integer?>" ;
    else if ( s_exp.at( i ).type == REAL_QMARK ) 
      s_exp.at( i ).token = "#<procedure real?>" ;
    else if ( s_exp.at( i ).type == NUMBER_QMARK ) 
      s_exp.at( i ).token = "#<procedure number?>" ;
    else if ( s_exp.at( i ).type == STRING_QMARK ) 
      s_exp.at( i ).token = "#<procedure string?>" ;
    else if ( s_exp.at( i ).type == BOOLEAN_QMARK ) 
      s_exp.at( i ).token = "#<procedure boolean?>" ; 
    else if ( s_exp.at( i ).type == SYMBOL_QMARK ) 
      s_exp.at( i ).token = "#<procedure symbol?>" ;
    else if ( s_exp.at( i ).type == ADD ) 
      s_exp.at( i ).token = "#<procedure +>" ;
    else if ( s_exp.at( i ).type == SUB ) 
      s_exp.at( i ).token = "#<procedure ->" ;
    else if ( s_exp.at( i ).type == MULT ) 
      s_exp.at( i ).token = "#<procedure *>" ;
    else if ( s_exp.at( i ).type == DIV ) 
      s_exp.at( i ).token = "#<procedure />" ;
    else if ( s_exp.at( i ).type == NOTT ) 
      s_exp.at( i ).token = "#<procedure not>" ;
    else if ( s_exp.at( i ).type == AND ) 
      s_exp.at( i ).token = "#<procedure and>" ;
    else if ( s_exp.at( i ).type == OR ) 
      s_exp.at( i ).token = "#<procedure or>" ;
    else if ( s_exp.at( i ).type == BIGGERTHAN ) 
      s_exp.at( i ).token = "#<procedure >>" ;
    else if ( s_exp.at( i ).type == BIGGEREQUAL ) 
      s_exp.at( i ).token = "#<procedure >=>" ;
    else if ( s_exp.at( i ).type == LESSTHAN ) 
      s_exp.at( i ).token = "#<procedure <>" ;
    else if ( s_exp.at( i ).type == LESSEQUAL ) 
      s_exp.at( i ).token = "#<procedure <=>" ;
    else if ( s_exp.at( i ).type == EQUAL ) 
      s_exp.at( i ).token = "#<procedure =>" ;
    else if ( s_exp.at( i ).type == STRING_APPEND ) 
      s_exp.at( i ).token = "#<procedure string-append>" ;
    else if ( s_exp.at( i ).type == STRING_BIGGER ) 
      s_exp.at( i ).token = "#<procedure string>>" ;
    else if ( s_exp.at( i ).type == STRING_LESS ) 
      s_exp.at( i ).token = "#<procedure string<>" ;
    else if ( s_exp.at( i ).type == STRING_EQUAL ) 
      s_exp.at( i ).token = "#<procedure string=>" ;
    else if ( s_exp.at( i ).type == EQV_QMARK ) 
      s_exp.at( i ).token = "#<procedure eqv?>" ;
    else if ( s_exp.at( i ).type == EQUAL_QMARK ) 
      s_exp.at( i ).token = "#<procedure equal?>" ;
    else if ( s_exp.at( i ).type == BEGIN ) 
      s_exp.at( i ).token = "#<procedure begin>" ;
    else if ( s_exp.at( i ).type == IF ) 
      s_exp.at( i ).token = "#<procedure if>" ;
    else if ( s_exp.at( i ).type == COND ) 
      s_exp.at( i ).token = "#<procedure cond>" ;
    else if ( s_exp.at( i ).type == CLEAN_ENVIRONMENT ) 
      s_exp.at( i ).token = "#<procedure clean-environment>" ;
    else if ( s_exp.at( i ).type == EXIT ) 
      s_exp.at( i ).token = "#<procedure exit>" ;
    else if ( s_exp.at( i ).type == LAMBDA_FUNC ) 
      s_exp.at( i ).token = "#<procedure lambda>" ;
    else if ( s_exp.at( i ).type == LEFT_PAREN && i+1 < s_exp.size() 
              && s_exp.at( i + 1 ).type == RIGHT_PAREN ) {
      s_exp.at( i ).token = "nil" ;
      s_exp.at( i ).type = NIL ;
      s_exp.erase( s_exp.begin()+i+1 ) ;
      
    } // else if 
    
    i++ ;
      
  } // while
    
} // FixSystemPrimitiveAndNil() 

vector<EXP> CopyVector( vector<EXP> input )
{
  vector<EXP> out ; 
  for ( int i = 0 ; i < input.size() ; i++ )
  {
    out.push_back( input.at( i ) ) ; 
  } // for 

  return out ; 
} // CopyVector()

class Functions {
private: 
  vector<MAP> msymbolMap ;
  vector<MAP> mlocalsymbolMap ;
  vector<MAP> mLambdaSymbolMap ;
  EXP* mexeNode ; 
  vector<EXP> mnonListVec ; // used to save non-list  
  int mlevel ; 
  int memNum ;
  vector<EXP> mresult ; 

  void InsertLambdaMap( MAP m ) 
  { 
    mLambdaSymbolMap.push_back( m ) ; 
  } // InsertLambdaMap() 

  void ClearLambdaMap() 
  { 
    mLambdaSymbolMap.clear() ; 
  } // ClearLambdaMap() 

  bool SearchLambdaMap( string key, MAP & value )
  {
    
    for ( int i = 0 ; i < mLambdaSymbolMap.size() ; i++ )
    {
      if ( mLambdaSymbolMap.at( i ).str == key )
      {
        value = mLambdaSymbolMap.at( i ) ; 
        return true ; 
      } // if 
    } // for

    return false ; 

  } // SearchLambdaMap()

  void CheckLambdaFormat( EXP* now ) ; 

  bool InsertMapAndPar( string str, vector<EXP> vec, vector<string> pp, FuncType type ) { 
    // insert parameter/ sexp/ type �� global map
    int i = 0 ;
    while ( i < msymbolMap.size() ) {
      if ( msymbolMap.at( i ).str == str ) {
        msymbolMap.at( i ).vec.assign( vec.begin(), vec.end() ) ;
        msymbolMap.at( i ).par_name.assign( pp.begin(), pp.end() ) ;
        msymbolMap.at( i ).funcType = type ;
        return true ;
      } // if
      
      i++ ;
    } // while
    
    MAP mm ;
    mm.str = str ;
    mm.vec.assign( vec.begin(), vec.end() ) ;
    mm.par_name.assign( pp.begin(), pp.end() ) ;
    mm.funcType = type ;
    msymbolMap.push_back( mm ) ;
    return false ;
    
  } // InsertMapAndPar()

  bool InsertMap( string str, vector<EXP> vec ) { // return true�w�g�b�̭����� 

    int i = 0 ;
    while ( i < msymbolMap.size() ) {
      if ( msymbolMap.at( i ).str == str ) {
        msymbolMap.at( i ).vec.assign( vec.begin(), vec.end() ) ;
        return true ;
      } // if
      
      i++ ;
    } // while
    
    MAP mm ;
    mm.str = str ;
    mm.vec.assign( vec.begin(), vec.end() ) ;
    mm.funcType = TYPENONE ;
    msymbolMap.push_back( mm ) ;
    return false ;
    
  } // InsertMap()

  bool InsertLocalMap( string str, vector<EXP> vec ) { // local variable�� 
    int i = 0 ;
    while ( i < mlocalsymbolMap.size() ) {
      if ( mlocalsymbolMap.at( i ).str == str ) {
        mlocalsymbolMap.at( i ).vec.assign( vec.begin(), vec.end() ) ;
        return true ;
      } // if
      
      i++ ;
    } // while
    
    MAP mm ;
    mm.str = str ;
    mm.vec.assign( vec.begin(), vec.end() ) ;
    mlocalsymbolMap.push_back( mm ) ;
    return false ;
    
  } // InsertLocalMap()

  bool InsertFuncInGlobalMap( string str, vector<string> parV, 
                              vector<EXP> vec, FuncType type ) { // insert function

    int i = 0 ;
    while ( i < msymbolMap.size() ) {
      if ( msymbolMap.at( i ).str == str ) {
        msymbolMap.at( i ).vec.assign( vec.begin(), vec.end() ) ;
        msymbolMap.at( i ).par_name.assign( parV.begin(), parV.end() ) ;
        msymbolMap.at( i ).funcType = type ;
        return true ;
      } // if
      
      i++ ;
    } // while
    
    MAP mm ;
    mm.str = str ;
    mm.vec.assign( vec.begin(), vec.end() ) ;
    mm.par_name.assign( parV.begin(), parV.end() ) ;
    mm.funcType = type ;
    msymbolMap.push_back( mm ) ;
    return false ;
    
  } // InsertFuncInGlobalMap()
    
  int FindMemNum( string str ) {
    
    int i = 0 ;
    while ( i < msymbolMap.size() ) {
      if ( msymbolMap.at( i ).str == str ) {
        return msymbolMap.at( i ).vec.at( 0 ).memSpace ; 
      } // if
      
      i++ ;
    } // while
  
    return -1 ;
    
  } // FindMemNum()

  bool CheckNumOfArgFromParen( EXP* node, int num )
  {
    // node start at ( 
    // node -> next : firstArg 
    node = node->next ; 
    int argNum = 0 ; 
    while ( node->type != RIGHT_PAREN )
    {
      // cout << "node : " << node->token << endl ; 
      argNum ++ ; 
      node = node->next ; 
    } // while 

    if ( argNum == num )
      return true ; 
    
    return false ; 
  } // CheckNumOfArgFromParen()

  bool CheckNumOfArg( int num ) {
    // mexeNode start at firstArgument 
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

  bool IsList( vector<EXP> vec )
  {
    int i = 0 ;
    bool isList = true ; 
    int parNum = 0 ; 
    while ( i < vec.size() )
    {
      if ( vec.at( i ).type == LEFT_PAREN )
      {
        parNum ++ ; 
      } // if 
      else if ( vec.at( i ).type == RIGHT_PAREN )
      {
        parNum -- ; 
      } // else if 

      if ( parNum > 1 )
      {
        // skip ( ... ) 
        while ( parNum != 1 )
        {
          i ++ ; 
          if ( vec.at( i ).type == RIGHT_PAREN )
          {
            parNum -- ; 
          } // if 
        } // while
      } // if 
      else if ( vec.at( i ).type == DOT )
      {
        return false ; 
      } // else if 

      i ++ ; 
    } // while 

    return true ; 

  } // IsList()

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

  void FixNil( vector<EXP>& s_exp )
  {
    for ( int i = 0 ; i < s_exp.size() ; i++ )
    {
      if ( s_exp.at( i ).type == LEFT_PAREN && i+1 < s_exp.size() 
           && s_exp.at( i + 1 ).type == RIGHT_PAREN ) {
        s_exp.at( i ).token = "nil" ;
        s_exp.at( i ).type = NIL ;
        s_exp.at( i ).row = s_exp.at( i + 1 ).row ;
        s_exp.at( i ).nowRow = s_exp.at( i + 1 ).nowRow ;
        s_exp.erase( s_exp.begin()+i+1 ) ;

      } // if
    } // for 
  } // FixNil() 
  
public : 
  void SetRoot() ; 
  void ClearLocalMap() ;
  bool ThisIsFunction( string str ) ;
  void PrintMap() ;
  void Eval() ; 
  void Define() ; 
  void Cons() ; 
  bool IsSystemPrimitive( Type type ) ; 
  bool FindMap( string str, vector<EXP> &new_vector ) ; 
  bool FindLocalMap( string str, vector<EXP> &new_vector ) ;
  bool FindGlobalMap( string str, vector<EXP> &new_vector, vector<string> &ss, FuncType &type ) ;
  void Qmark( string whichQmark ) ; 
  // atom? , null? , integer? , real? , boolean?                         
  void Quote() ; 
  void Car() ; 
  void Cdr() ; 
  void Arithmetic_Add_Sub_Mul_DIV( string whichOperator ) ; // + - * /  
  void CompareNum( string whichOperator ) ; // > , >= , < , <= , = 
  void CompareString( string whichOperator ) ; // string>? , string<? , string=?
  void String_append() ; 
  void Not() ; 
  void Equal_qmark() ;
  void Eqv_qmark() ; 
  void List() ;
  void And() ; 
  void Or() ; 
  void Clean_Environment() ; 
  void ResetMemNum() ;
  void ResetLevel()
  {
    mlevel = 0 ; 
  } // ResetLevel()

  void ClearResult() 
  {
    mresult.clear() ; 
  } // ClearResult()

  void Pair_qmark() ; // pair?
  void List_qmark() ; // list?
  void Begin() ; // begin
  void Cond() ; // cond 
  void If() ; // if
  void Exit() ; 


  void Let() ; 
  void CallFunction() ;
  void CallLambdaFunction() ; 
  void Lambda() ; 
  
  void InitFunc()
  {
    mlevel = 0 ; 
    memNum = 0 ;
    msymbolMap.clear() ; 
    mexeNode = NULL ; 
    mnonListVec.clear() ; 
    mresult.clear() ; 
  } // InitFunc()
  
  void InitExeNode() {
    mexeNode = NULL ; 
  } // InitExeNode()
  
  vector<EXP> GetResult()
  {
    // cout << "mresult: " << mresult.at(0).token ;
    FixSystemPrimitiveAndNil( mresult ) ;
    return mresult ; 
  } // GetResult() 
    
  bool TorF( vector<EXP> exp ) 
  {
    if ( exp.size() == 1 && exp.at( 0 ).type == NIL )
    {
      return false ; 
    } // if 

    return true ; 
  } // TorF() 
   


  void Execute() { // ptr���bfunction call�W�� 

    EXP * emptyptr = mexeNode->pre_next->pre_listPtr ; 
    
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
    else if ( mexeNode->type == PAIR_QMARK ) 
      Pair_qmark() ; 
    else if ( mexeNode->type == LIST_QMARK )
      List_qmark() ; 
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
    else if ( mexeNode->type == BEGIN )
      Begin() ; 
    else if ( mexeNode->type == COND  )
      Cond() ;
    else if ( mexeNode->type == IF  )
      If() ;
    else if ( mexeNode->type == EXIT )
      Exit() ;
    else if ( mexeNode->type == LET ) 
      Let() ;
    else if ( mexeNode->type == LAMBDA )
      Lambda() ; 
      
    if ( NOT emptyptr->vec.empty() )
    {
      // FixToken( emptyptr->vec ) ; 
      FixNil( emptyptr->vec ) ; 
      DeleteDotParen( emptyptr->vec ) ; 
    } // if 
    
  } // Execute()

}; // Functions

void Functions::PrintMap() {
  cout << "======= LOCAL =======" << endl ;
  int i = 0 ;
  int k = 0 ;
  while ( i < mlocalsymbolMap.size() ) {
    cout << mlocalsymbolMap.at( i ).str << ": " ;
    k = 0 ;
    while ( k < mlocalsymbolMap.at( i ).vec.size() ) {
      cout << mlocalsymbolMap.at( i ).vec.at( k ).token ;
      k++ ;
    } // while

    cout << endl ;
    i++ ;
  } // while


  cout << endl << "======= GLOBAL =======" << endl ;
  i = 0 ;
  while ( i < msymbolMap.size() ) {
    cout << msymbolMap.at( i ).str << ": " ;
    k = 0 ;
    cout << endl << "memSpace: " << msymbolMap.at( i ).vec.at( 0 ).memSpace << endl ;
    while ( k < msymbolMap.at( i ).vec.size() ) {
      cout << msymbolMap.at( i ).vec.at( k ).token << " " ;
      k++ ;
    } // while

    cout << ", par_name: " ;
    k = 0 ;
    while ( k < msymbolMap.at( i ).par_name.size() ) {
      cout << msymbolMap.at( i ).par_name.at( k ) << " " ;
      k++ ;
    } // while
    
    cout << endl ;
    i++ ;
  } // while
     
  cout << endl << "================" << endl ;
} // Functions::PrintMap()

void Functions::ResetMemNum() {
  memNum = 0 ;
} // Functions::ResetMemNum()

void Functions::SetRoot() {
  mexeNode = gHead ;
} // Functions::SetRoot()

void Functions::ClearLocalMap() {
  mlocalsymbolMap.clear() ;
} // Functions::ClearLocalMap()

bool Functions::FindMap( string str, vector<EXP> &new_vector ) { // �� find local -> global 
  int i = 0 ; 
  while ( i < mlocalsymbolMap.size() ) { // find local map
    if ( mlocalsymbolMap.at( i ).str == str ) {
      new_vector.assign( mlocalsymbolMap.at( i ).vec.begin(), mlocalsymbolMap.at( i ).vec.end() ) ;
      return true ;
    } // if
    
    i++ ;
  } // while
  
  i = 0 ;
  while ( i < msymbolMap.size() ) { // find global map
    if ( msymbolMap.at( i ).str == str ) {
      new_vector.assign( msymbolMap.at( i ).vec.begin(), msymbolMap.at( i ).vec.end() ) ;
      return true ;
    } // if
    
    i++ ;
  } // while
  
  return false ;
    
} // Functions::FindMap() 

bool Functions::ThisIsFunction( string str )
{
  int i = 0 ; 
  while ( i < msymbolMap.size() ) { // find global map
    if ( msymbolMap.at( i ).str == str ) {
      
      if ( msymbolMap.at( i ).funcType != TYPENONE )
      {
        return true ;
      } // if 
      else
      {
        return false ; 
      } // else 
    } // if

    i++ ;
  } // while

  return false ; 
} // Functions::ThisIsFunction() 

bool Functions::FindGlobalMap( string str, vector<EXP> &new_vector, 
                               vector<string> &ss, FuncType &type ) { // find global map
  int i = 0 ;
  while ( i < msymbolMap.size() ) {
    if ( msymbolMap.at( i ).str == str ) {
      new_vector.assign( msymbolMap.at( i ).vec.begin(), msymbolMap.at( i ).vec.end() ) ;
      ss.assign( msymbolMap.at( i ).par_name.begin(), msymbolMap.at( i ).par_name.end() ) ;
      type = msymbolMap.at( i ).funcType ;
      return true ;
    } // if
    
    i++ ;
  } // while
  
  return false ;
    
} // Functions::FindGlobalMap() 

bool Functions::FindLocalMap( string str, vector<EXP> &new_vector ) { // �ufind local map
  int i = 0 ; 
  while ( i < mlocalsymbolMap.size() ) { // find local map
    if ( mlocalsymbolMap.at( i ).str == str ) {
      new_vector.assign( mlocalsymbolMap.at( i ).vec.begin(), mlocalsymbolMap.at( i ).vec.end() ) ;
      return true ;
    } // if
    
    i++ ;
  } // while
  
  return false ;
    
} // Functions::FindLocalMap() 

void Functions::Let() {
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  vector<string> ss ;
  EXP ex ;
  InitExp( ex ) ;
  FuncType type ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    cout << "ERROR (LET format)" << endl ; // pretty print
    mnonListVec.clear() ; 
    TraversalEmpty( gRoot ) ; 
    throw new DefineFormatException( mnonListVec ) ;
    
  } // if
  else { 
    if ( temp->type != EMPTYPTR && temp->type != NIL ) {
      cout << "ERROR (LET format)" << endl ; // pretty print
    } // if
    else {
      if ( temp->type == EMPTYPTR ) {
        EXP* local = temp->listPtr->next ;
        while ( local->type != RIGHT_PAREN ) {
          new_vector.clear() ;
          if ( local->type != EMPTYPTR ) {
            cout << "ERROR (LET format)" << endl ; // pretty print
            mnonListVec.clear() ; 
            TraversalEmpty( gRoot ) ; 
            throw new DefineFormatException( mnonListVec ) ;
          } // if
          else { // mlocalsymbolMap InsertLocalMap(str, vec) 
            EXP* localTP = local->listPtr->next ;
            string str = "\0" ;
            if ( IsSystemPrimitive( localTP->type ) || localTP->type != SYMBOL ) {
              cout << "ERROR (LET format)" << endl ; // pretty print
              mnonListVec.clear() ; 
              TraversalEmpty( gRoot ) ; 
              throw new DefineFormatException( mnonListVec ) ;
            } // if
            else {
              str = localTP->token ;
            } // else

            localTP = localTP->next ;
            
            if ( FindGlobalMap( localTP->token, new_vector, ss, type ) == true ) { // FindGlobalMap
              InsertLocalMap( str, new_vector ) ;
                 
            } // if
            else if ( localTP->type == SYMBOL ) {
              throw new UnboundException( localTP ) ; 
            } // else if 
            else if (  localTP->type == EMPTYPTR ) {
              mexeNode = localTP ;
              Eval() ;
              
              InsertLocalMap( str, localTP->vec ) ;
              
            } // else if 
            else {
              ex.token = localTP->token ;
              ex.type = localTP->type ;
              new_vector.push_back( ex ) ;
              InsertLocalMap( str, new_vector ) ;
            } // else
            
          } // else 
          
          local = local->next ;
          
        } // while 
        
      } // if

      
      temp = temp->next ;
      
      while ( temp->type != RIGHT_PAREN ) {
        new_vector.clear() ;
        
        if ( FindMap( temp->token, new_vector ) == true ) {
          ;
        } // if
        else if ( temp->type == SYMBOL ) {
          throw new UnboundException( temp ) ; 
        } // else if 
        else if (  temp->type == EMPTYPTR ) {
          mexeNode = temp ;
          Eval() ;
          new_vector.assign( temp->vec.begin(), temp->vec.end() ) ;
        
        } // else if
        else {
          ex.token = temp->token ;
          ex.type = temp->type ;
          new_vector.push_back( ex ) ;
           
        } // else
        
        temp = temp->next ;
        
      } // while
      
      emptyptr->vec.assign( new_vector.begin(), new_vector.end() ) ;

    } // else 
    
  } // else
  
} // Functions::Let()

void Functions::Exit()
{
  if ( CheckNumOfArg( 0 ) )
  {
    throw new ExitException( "EXIT" ) ; 
  } // if
  else
  {
    throw new IncorrectNumberException( "exit" ) ; 
  } // else 

} // Functions::Exit() 

void Functions::If()
{
  // mexeNode : if
  // now : �ܼ� 
  // emptyptr : root 
  // ( cond (...) (...) . . . . (else ...) ) 
  EXP* now = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* decide = NULL ;
  EXP* answer = NULL ;

  vector<EXP> new_vector ; 
  decide = now ; 
  if ( decide->type == EMPTYPTR )
  {
    mexeNode = decide ; 
    Eval() ; 
    emptyptr->vec.clear() ; 
    emptyptr->vec = CopyVector( decide->vec ) ; 
    // emptyptr->vec.assign( decide->vec.begin(), decide->vec.end() ) ; 
  } // if 
  else if ( FindMap( decide->token, new_vector ) )
  {
    emptyptr->vec.clear() ; 
    emptyptr->vec = CopyVector( new_vector ) ;
    // emptyptr->vec.assign( new_vector.begin(), new_vector.end() ) ;
  } // else if 
  else if ( decide->type != SYMBOL )
  {
    emptyptr->vec.clear() ;
    emptyptr->vec.push_back( *decide ) ;
  } // else if 
  else
  {
    throw new UnboundException( decide ) ; 
  } // else 
  
  if ( TorF( emptyptr->vec ) == true )
  {
    emptyptr->vec.clear() ; 
    answer = decide->next ; 
    if ( answer->type == EMPTYPTR )
    {
      mexeNode = answer ; 
      Eval() ; 
      emptyptr->vec.clear() ; 
      emptyptr->vec = CopyVector( answer->vec ) ;
      // emptyptr->vec.assign( answer->vec.begin(), answer->vec.end() ) ; 
    } // if 
    else if ( FindMap( answer->token, new_vector ) )
    {
      emptyptr->vec.clear() ; 
      emptyptr->vec = CopyVector( new_vector ) ;
      // emptyptr->vec.assign( new_vector.begin(), new_vector.end() ) ;
    } // else if 
    else if ( answer->type != SYMBOL )
    {
      emptyptr->vec.clear() ;
      emptyptr->vec.push_back( *answer ) ;
    } // else if 
    else
    {
      throw new UnboundException( answer ) ; 
    } // else 

  } // if 
  else
  {
    if ( decide->next->next->type != RIGHT_PAREN )
    {
      emptyptr->vec.clear() ; 
      answer = decide->next->next ; 
      if ( answer != NULL )
      {
        if ( answer->type == EMPTYPTR )
        {
          mexeNode = answer ; 
          Eval() ; 
          emptyptr->vec.clear() ; 
          emptyptr->vec = CopyVector( answer->vec ) ;
          // emptyptr->vec.assign( answer->vec.begin(), answer->vec.end() ) ; 
        } // if 
        else if ( FindMap( answer->token, new_vector ) )
        {
          emptyptr->vec.clear() ; 
          emptyptr->vec = CopyVector( emptyptr->vec ) ;
          // emptyptr->vec.assign( new_vector.begin(), new_vector.end() ) ;
        } // else if 
        else if ( answer->type != SYMBOL )
        {
          emptyptr->vec.clear() ;
          emptyptr->vec.push_back( *answer ) ;
        } // else if 
        else
        {
          throw new UnboundException( answer ) ; 
        } // else
      } // if 
      else
      {
        mnonListVec.clear() ; 
        TraversalEmpty( emptyptr->listPtr ) ; 
        throw new NoReturnException( mnonListVec ) ; 
      } // else 

    } // if 
    else
    {
      mnonListVec.clear() ; 
      TraversalEmpty( emptyptr->listPtr ) ; 
      throw new NoReturnException( mnonListVec ) ; 
    } // else 
  } // else 

} // Functions::If()

void Functions::Cond()
{
  // mexeNode : cond
  // now : �ܼ� 
  // emptyptr : root 
  // ( cond (...) (...) . . . . (else ...) ) 
  EXP* now = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* node = NULL ;
  bool getResult = false ; 
  bool isLast = false ; 

  if ( NOT CheckNumOfArg( 0 ) ) // >= 1 
  {
    EXP * ex = now ;  

    // �ˬd��k 
    while ( ex->type != RIGHT_PAREN )
    {
      // cout << "ex : " << ex->token << endl ;
      if ( ex->type != EMPTYPTR )
      {
        mnonListVec.clear() ; 
        TraversalEmpty( emptyptr->listPtr ) ;
        throw new CondFormatException( mnonListVec ) ;
      } // if
      else
      {
        EXP * sub = ex->listPtr ; 
        // cout << "sub : " << sub->token << endl ; 
        if ( IsNonList( sub ) )
        {
          mnonListVec.clear() ; 
          TraversalEmpty( emptyptr->listPtr ) ;
          throw new CondFormatException( mnonListVec ) ;
        } // if
        else if ( CheckNumOfArgFromParen( sub, 1 ) )
        {
          mnonListVec.clear() ; 
          TraversalEmpty( emptyptr->listPtr ) ;
          throw new CondFormatException( mnonListVec ) ;
        } // else if 

      } // else 

      ex = ex->next ; 
    } // while 


    // ����
    while ( NOT getResult && now->type != RIGHT_PAREN )
    { 
      if ( now->next->type == RIGHT_PAREN )
      {
        isLast = true ; 
      } // if 

      // cout << "�o�̬O�Ĥ@�ӰѼ� : " << now->token << endl ;
      if ( NOT isLast )
      {

        node = now->listPtr->next ; // first augument
        bool trueOdFalse = true ; 
        int cnt = 0 ; 
        bool jump = false ; 
        while ( trueOdFalse == true && node->type != RIGHT_PAREN )
        {
          // cout << endl << endl << "RUN" << endl ;
          if ( node->type == EMPTYPTR )
          {
            mexeNode = node ; 
            Eval() ; 
            if ( NOT getResult )
            { 
              emptyptr->vec.clear() ; 
              emptyptr->vec = CopyVector( node->vec ) ;
              // emptyptr->vec.assign( node->vec.begin(), node->vec.end() ) ;
              // cout << "Pretty1 : " << PrettyString( emptyptr->vec ) ; 
            } // if 
          } // if 
          else
          {

            mexeNode = node ; 
            // cout << "mexeNode : " << mexeNode->token << endl ;
            vector<EXP> new_vector ; 

            if ( FindMap( mexeNode->token, new_vector ) ) 
            {
              if ( NOT getResult )
              { 
                emptyptr->vec.clear() ; 
                emptyptr->vec = CopyVector( new_vector ) ;
                // emptyptr->vec.assign( new_vector.begin(), new_vector.end() ) ;
                // cout << "Pretty2 : " << PrettyString( emptyptr->vec ) ;
              } // if
            } // if 
            else if ( mexeNode->type == SYMBOL )
            {
              throw new UnboundException( mexeNode ) ; 
            } // else if 
            else 
            {
              if ( NOT getResult )
              { 
                emptyptr->vec.clear() ; 
                emptyptr->vec.push_back( *mexeNode ) ;
                // cout << "Pretty3 : " << PrettyString( emptyptr->vec ) ;
              } // if
            } // else 

          } // else 

          if ( cnt == 0 )
          {

            trueOdFalse = TorF( emptyptr->vec ) ; 
            // cout << " Check : " << trueOdFalse << endl ;
            if ( trueOdFalse == false )
            {
              jump = true ; 
            } // if 
          } // if 

          cnt ++ ; 
          node = node->next ;
          // cout << "next : " << node->token << endl ; 
        } // while 

        if ( trueOdFalse == true )
        {
          // cout << "Get result : " << PrettyString( emptyptr->vec ) << endl ;
          getResult = true ; 
        } // if 

      } // if 
      else
      {
        node = now->listPtr->next ; // first augument 
        bool trueOdFalse = true ; 
        int cnt = 0 ; 
        int e = 0 ;
        while ( node->type != RIGHT_PAREN )
        {
          // cout << endl << endl << "RUN" << endl ;
          if ( node->type == EMPTYPTR )
          {
            mexeNode = node ; 
            Eval() ; 
            if ( NOT getResult )
            { 
              emptyptr->vec.clear() ; 
              emptyptr->vec = CopyVector( node->vec ) ;
              // emptyptr->vec.assign( node->vec.begin(), node->vec.end() ) ;
              // cout << "Pretty1 : " << PrettyString( emptyptr->vec ) ; 
            } // if 
          } // if 
          else
          {

            mexeNode = node ; 
            // cout << "mexeNode : " << mexeNode->token << endl ;
            vector<EXP> new_vector ; 
            if ( mexeNode->token == "else" && e == 0 )
            {
              e++ ; 
              if ( NOT getResult )
              { 
                emptyptr->vec.clear() ; 
                emptyptr->vec.push_back( *mexeNode ) ;
                // cout << "else : " << PrettyString( emptyptr->vec ) ;
              } // if
            } // if
            else if ( FindMap( mexeNode->token, new_vector ) ) 
            {
              if ( NOT getResult )
              { 
                emptyptr->vec.clear() ; 
                emptyptr->vec = CopyVector( new_vector ) ;
                // emptyptr->vec.assign( new_vector.begin(), new_vector.end() ) ;
                // cout << "Pretty2 : " << PrettyString( emptyptr->vec ) ;
              } // if
            } // else if 
            else if ( mexeNode->type == SYMBOL )
            {
              throw new UnboundException( mexeNode ) ; 
            } // else if 
            else 
            {
              if ( NOT getResult )
              { 
                emptyptr->vec.clear() ; 
                emptyptr->vec.push_back( *mexeNode ) ;
                // cout << "Pretty3 : " << PrettyString( emptyptr->vec ) ;
              } // if
            } // else 

          } // else 

          if ( cnt == 0 )
          {

            trueOdFalse = TorF( emptyptr->vec ) ; 
            // cout << " Check : " << trueOdFalse << endl ; 

            if ( trueOdFalse == false )
            {
              mnonListVec.clear() ; 
              TraversalEmpty( emptyptr->listPtr ) ;
              throw new NoReturnException( mnonListVec ) ; 
            } // if 
          } // if 

          cnt ++ ; 
          node = node->next ;
          // cout << "next : " << node->token << endl ; 
        } // while 

        if ( trueOdFalse == true )
        {
          // cout << "Get result : " << PrettyString( emptyptr->vec ) << endl ;
          getResult = true ; 
        } // if 

      } // else 

      now = now->next ; 
    } // while 

  } // if 
  else
  {
    mnonListVec.clear() ; 
    TraversalEmpty( emptyptr->listPtr ) ;
    throw new CondFormatException( mnonListVec ) ;
  } // else 

} // Functions::Cond() 

void Functions::Begin()
{
  // mexeNode : begin 
  // now : �ܼ� 
  // emptyptr : root 
  EXP* now = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;

  if ( NOT CheckNumOfArg( 0 ) ) // >= 1 
  {
    while ( now->type != RIGHT_PAREN )
    {
      if ( now->type == EMPTYPTR )
      {
        mexeNode = now ; 
        Eval() ; 
        emptyptr->vec.clear() ; 
        emptyptr->vec = CopyVector( now->vec ) ;
        // emptyptr->vec.assign( now->vec.begin(), now->vec.end() ) ; 
      } // if 
      else
      {
        mexeNode = now ; 
        Eval() ;
        emptyptr->vec.clear() ; 
        emptyptr->vec = CopyVector( mresult ) ;
        // emptyptr->vec.assign( mresult.begin(), mresult.end() ) ;
      } // else 
      
      now = now->next ; 
    } // while 

  } // if 
  else
  {
    throw new IncorrectNumberException( "begin" ) ;
  } // else 

} // Functions::Begin()

void Functions::Pair_qmark()
{
  // �u�n���Oatom �N�O true 
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP ex ;
  InitExp( ex ) ; 
  if ( CheckNumOfArg( 1 ) )
  {
    Qmark( "atom?" ) ; 
    if ( emptyptr->vec.at( 0 ).type == NIL )
    { emptyptr->vec.clear() ; 
      ex.token = "#t" ;
      ex.type = T ;
      emptyptr->vec.push_back( ex ) ;
    } // if 
    else
    {
      emptyptr->vec.clear() ;
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // else

  } // if 
  else
  {
    throw new IncorrectNumberException( "pair?" ) ; 
  } // else 

} // Functions::Pair_qmark()

void Functions::List_qmark()
{ 

  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  vector<EXP> new_vector ; 
  EXP ex ;
  InitExp( ex ) ;
  if ( CheckNumOfArg( 1 ) )
  {
    Pair_qmark() ;  
    // cout << "Pretty : " << PrettyString( mnonListVec ) ;
    if ( emptyptr->vec.at( 0 ).token == "#t" )
    {
      if ( IsList( mnonListVec ) ) 
      {
        emptyptr->vec.clear() ;
        ex.token = "#t" ;
        ex.type = T ;
        emptyptr->vec.push_back( ex ) ;
      } // if 
      else
      {
        emptyptr->vec.clear() ;
        ex.token = "nil" ;
        ex.type = NIL ;
        emptyptr->vec.push_back( ex ) ;
      } // else
    } // if 
    else
    {
      emptyptr->vec.clear() ;
      ex.token = "nil" ;
      ex.type = NIL ;
      emptyptr->vec.push_back( ex ) ;
    } // else 
    

  } // if 
  else
  {
    throw new IncorrectNumberException( "list?" ) ; 
  } // else 

} // Functions::List_qmark()

void Functions::Clean_Environment() {
  if ( CheckNumOfArg( 0 ) ) {
    msymbolMap.clear() ;
    EXP temp ; 
    InitExp( temp ) ;
    temp.token = "environment cleaned" ; 
    temp.type = SYMBOL ; 
    mresult.clear() ; 
    mresult.push_back( temp ) ; 
  } // if
  else {
    throw new IncorrectNumberException( "clean-environment" ) ; 
  } // else
  
} // Functions::Clean_Environment()

void Functions::Or() { // arg >= 2 
  // �^�ǲĤ@�Ӥ��Onil��arg �p�G���Onil �^�ǳ̫�@��arg 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  vector<EXP> lastStmt ;
  bool bbreak = false ;
  EXP ex ;
  InitExp( ex ) ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw new IncorrectNumberException( "or" ) ;
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
        throw new UnboundException( temp ) ; 
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
  // �^�ǲĤ@�ӥX�{��nil �p�G�S�� �^�ǳ̫�@��arg 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  vector<EXP> lastStmt ;
  EXP ex ;
  InitExp( ex ) ;
  bool isNIL = false ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw new IncorrectNumberException( "and" ) ;
  } // if
  else { 
    while ( isNIL == false && temp->type != RIGHT_PAREN ) { 
      
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
        throw new UnboundException( temp ) ; 
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
  InitExp( ex ) ;
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
      throw new UnboundException( temp ) ; 
    } // else if 
    else if ( temp->type == EMPTYPTR ) {
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
  // �u��������F��Oatom(����Ostring)�� �άO���object���ۦP��mem space �Otrue 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ; 
  EXP* temp = mexeNode->next ;         
  EXP ex ;
  InitExp( ex ) ;
  vector<EXP> new_vector ;
  int argNum = 1 ;    
  EXP firstArg ;
  InitExp( firstArg ) ;
  EXP secondArg ;
  InitExp( secondArg ) ;
  bool noError = true ;
  bool sameMap = false ;
  bool isNil = false ;
  string symbolDefined = "" ;
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

        } // else if

      } // if
      else if ( temp->type == SYMBOL ) {                            
        throw new UnboundException( temp ) ; 
        noError = false ;
      } // else if
      else if ( temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        if ( argNum == 1 ) 
          firstArg.vec.assign( temp->vec.begin(), temp->vec.end() ) ;
        else if ( argNum == 2 ) 
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
    InitExp( ex ) ;
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
    throw new IncorrectNumberException( "eqv?" ) ;
  } // else 
  
} // Functions::Eqv_qmark()

void Functions::Equal_qmark() { // arg == 2 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ; 
  EXP* temp = mexeNode->next ;         
  EXP ex ;
  InitExp( ex ) ;
  vector<EXP> new_vector ;
  int argNum = 1 ;    
  EXP firstArg ;
  InitExp( firstArg ) ;
  EXP secondArg ;
  InitExp( secondArg ) ;
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
        throw new UnboundException( temp ) ; 
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
    InitExp( ex ) ;
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
    throw new IncorrectNumberException( "=?" ) ;
  } // else 
  
} // Functions::Equal_qmark()

void Functions::Not() { // arg == 1
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;          
  EXP ex ;
  InitExp( ex ) ;
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
      throw new UnboundException( temp ) ; 
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
    throw new IncorrectNumberException( "not" ) ;
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

void Functions::String_append() { 
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> new_vector ;
  string str = "\"" ;
  bool isTrue = true ;
  EXP ex ;
  InitExp( ex ) ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw new IncorrectNumberException( "string-append" ) ;
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
          throw new IncorrectArgumentException( "string-append", new_vector ); 
        } // else

      } // else if 
      else if ( temp->type == SYMBOL ) {
        isTrue = false ;
        throw new UnboundException( temp ) ; 
      } // else if 
      else if (  temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        
        if ( temp->vec.size() == 1 && temp->vec.at( 0 ).type == STRING ) {
          str = str + FixDoubleQuotes( temp->vec.at( 0 ).token ) ;
        } // if
        else {
          isTrue = false ;
          throw new IncorrectArgumentException( "string-append", temp->vec );
        } // else
        
      } // else if
      else {
        isTrue = false ;
        throw new IncorrectArgumentException( "string-append", *temp );
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
  string previous_string = "" ;
  vector<EXP> new_vector ;
  EXP ex ;
  InitExp( ex ) ;
  bool isTrue = true ;
  bool isFirstArg = true ;
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw new IncorrectNumberException( whichOperator ) ;
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
          throw new IncorrectArgumentException( whichOperator, new_vector ) ; 
        } // else

      } // else if 
      else if ( temp->type == SYMBOL ) {
        isTrue = false ;
        throw new UnboundException( temp ) ;  
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
          throw new IncorrectArgumentException( whichOperator, temp->vec ) ; 
        } // else
        
      } // else if
      else {
        isTrue = false ;
        throw new IncorrectArgumentException( whichOperator, *temp ) ; 
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
  InitExp( ex ) ;
  bool isTrue = true ;
  bool isFirstArg = true ;
  
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw new IncorrectNumberException( whichOperator ) ;
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
          throw new IncorrectArgumentException( whichOperator, new_vector ) ; 
        } // else

      } // else if 
      else if ( temp->type == SYMBOL ) {
        isTrue = false ;
        throw new UnboundException( temp ) ; 
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
          throw new IncorrectArgumentException( whichOperator, temp->vec ) ; 
        } // else
        
      } // else if
      else {
        isTrue = false ;
        throw new IncorrectArgumentException( whichOperator, *temp ) ; 
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
//  cout << "================" << endl ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  float sum = 0 ;
  vector<EXP> new_vector ;
  bool noError = true ;
  bool isFirstArg = true ;
  bool hasFloat = false ;
  
  if ( CheckNumOfArg( 1 ) || CheckNumOfArg( 0 ) ) {
    throw new IncorrectNumberException( whichOperator ) ;
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
            throw new DivByZeroException() ; 
          } // if
          else {
            sum = sum / atof( temp->token.c_str() ) ;
          } // else
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
              throw new DivByZeroException() ; 
            } // if
            else {
              sum = sum / atof( new_vector.at( 0 ).token.c_str() ) ;
            } // else
          } // else if
        
        } // if
        else {
          noError = false ;
          // cout << "aaaa" << endl ;
          throw new IncorrectArgumentException( whichOperator, new_vector ) ; 
        } // else
      } // else if
      else if ( temp->type == SYMBOL ) {
        noError = false ;
        throw new UnboundException( temp ) ; 
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
              throw new DivByZeroException() ;
            } // if
            else {
              sum = sum / atof( temp->vec.at( 0 ).token.c_str() ) ;
            } // else
          } // else if
          
        } // if
        else { 
          noError = false ;
          // cout << "ADD emptyptr: " ;
          // PreOrderTraversal( emptyptr ) ;
          // cout << endl << "ADD gRoot: " ;
          // PreOrderTraversal( gRoot ) ;
          throw new IncorrectArgumentException( whichOperator, temp->vec ) ; 
        } // else
        
      } // else if
      else {
        throw new IncorrectArgumentException( whichOperator, *temp ) ; 
        noError = false ;
      } // else 
      
      temp = temp->next ;
    } // while
    
    if ( noError == true ) {
      EXP ex ;
      InitExp( ex ) ;
      if ( hasFloat == true ) {
        
        ex.token = Rounding( FloatToString( sum ) ) ;
        ex.type = FLOAT ;      
      } // if
      else { // INT
        ex.token = IntToString( sum ) ;
        ex.type = INT ;
      } // else
        
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
  InitExp( ex ) ;
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
      mnonListVec.clear() ; 
      mnonListVec.assign( new_vector.begin(), new_vector.end() ) ; 
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
      throw new UnboundException( temp ) ; 
    } // else if
    else if ( temp->type == EMPTYPTR ) {
      mexeNode = temp ;
      Eval() ;
      
      mnonListVec.clear() ; 
      mnonListVec.assign( temp->vec.begin(), temp->vec.end() ) ; 

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
    throw new IncorrectNumberException( whichQmark ) ;
  } // else 
  
} // Functions::Qmark()

void Functions::Cdr() {

  vector<EXP> new_vector ;
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP ex ;
  InitExp( ex ) ;
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
        throw new IncorrectArgumentException( "cdr", new_vector ) ;
      } // else

    } // if 
    else if ( temp->type == SYMBOL ) {
      throw new UnboundException( temp ) ; 
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
    else if ( temp->type == EMPTYPTR )
    {
      mexeNode = temp ; 
      Eval() ; 
      if ( temp->vec.size() <= 1 )
      {
        throw new IncorrectArgumentException( "cdr", temp->vec ) ;
      } // if 
      
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
    } // else if 
    else {
      throw new IncorrectArgumentException( "cdr", *temp ) ; 
    } // else
  } // if CheckNumOfArg( 1 )

  else {
    throw new IncorrectNumberException( "cdr" ) ;
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
  InitExp( ex ) ;
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
        throw new IncorrectArgumentException( "car", new_vector ) ; 
      } // else

    } // if FindMap
    else if ( temp->type == SYMBOL ) {
      throw new UnboundException( temp ) ; 
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
        emptyptr->vec.push_back( temp->vec.at( 1 ) ) ; 
      } // else

    } // if temp->type == EMPTYPTR
    else if ( temp->type == EMPTYPTR )
    {
      mexeNode = temp ;
      Eval() ;
      if ( temp->vec.size() <= 1 )
      {
        throw new IncorrectArgumentException( "car", temp->vec ) ; 
      } // if 
      else if ( temp->vec.size() >= 2 && temp->vec.at( 1 ).type == LEFT_PAREN ) {
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
      } // else if
      else {
        emptyptr->vec.push_back( temp->vec.at( 1 ) ) ; 
      } // else

    } // else if 
    else {
      throw new IncorrectArgumentException( "car", *temp ) ; 
    } // else
  } // if CheckNumOfArg( 1 )
  else {
    throw new IncorrectNumberException( "car" ) ; 
  } // else 

} // Functions::Car() 

void Functions::Quote() {
  EXP* temp = mexeNode->next ;
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP ex ;
  InitExp( ex ) ;
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
        
        while ( temp->type != LEFT_PAREN ) {  
          temp = temp->pre_next ; 
        } // while
        
        temp = temp->pre_listPtr ;
        if ( temp == emptyptr ) {
          bbreak = true ;
        } // if
        else {
          temp = temp->next ;
        } // else
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
    throw new IncorrectNumberException( "\'" ) ;
  } // else

} // Functions::Quote()

void Functions::Cons() { 
  string str = "\0" ;
  EXP ex ;
  InitExp( ex ) ;
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
        for ( int i = 0; i < new_vector.size() ; i++ ) {
          emptyptr->vec.push_back( new_vector.at( i ) ) ;
        } // for
        
        new_vector.clear() ;
        
      } // else if
      else if ( temp->type == SYMBOL ) {
        throw new UnboundException( temp ) ; 
      } // else if 
      else if ( temp->type == EMPTYPTR ) {
        mexeNode = temp ;
        Eval() ;
        for ( int i = 0; i < temp->vec.size() ; i++ ) {
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
 
  } // if
  else {
    throw new IncorrectNumberException( "cons" ) ;
  } // else

} // Functions::Cons()

void DeleteTree( EXP* focusNode )
{
  if ( focusNode == NULL ) {
    return ;
  } // if

  DeleteTree( focusNode->next ) ;
  DeleteTree( focusNode->listPtr ) ;
  delete focusNode;
  focusNode = NULL ;
} // DeleteTree() 

void Functions::CallFunction() {
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  EXP* temp = mexeNode->next ;
  vector<EXP> s_exp ;
  vector<string> parameter ;
  FuncType type ; 
  FindGlobalMap( mexeNode->token, s_exp, parameter, type ) ;
  EXP ex ;
  InitExp( ex ) ;

  if ( CheckNumOfArg( parameter.size() ) ) { 
    int i = 0 ;
    vector<EXP> new_vector ;

    while ( i < parameter.size() ) {  
      new_vector.clear() ;
      if ( temp->type == EMPTYPTR ) {
        
        GetPreOrderTraversalWithNoEmpty( temp->listPtr, new_vector ) ;
        // cout << endl << endl << "new_vector : " << PrettyString(new_vector)<< endl ;
        InsertLocalMap( parameter.at( i ), new_vector ) ;
      } // if
      else {
        ex.token = temp->token ; 
        ex.type = temp->type ;
        new_vector.push_back( ex ) ;
        InsertLocalMap( parameter.at( i ), new_vector ) ;
      } // else

      temp = temp->next ;
      i++ ;
      
    } // while
    
    new_vector.clear() ;
    vector<EXP> new_s_exp ; 
    // cout << "s_exp : " << PrettyString( s_exp ) << endl ; 
    for ( int i = 0 ; i < s_exp.size() ; i++ ) { 
      if ( s_exp.at( i ).type == LET || s_exp.at( i ).type == LAMBDA ) {
        i-- ;
        int count = 0 ;
        bool bbreak = false ;
        
        int cnt = 0 ;
        // ( define ( a b ) ( let (( a b )(b b ) ( c b )) ( + a b c ) ) )
        while ( bbreak == false ) {
          
          if ( s_exp.at( i ).type == LEFT_PAREN ) {
            count++;
          } // if
          else if ( s_exp.at( i ).type == RIGHT_PAREN ) {
            count--;
            if ( count == 0 ) {
              bbreak = true ;
            } // if

          } // else if
          
            

          if ( bbreak == false ) {
            
            bool isPush = false ;
            if ( count == 3 )
            {  
              // cout << "new_s_exp : " << PrettyString( new_s_exp ) << endl ;
              cnt ++ ;
              if ( cnt == 2 ) 
              {
                // cout << "Count == 3 && cnt == 2 : " << s_exp.at( i ).token << endl ; 
                
                if ( FindLocalMap( s_exp.at( i + 1 ).token, new_vector ) )
                {
                  isPush = true ; 
                  
                  for ( int i = 0; i < new_vector.size() ; i++ ) 
                  {
                    new_s_exp.push_back( new_vector.at( i ) ) ;
                  } // for
                  
                  
                } // if  
              } // if  
              
            } // if
            else if ( count == 2 )
            {
              cnt = 0 ; 
            } // else if 
            
            i++ ;
            if ( isPush == false )
            {
              new_s_exp.push_back( s_exp.at( i ) ) ;  
            } // if 
          } // if

        } // while
     
      } // if
      else if ( s_exp.at( i ).type == SYMBOL && FindLocalMap( s_exp.at( i ).token, new_vector ) ) {
        for ( int i = 0; i < new_vector.size() ; i++ ) {
          new_s_exp.push_back( new_vector.at( i ) ) ;
        } // for
        
      } // else if
      else {
        new_s_exp.push_back( s_exp.at( i ) ) ;
      } // else
      
    } // for
    
    ClearLocalMap() ;
    i = 0 ;
    temp = NULL ;
    EXP* root = DynamicBuildTree( new_s_exp, i ) ;
    
    emptyptr->listPtr->pre_listPtr = NULL ;
    emptyptr->listPtr = root->listPtr ; // double link list
    root->listPtr = NULL ;
    emptyptr->listPtr->pre_listPtr = emptyptr ;
    mexeNode = emptyptr->listPtr->next ; // first argument 
    
    // cout << "Extention Function : " ; 
    // PreOrderTraversal( gRoot ) ; 
    // cout << endl ; 
  } // if
  else {
    throw new IncorrectNumberException( mexeNode->token ) ;
  } // else
    

} // Functions::CallFunction()

void Functions :: CallLambdaFunction() 
{ 
  
  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ;
  
  MAP value ; 
  
  SearchLambdaMap( mexeNode->token, value ) ;
  
  vector<string> parameter ; 
  
  parameter.assign( value.par_name.begin(), value.par_name.end() ) ;
  
  int parameterSize = value.par_name.size() ; 
  
  EXP* temp = mexeNode->next ;
  
  vector<EXP> s_exp ; 
  
  s_exp.assign( value.vec.begin(), value.vec.end() ) ; 
  
  // cout << "lambda_exp : " << PrettyString( s_exp ) << endl ;
  
  vector<EXP> new_s_exp ;  
  if ( CheckNumOfArg( parameterSize ) )
  {
    int j = 0 ;
    vector<EXP> new_vector ;
    while ( j < parameterSize ) {  
      new_vector.clear() ;
      if ( temp->type == EMPTYPTR ) {
        GetPreOrderTraversalWithNoEmpty( temp->listPtr, new_vector ) ;
        InsertLocalMap( parameter.at( j ), new_vector ) ;
      } // if
      else {
        EXP ex ; 
        ex.token = temp->token ; 
        ex.type = temp->type ;
        new_vector.push_back( ex ) ;
        InsertLocalMap( parameter.at( j ), new_vector ) ;
      } // else

      temp = temp->next ;
      j++ ;
    } // while

    new_vector.clear() ;
    vector<EXP> new_s_exp ;
    bool isLamdaOrLet = false ; 
    int parNum = 0 ; 
    for ( int i = 0 ; i < s_exp.size() ; i++ ) 
    {
      if ( isLamdaOrLet == false && ( s_exp.at( i ).type == LAMBDA || s_exp.at( i ).type == LET ) ) 
      {
        isLamdaOrLet = true ; 
        parNum = 1 ; 
      } // if 

      if ( isLamdaOrLet == false )
      {
        if ( s_exp.at( i ).type == SYMBOL && FindLocalMap( s_exp.at( i ).token, new_vector ) ) {
          for ( int i = 0; i < new_vector.size() ; i++ ) {
            new_s_exp.push_back( new_vector.at( i ) ) ;
          } // for
        } // if
        else {
          new_s_exp.push_back( s_exp.at( i ) ) ;
        } // else
      } // if 
      else
      {
        new_s_exp.push_back( s_exp.at( i ) ) ;
      } // else

      if ( isLamdaOrLet == true )
      {
        if ( s_exp.at( i ).type == LEFT_PAREN )
        {
          parNum ++ ; 
        } // if 
        else if ( s_exp.at( i ).type == RIGHT_PAREN )
        {
          parNum -- ; 
        } // else if
      } // if 
      
      if ( isLamdaOrLet == true && parNum == 0 )
      {
        isLamdaOrLet = false ; 
      } // if 
    } // for

    // cout << "Extention : " << PrettyString( new_s_exp ) << endl ;

    ClearLocalMap() ;
    int i = 0 ;
    EXP* root = DynamicBuildTree( new_s_exp, i ) ;
    // PreOrderTraversal( root ) ; 
    emptyptr->listPtr->pre_listPtr = NULL ;
    emptyptr->listPtr = root->listPtr ; // double link list
    root->listPtr = NULL ;
    emptyptr->listPtr->pre_listPtr = emptyptr ;
    mexeNode = emptyptr->listPtr->next ; // first argument 
  } // if 
  else
  {
    throw new IncorrectNumberException( mexeNode->token ) ;
  } // else

} // Functions::CallLambdaFunction()

void Functions::CheckLambdaFormat( EXP* now )
{
  // format : ( lambda ( zero-or-more-symbols ) one-or-more-S-expressions )
  // now => "(" 

  int parNum = 1 ; 
  now = now->next ; // now = "lambda"

  if ( now->next->type != EMPTYPTR )
  {
    mnonListVec.clear() ; 
    TraversalEmpty( gRoot ) ; 
    throw new LambdaFormatException( mnonListVec ) ; 
  } // if 
  else if (  now->next->type == EMPTYPTR )
  { 
    now = now -> next ; // now = empty 
    if ( now->next->type == RIGHT_PAREN )
    {
      mnonListVec.clear() ; 
      TraversalEmpty( gRoot ) ; 
      throw new LambdaFormatException( mnonListVec ) ;
    } // if 
    else
    { 
      EXP* temp = now->listPtr ; // now = "("
      temp = temp->next ; // temp = first symbol
      do // 檢查 parmater 是不是 symbol 
      {
        if ( IsSystemPrimitive( temp->type ) || temp->type != SYMBOL ) {
          mnonListVec.clear() ; 
          TraversalEmpty( gRoot ) ; 
          throw new LambdaFormatException( mnonListVec ) ; 
        } // if
        
        temp = temp->next ; 
      } while ( temp->type != RIGHT_PAREN ) ;

    } // else 

  } // else if

  now = now -> listPtr ; // now = "(" 
} // Functions::CheckLambdaFormat()

void Functions::Lambda()
{
  // lambda format
  // mexeNode = "lambda" 
  mLambdaSymbolMap.clear() ; 

  EXP* emptyptr = mexeNode->pre_next->pre_listPtr ; 
  EXP tt ; 
  tt.type = LAMBDA_FUNC ; 
  tt.token = "lambda" ;
  emptyptr->vec.push_back( tt ) ; 
  EXP* now = mexeNode->pre_next ; // now = "(" 

  CheckLambdaFormat( now ) ; 

  // now = ( lambda "(" 
  int numOfParmeter = 0 ; 
  vector<string> parameter ; 

  now = now->next->next  ; 

  EXP* par = now ->listPtr ; // par = "("
  // cout << endl << "parameter : [ " ;
  while ( par->type != RIGHT_PAREN ) {
    
     

    if ( par->type != LEFT_PAREN )
    {
      // cout << par->token << " " ; 
      parameter.push_back( par->token ) ;
    } // if 
      
    par = par->next ;

  } // while
  
  // cout << "]" << endl ; 

  now = now -> next ; // now = first S-expression

  int parNum = 0 ; 
  vector<EXP> s_exp ;

  GetPreOrderTraversalWithNoEmpty( now, s_exp ) ;
  s_exp.pop_back() ; 
  // cout << PrettyString( s_exp ) << endl ; 

  MAP lambda_vec ; 
  lambda_vec.str = "lambda" ;
  lambda_vec.funcType = TYPELAMBDA ;
  lambda_vec.par_name.assign( parameter.begin(), parameter.end() ) ; 
  lambda_vec.vec.assign( s_exp.begin(), s_exp.end() ) ; 
  InsertLambdaMap( lambda_vec ) ; 

} // Functions::Lambda()

void Functions::Define() { 
  memNum++ ;
  string str = "\0" ;
  vector<EXP> vs ;
  vector<EXP> new_vector ; 
  vector<string> pp ;
  EXP ex ;
  InitExp( ex ) ;
  EXP* temp = mexeNode->next ;
  FuncType type ;
  
  if ( CheckNumOfArg( 0 ) || CheckNumOfArg( 1 ) ) { 
    mnonListVec.clear() ; 
    TraversalEmpty( gRoot ) ;
    throw new DefineFormatException( mnonListVec ) ;
  } // if
  else {
    
    if ( temp->type == SYMBOL ) { // project1 + 2
      str = temp->token ;
      temp = temp->next ;
      if ( temp->type == EMPTYPTR ) { 
        mexeNode = temp ;
        Eval() ;

        temp->vec.at( 0 ).memSpace = memNum ;
        InsertMap( str, temp->vec ) ;
        
      } // if
      else if ( FindGlobalMap( temp->token, new_vector, pp, type ) ) { 
        new_vector.at( 0 ).memSpace = FindMemNum( temp->token ) ; 
        if ( type == TYPEDEFINE ) {
          InsertMapAndPar( str, new_vector, pp, TYPEDEFINE ) ; 
        } // if
        else {
          InsertMap( str, new_vector ) ;
        } // else
        
        
      } // else if
      else if ( temp->type == SYMBOL ) {
        throw new UnboundException( temp ) ; 
      } // else if
      else {
        ex.token = temp->token ;
        ex.type = temp->type ;
        ex.memSpace = memNum ;
        vs.push_back( ex ) ; 
        InsertMap( str, vs ) ;
        
      } // else 
      
    } // if
    else if ( temp->type == EMPTYPTR ) { // new_vector.at( 0 ).memSpace = FindMemNum( temp->token ) ; 
      EXP* funcName = temp->listPtr->next ;
      if ( IsSystemPrimitive( funcName->type ) || funcName->type != SYMBOL ) {
        mnonListVec.clear() ; 
        TraversalEmpty( gRoot ) ; 
        throw new DefineFormatException( mnonListVec ) ; 
      } // if
      else {
        str = funcName->token ;
        funcName = funcName->next ;
        
        while ( funcName->type != RIGHT_PAREN ) {
          if ( IsSystemPrimitive( funcName->type ) || funcName->type != SYMBOL ) {
            mnonListVec.clear() ; 
            TraversalEmpty( gRoot ) ; 
            throw new DefineFormatException( mnonListVec ) ; 
          } // if
          else {
            pp.push_back( funcName->token ) ;
          } // else
          
          funcName = funcName->next ;
        } // while
        
        temp = temp->next ;
        
        while ( temp->type != RIGHT_PAREN ) {
          new_vector.clear() ;
          if ( temp->type == EMPTYPTR ) {
            EXP* expression = temp ;
            
            while ( expression != temp->next ) {
              
              if ( expression->type == EMPTYPTR ) {
                expression = expression->listPtr ;
              } // if
              
              ex.token = expression->token ;
              ex.type = expression->type ;
              new_vector.push_back( ex ) ;
              
              if ( expression->type == RIGHT_PAREN ) {
              
                while ( expression->type != LEFT_PAREN ) { 
                  expression = expression->pre_next ; 
                } // while
                
                expression = expression->pre_listPtr->next ;
                
              } // if
              else {
                expression = expression->next ;
              } // else

            } // while
            
          } // if
          else {
            ex.token = temp->token ;
            ex.type = temp->type ;
            new_vector.push_back( ex ) ;
          } // else
          
          temp = temp->next ;
          
        } // while
        
        new_vector.at( 0 ).memSpace = memNum ; // 
        InsertFuncInGlobalMap( str, pp, new_vector, TYPEDEFINE ) ;
    
      } // else
      
    } // else if
    else {
      mnonListVec.clear() ; 
      TraversalEmpty( gRoot ) ; 
      throw new DefineFormatException( mnonListVec ) ; 
    } // else
      
  } // else

  mresult.clear() ; 
  EXP tt ;
  InitExp( tt ) ;
  tt.token = str + " defined" ; 
  tt.type = SYMBOL ;
  mresult.push_back( tt ) ;

} // Functions::Define()

bool Functions::IsSystemPrimitive( Type type ) {
  if ( type == EXIT || type == CONS || type == LIST || type == QUOTE || type == DEFINE || type == CAR 
       || type == CDR || type == ATOM_QMARK || type == PAIR_QMARK || type == LIST_QMARK 
       || type == NULL_QMARK || type == INTEGER_QMARK || type == REAL_QMARK 
       || type == NUMBER_QMARK || type == STRING_QMARK || type == BOOLEAN_QMARK 
       || type == SYMBOL_QMARK || type == ADD || type == SUB || type == MULT || type == DIV 
       || type == NOTT || type == AND || type == OR || type == BIGGERTHAN 
       || type == BIGGEREQUAL || type == LESSTHAN || type == LESSEQUAL || type == EQUAL 
       || type == STRING_APPEND || type == STRING_BIGGER || type == STRING_LESS 
       || type == STRING_EQUAL || type == EQV_QMARK || type == EQUAL_QMARK || type == BEGIN 
       || type == IF || type == COND || type == CLEAN_ENVIRONMENT 
       || type == LET || type == LAMBDA || type == LAMBDA_FUNC ) {
    return true ;
  } // if 
  else 
    return false ;
    
} // Functions::IsSystemPrimitive() 

void Functions::Eval() {
  bool callFunction = false ; 
  // cout << endl << "eval mexenode : " << mexeNode->token << endl ;
  // cout << "full Exp : " ; 
  // PreOrderTraversal( gRoot ) ; 
  // cout << endl ; 

  bool hasError = false ;
  vector<EXP> new_vector ; 
  EXP* temp = mexeNode ;
  EXP* firstArgument = NULL ; 
  mlevel ++ ; 
                                                                     
  if ( IsATOM( temp ) && temp->type != SYMBOL  )  {
    hasError = true ; 
    mexeNode = temp ;
    mresult.clear() ; 
    mresult.push_back( *temp ) ; 
  } // if
  else if ( temp->type == SYMBOL ) {
    hasError = true ; 
    vector<string> ss ; 
    FuncType tt ; 
    if ( FindGlobalMap( temp->token, new_vector, ss, tt ) == false ) // unbound symbol 
    { 
      throw new UnboundException( temp ) ; 
    } // if  
    else
    {
      // cout << "HERE : " << PrettyString( new_vector ) << endl ; 
      if ( tt == TYPENONE )
      {
        mresult.clear() ; 
        mresult.assign( new_vector.begin(), new_vector.end() ) ;
      } // if 
      else if ( tt == TYPEDEFINE )
      {
        mresult.clear() ; 
        EXP t ; 
        string binding ; 
        int memspace = FindMemNum( temp->token ) ; 
        for ( int j = 0 ; j < msymbolMap.size() ; j ++ )
        {
          if ( memspace == msymbolMap.at( j ).vec.at( 0 ).memSpace )
          {
            binding = msymbolMap.at( j ).str ; 
            j = msymbolMap.size() ;
          } // if 
        } // for
         
        t.token = "#<procedure " + binding + ">" ;
        t.type = SYMBOL ; 
        mresult.push_back( t ) ;
      } // else if
      
      return ; 
    } // else 

  } // else if 
  else if ( IsSystemPrimitive( temp->type ) )
  {
    mresult.clear() ; 
    mresult.assign( new_vector.begin(), new_vector.end() ) ; 
    return ; 
  } // else if 
  else 
  { // temp == emptyptr || temp == bounding symbol 
    temp = temp->listPtr ; // first LEFT_PAREN
    firstArgument = temp->next ; 

    if ( FindMap( firstArgument->token, new_vector ) )
    {
      if ( IsSystemPrimitive( new_vector.at( 0 ).type ) )
      {
        firstArgument->token = new_vector.at( 0 ).token ; 
        firstArgument->type = new_vector.at( 0 ).type ; 
      } // if 
    } // if

    if ( firstArgument->type == QUOTE ) {
      mexeNode = firstArgument ; 
    } // if
    else if ( IsNonList( temp ) ) // non list error 
    {
      hasError = true ; 
      throw new NonListException( mnonListVec ) ; 
    } // else if 
    else if ( IsATOM( firstArgument ) && firstArgument->type != SYMBOL && 
              NOT IsSystemPrimitive( firstArgument->type ) )  
    { // non known function
      hasError = true ; 
      throw new NonFunctionException( firstArgument->token ) ; 
    } // else if 

    else if ( firstArgument->type == SYMBOL || IsSystemPrimitive( firstArgument->type ) )
    { 
      if ( IsSystemPrimitive( firstArgument->type ) )
      { 
        
        if ( firstArgument->type == CLEAN_ENVIRONMENT || firstArgument->type == EXIT )
        {
          if ( mlevel > 1 )
          { 
            throw new ErrorLevelException( firstArgument->token ) ; 
          } // if 

          mexeNode = firstArgument ; 
        } // if 
        else if ( firstArgument->type == DEFINE || firstArgument->type == COND || 
                  firstArgument->type == LET || firstArgument->type == LAMBDA ) 
        { 
          if ( firstArgument->type == DEFINE )
          {
            if ( mlevel > 1 )
            { 
              throw new ErrorLevelException( firstArgument->token ) ; 
            } // if
            else
            {
              // CheckDefineFormat() ; 
            } // else 
          } // if 
          else if (  firstArgument->type == COND  )
          { 
          } // else if 
          else if ( firstArgument->type == LET )
          {
             // CheckLetFormat() ; 
          } // else if 
          else if ( firstArgument->type == LAMBDA )
          { 

          } // else if 

          mexeNode = firstArgument ; 
        } // else if  
        else if ( firstArgument->type == IF )
        { 
          mexeNode = firstArgument ; 
          if ( CheckNumOfArg( 2 ) || CheckNumOfArg( 3 ) ) 
          {
            mexeNode = firstArgument ; 
          } // if 
          else
          {
            hasError = true ;
            throw new IncorrectNumberException( "if" ) ;
          } // else 
        } // else if 
        else if ( firstArgument->type == AND || firstArgument->type == OR )
        { 
          mexeNode = firstArgument ; 
          if ( NOT CheckNumOfArg( 1 ) && NOT CheckNumOfArg( 0 ) ) // >= 2 
          {
            mexeNode = firstArgument ; 
          } // if 
          else
          {
            hasError = true ;
            throw new IncorrectNumberException( firstArgument->token ) ; 
          } // else

                             
        } // else if
        else  
        {
          mexeNode = firstArgument ; 
        } // else 
      } // if
      else // SYM is not the name of a known function
      {
         
        if ( FindMap( firstArgument->token, new_vector ) == false )
        {
          hasError = true ;
          throw new UnboundException( firstArgument ) ; 
        } // if 
        else
        {
          if ( NOT ThisIsFunction( firstArgument->token ) )  
          {
            hasError = true ;
            throw new NonFunctionException( new_vector ) ; 
          } // if() 
          
          mexeNode = firstArgument ;
          callFunction = true ; 
            
          
        } // else 
      } // else 
    } // else if 
    else // the first argument of ( ... ) is (。。。), i.e., it is ( (。。。) ...... )
    {
      mexeNode = firstArgument ; 
      Eval() ;

      
      if ( NOT firstArgument->vec.empty() && 
           firstArgument->vec.size() == 1 && 
           IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) ) 
      {
        firstArgument->token = firstArgument->vec.at( 0 ).token ; 
        firstArgument->type = firstArgument->vec.at( 0 ).type ;
        // cout << firstArgument->token << "  " ;
        mexeNode = firstArgument ;
      } // if 
      else if ( NOT firstArgument->vec.empty() 
                && NOT IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) )
      {
        throw new NonFunctionException( firstArgument->vec ) ; 
      } // else if 
      else
      {
        // cout << "ERROR : the vec is empty" ;
        hasError = true ; 
      } // else 

    } // else 

  } // else if

  if ( callFunction == true )
  {
    // cout << "call function" << mexeNode->token << endl ;
    CallFunction() ;
    // cout << "Extention function : " ;
    // ClearEmptyPtr( gRoot ) ;
    // PreOrderTraversal( gRoot ) ; 
    // cout << endl ; 

    if ( mexeNode->type == EMPTYPTR )
    {
      firstArgument = mexeNode ; 
      // cout << "FirstArgument : " << firstArgument->token << endl ;
      Eval() ;
      // cout << "FirstArgument : " << firstArgument->token << " "<< firstArgument->vec.size() << endl ;
      if ( NOT firstArgument->vec.empty() && 
           firstArgument->vec.size() == 1 && 
           IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) ) 
      {
        firstArgument->token = firstArgument->vec.at( 0 ).token ; 
        firstArgument->type = firstArgument->vec.at( 0 ).type ;
        // cout << firstArgument->token << "  " ;
        mexeNode = firstArgument ;
      } // if 
      else if ( NOT firstArgument->vec.empty() &&
                NOT IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) )
      {
        throw new NonFunctionException( firstArgument->vec ) ; 
      } // else if 
      else
      {
        // cout << "ERROR : the vec is empty" ;
        hasError = true ; 
      } // else
    } // if 
 
  } // if 

  while ( mexeNode->type == LAMBDA_FUNC )
  {
    // cout << "Call : " << mexeNode->token << endl ;
    if ( firstArgument->type == LAMBDA_FUNC )
    {
      // cout << "CallLambdaFunction : " << mexeNode->token << endl ;
      CallLambdaFunction() ; 
      // cout << "Extention function : " ;
      // PreOrderTraversal( gRoot ) ; 
      // cout << endl ; 

      if ( mexeNode->type == EMPTYPTR )
      {
        firstArgument = mexeNode ; 
        // cout << "FirstArgument is EMPTY call Eval() : " << firstArgument->token << endl ;
        Eval() ;

        if ( NOT firstArgument->vec.empty() && 
             firstArgument->vec.size() == 1 && 
             IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) ) 
        {
          firstArgument->token = firstArgument->vec.at( 0 ).token ; 
          firstArgument->type = firstArgument->vec.at( 0 ).type ;
          // cout << "Now FirstArgument is [" << firstArgument->token << "]" << endl ;
          mexeNode = firstArgument ;
        } // if 
        else if ( NOT firstArgument->vec.empty() &&
                  NOT IsSystemPrimitive( firstArgument->vec.at( 0 ).type ) )
        {
          throw new NonFunctionException( firstArgument->vec ) ; 
        } // else if 
        else
        {
          // cout << "ERROR : the vec is empty" ;
          hasError = true ; 
        } // else
      } // if 

    } // if

  } // while 

   
  if ( hasError == false ) {
    // cout << "Now Execute [ " << mexeNode->token << " ]" << endl ;
    Execute() ;
    // cout << endl << "Execute done : " << mexeNode->token << endl ; 
    // PreOrderTraversal( gRoot ) ; 
    // cout << endl ;
    
  } // if 

} // Functions::Eval()

bool PrintRoot() 
{
  if ( NOT gHead->vec.empty() )
  {
    DeleteDotParen( gHead->vec ) ; 
    FixSystemPrimitiveAndNil( gHead->vec ) ;
    PrintS_EXP( gHead->vec ) ; 
    return true ; 
  } // if 
 
  return false ; 
} // PrintRoot()

static int uTestNum = -1 ; 

int main() { 
  cin >> uTestNum ; 
  int i = 0 ;
  bool syntaxIsTrue = false ;
  vector<Type> myStack ; 
  vector<DotCheck> dotStack ; 
  bool hasDot = false ; 

  cout << "Welcome to OurScheme!" << endl  ;
  
  bool quit = false ; 
  bool readEXP = true ;
  
  vector<EXP> s_exp ;
  EXP nextToken ;
  InitExp( nextToken ) ; 
  EXP lastToken ; 
  InitExp( lastToken ) ;

  Functions * funcClass = new Functions() ; 
  funcClass->InitFunc() ; 
  funcClass->ResetMemNum() ;

  while ( NOT quit )
  {
    cout << endl << "> " ; 
    readEXP = true ; 
    gNowColumn = 0 ; 
    s_exp.clear() ; 
    myStack.clear() ;
    dotStack.clear() ;
    funcClass->ResetLevel() ;
    funcClass->InitExeNode() ;
    funcClass->ClearLocalMap() ;
    funcClass->ClearResult() ; 
    gHead = NULL ;
    DeleteTree( gRoot ) ; 
    gRoot = NULL ;
    
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
            throw new SyntaxErrorException( SYNERR_RIGHTPAREN, nextToken ) ; 
          } // if 
        
        } // if 

        if ( nextToken.type == RIGHT_PAREN && NOT s_exp.empty() && s_exp.back().type == DOT )
        {
          throw new SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
        } // if 

        if ( nextToken.type == DOT && NOT s_exp.empty() && s_exp.back().type == LEFT_PAREN )
        {
          // cout << endl << endl << "DOT ERROR" << endl ; 
          throw new SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
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
            throw new SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
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
            // cout << endl << endl << "DOT ERROR : " << nextToken.column << endl ;
            throw new SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
          } // if 
          else if ( myStack.back() == DOT ) 
          {
            // cout << " has Exception" << endl ; 
            throw new SyntaxErrorException( SYNERR_RIGHTPAREN, nextToken ) ; 
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
          else 
          {
            
            
            FixQuote( s_exp ) ; 
            FixToken( s_exp ) ;
            DeleteDotParen( s_exp ) ;

            funcClass->ClearLocalMap() ;
            funcClass->InitExeNode() ;
            funcClass->ResetLevel() ; 
            gHead = NULL ;
            DeleteTree( gRoot ) ; 
            gRoot = NULL ;
            i = 0 ;
            BuildTree( s_exp, i ) ;
            gHead = gRoot ; // new

            gnum = 0 ;
            S_EXP( gHead ) ; 
            gLastRow = s_exp.at( s_exp.size() - 1 ).nowRow ;
            funcClass->SetRoot() ;
            funcClass->Eval() ;
            funcClass->ResetLevel() ; 
            // funcClass->PrintMap() ; 
            

            if ( PrintRoot() == false )
            {
              // cout << "mResult" << "  " ; 
              vector<EXP> result = funcClass->GetResult() ; 
              cout << PrettyString( result ) ;
              funcClass->ClearResult() ; 
            } // if  

             
            readEXP = false ;

          } // else if 
        } // if 
        else if ( myStack.back() == DOT && myStack.size() == 1 )
        {
          throw new SyntaxErrorException( SYNERR_ATOM_PAR, nextToken ) ; 
        } // else if 

      } // try 
      catch ( NotAStringException * ex ) // has no closing quote exception 
      {
        cout << ex->What() << endl ; 

        // After print an String Error Message
        // we should break the while( readEXP ) 
        readEXP = false ; 

        gLastRow = gNowRow ;
      } // catch  the No Closing Quote Exception 
      catch ( SyntaxErrorException * ex ) // SyntaxError 
      {
        

        cout << ex->What() << endl ; 
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
        gNowColumn = 0 ; 

      } // catch Syntax Exception
      catch ( EofException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ; 
        quit = true ; 
      } // catch 
      catch ( UnboundException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ; 
        gLastRow = gNowRow ;
      } // catch 
      catch ( NonListException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( IncorrectArgumentException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( IncorrectNumberException * ex )
      {
        funcClass->ResetLevel() ; 
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch
      catch ( NonFunctionException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( DivByZeroException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( DefineFormatException * ex )
      {
        cout << ex->What() ;
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( LambdaFormatException * ex )
      {
        cout << ex->What() ;
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( ErrorLevelException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( ExitException * ex )
      {
        ex->What() ; 
        readEXP = false ;
        quit = true ; 
        gLastRow = gNowRow ;
      } // catch 
      catch ( CondFormatException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ;
        gLastRow = gNowRow ;
      } // catch 
      catch ( NoReturnException * ex )
      {
        cout << ex->What() ; 
        readEXP = false ; 
        gLastRow = gNowRow ;
      } // catch 
      
    } // while ( readEXP )
    
  } // while ( NOT quit )  
  
  printf( "\nThanks for using OurScheme!" ) ;

  return 0 ;
    

} // main()
