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
  
  EMPTYPTR, 

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
  else if ( type == EMPTYPTR )
    return "EMPTY";

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
EXP GetToken ( ) {
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

int emptyNum = 0 ; 
bool returnToEmpty = false ; 
void InsertNode( EXP * root, EXP * node ) 
{
  /*
  左括號 往右邊跑 ( listPtr 方向 ) 
  其他一律往左邊跑 ( next 方向 )   
  */
   
  if ( root -> type == EMPTYPTR ) 
  {
    
    emptyNum ++ ; 
    cout << "Touch EMPTY : " << emptyNum << endl ; 
  } // if 
  
  
  
  
  if ( node -> type == LEFT_PAREN ) // 現在進來的是左括
  {
    
    if ( root -> type == RIGHT_PAREN ) 
    {
      returnToEmpty = true ; 
      cout << "1現在是 : " << PrintType(root -> type) << " 觸底了 回到上一個EMPTYPTR" << endl ; 
      return ; 
    } // if
    
    if ( root->type == EMPTYPTR && root -> listPtr == NULL ) 
    {
      root->listPtr = node ; 
      cout << "現在在EMPTY，他的右邊沒東西，所以在EMPTY右邊插入一個左括號" << endl ; 
    } // if 
    else if ( root->type == EMPTYPTR && root -> listPtr != NULL && emptyNum < gNumOfParen ) 
    {
      cout << "現在在EMPTY，但是右邊有東西，而且這不是他要的位置( emptyNum :" << emptyNum << " gNumOfParen : " << gNumOfParen << " ) " ; 
      cout << " ，所以要往右邊跑遞迴，右邊的東西是一個 : " << PrintType(root->listPtr->type) << endl ; 
      InsertNode( root->listPtr, node ) ;  
    } // else if 
    else if ( root->type == EMPTYPTR && root -> listPtr != NULL && emptyNum >= gNumOfParen ) 
    {
      cout << "現在在EMPTY，但是右邊有東西，但是這是他要的位置( emptyNum :" << emptyNum << " gNumOfParen : " << gNumOfParen << " )" ; 
      cout << "，所以要往左邊新增一個EMPTY，" ;
      if ( root -> next == NULL ) 
      {
        cout << "左邊沒東西可以直接生一個EMPTY" << endl  ;
        EXP * empty = new EXP() ; 
        empty -> token = "\"EMPTY\"" ; 
        empty -> type = EMPTYPTR ; 
        empty -> row = 0 ; 
        empty -> column = 0 ; 
        empty -> next = NULL ; 
        empty -> listPtr = NULL ; 
        root->next = empty ;
        InsertNode( root->next, node ) ; 
      } // if 
      else 
      {
        cout << "，左邊有東西了，要先找到NULL" << endl  ;
        InsertNode( root->next, node ) ;
      } // else 
      
    } // else if 
    else if ( root->type != EMPTYPTR && root -> next == NULL)  
    {
      EXP * empty = new EXP() ; 
      empty -> token = "\"EMPTY\"" ; 
      empty -> type = EMPTYPTR ; 
      empty -> row = 0 ; 
      empty -> column = 0 ; 
      empty -> next = NULL ; 
      empty -> listPtr = NULL ; 
      root->next = empty ;
      cout << "現在不是EMPTY，現在root是 \""<< root->token << "\"，所以要新增一個EMPTY放在左邊，然後傳EMPTY進去遞迴" << endl ;
      InsertNode( root->next, node ) ; 
    } // else if 
    else if ( root->type != EMPTYPTR && root -> next != NULL )  
    {
      cout << "現在不是EMPTY，現在root是 \""<< root->token << "\"，然後她的next不是NULL，所以要往下找到NULL" << endl ;
      InsertNode( root->next, node ) ;
    } // else if 
    
    cout << "2" << "  emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen <<  endl ; 
  
  } // if 
  else  // node 不是  (
  {
    
    if ( root -> type == RIGHT_PAREN ) 
    {
      returnToEmpty = true ; 
      cout << "2現在是 : " << PrintType(root -> type) << " 觸底了 回到上一個EMPTYPTR" << endl ; 
      return ; 
    } // if 
    
    if ( root->type != EMPTYPTR ) 
    {
      
      if ( root->next != NULL )
      {
        cout << "現在不是EMPTY，現在的root是 \""<< root->token << "\"，所以要往左邊做遞迴 把root->next 傳進去" << endl ;
        InsertNode( root->next, node ) ;
        
      } // if
      else if (  root->next == NULL )
      {
        cout << "現在不是EMPTY，現在的root是 \""<< root->token << "\"，然後root->next 是空的，所以把node和root->next 接起來" << endl ;
        root->next = node ; 
        
      } // else if 
    } // if 
    else if ( root->type == EMPTYPTR && emptyNum < gNumOfParen )
    {
      cout << "現在是EMPTY， 然後emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen << "  還沒到想要的EMPTY位置，所以要繼續找"<< endl ; 
      InsertNode( root->listPtr, node ) ; 
    } // else if 
    else if ( root->type == EMPTYPTR && emptyNum >= gNumOfParen )
    {

        cout << "現在是EMPTY， 然後emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen << "  到想要的EMPTY位置了，所以要先移到右邊的左括號"<< endl ; 
        InsertNode( root->listPtr, node ) ; 
      
    } // else if 
    else if ( root -> next == NULL ) 
    {
      cout << "emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen ; 
      cout << " 找到想要放的位置ㄌ，把 " << node->token << " 放進去 "<< endl ;
      root -> next = node ; 
    } // if 
    else // != NULL 
    {
      cout << "emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen ; 
      cout << "還沒找到 繼續找"<< endl ;
      InsertNode( root -> next, node ) ; 
    } // else 
 
    cout << "3 :" << PrintType(root->type) << "  emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen <<  endl ; 
  } // else 
    
  
  
  
  if ( root -> type != EMPTYPTR && returnToEmpty ) 
  {
    cout << "1反彈 現在的root : " << root -> token << "  emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen << endl ;   
    return ; 
  } // if 

  
  if ( root->type == EMPTYPTR && emptyNum >= gNumOfParen && returnToEmpty == true ) 
  { 
    returnToEmpty = false ; 
    cout << "現在是EMPTY， 然後emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen << "  到想要的EMPTY位置了，但是右邊結束了，所以要往左邊跑 "<< endl ; 
    if ( root->next == NULL ) 
    {
      cout << "但是如果左邊是NULL，就直接放進去" << endl ; 
      root->next = node ; 
    } // if 
    else 
    {
      cout << "但是如果左邊不是NULL，就遞迴root -> next " << endl ; 
      InsertNode( root->next, node ) ;
    } // else 
    
  } // if
  else if ( root->type == EMPTYPTR && emptyNum < gNumOfParen && returnToEmpty == true ) 
  {
    returnToEmpty = false ;
    cout << "現在是EMPTY， 然後emptyNum:" << emptyNum << " gNumOfParen:" << gNumOfParen << "  還沒到想要的EMPTY位置了，但是右邊結束了，所以要往左邊跑 "<< endl ;
    InsertNode( root->next, node ) ;
  } // else 
  
} // InsertNode( EXP * node ) 

EXP * root = new EXP() ; 

void preOrderTraversal(EXP* focusNode) 
{
  if (focusNode != NULL) {
    
    if ( focusNode->token != "\"EMPTY\"" ) 
    {
      cout << focusNode->token << " ";
    } // if 
    
    preOrderTraversal(focusNode->listPtr);

    preOrderTraversal(focusNode->next);
    

  } // if 
}  // preOrderTraversal(EXP* focusNode)  


EXP * NewNode( EXP data ) // 建立一個node 儲存EXP資料
{
  EXP * node = new EXP() ; 
  node -> token = data.token ; 
  node -> type = data.type ; 
  node -> column = data.column ;
  node -> row = data.row ; 
  node -> next = NULL ; 
  node -> listPtr = NULL ;
  return node ; 
} // New Node 


void BuildTree( vector<EXP> s_exp ) 
{
  gNumOfParen = 0 ; 
  root -> token = "\"EMPTY\""  ; 
  root -> type = EMPTYPTR ; 
  root -> next = NULL ; 
  root -> listPtr = NULL ; 
  for( int i = 0 ; i < s_exp.size() ; i ++ ) 
  {
    
    emptyNum = 0 ; 
    
    EXP * node = NewNode( s_exp.at( i )  ) ; 
    
    cout << endl ; 
    if ( node -> type == LEFT_PAREN ) 
    {  
      // cout << "ADD LEFT" << endl ; 
      gNumOfParen ++ ; 
    } // if 
  
    
    cout << "\""<< node->token << "\" " << PrintType(node -> type) << "   " ; 
    
    InsertNode( root, node ) ; 
    
    if ( node -> type == RIGHT_PAREN ) 
    {
      // cout << "ADD RIGHT" << endl ; 
      gNumOfParen -- ; 
    } // else if 
  } // for 
   
} // BuildTree( vector<EXP> s_exp ) 



int main() { // 這是用vector的版本

  bool readEXP = true ;
  int parnum = 0 ; 
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
        if ( nextToken.type != NONE ) // 還沒讀到 EOF，還有其他指令還沒讀
        {
          PrintS_EXP( s_exp ) ;   
          BuildTree( s_exp ) ;          
          cout << endl << "EXP DONE" << endl << "================================" << endl ; 
          preOrderTraversal( root ) ; 
          cout << endl ; 
        } // if 
        else // nextToken == NONE 代表讀到 EOF 了，沒有任何指令了 
        {
          cout << ">>> ALL s_exp Read Done " << endl ; 
        } // else 
        readEXP = false ; 
      } // if     
      
      
    } // while ( readEXP )  
    
    if ( nextToken.type == NONE ) // 讀到 EOF
    {
      ALL_EXP_DONE = true ;
    } // if 沒有 (exit) 
    else if ( s_exp.size() == 3 && s_exp.at( 0 ).token == "("  && s_exp.at( 1 ).token == "exit" && s_exp.at( 2 ).token == ")" )
    {
      ALL_EXP_DONE = true ;
      
    } // else if 
    
  } // while ( ALL_EXP ) 
    
  
} // main()     

