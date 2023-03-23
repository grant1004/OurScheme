
class Functions
{

private: 
  map<string,vector<EXP>> symbolMap ; // map symbol 
  EXP* exeNode ; 

public : 
  void SetRoot() ;
  void Eval( ) ; // 文法檢查 
  bool CheckNumOfArg( int num ) ; 
  void Define() ; // iris
  void Cons() ; // iris
  bool IsSystemPrimitive( Type type ) ;
  //    void Execute() ;
  void PrintMap() ;
  bool FindMap( string str, vector<EXP> &new_vector ) ;
  void Atom_qmark() ; // iris
  void Null_qmark() ; // iris
  void Integer_qmark() ; // iris
  void Real_qmark() ; // iris
  //    void Car() ; // iris
  //    EXP List() ; // list
  //    EXP Quote() ; // ' 
  //    EXP Cdr() ; // cdr 
  //    EXP Pair_qmark() ; // pair?
  //    EXP List_qmark() ; // list?
  //    EXP String_qmark() ; // string? 
  //    EXP Boolean_qmark() ; // boolean?
  //    EXP Symbol_qmark() ; // symbol?
  //    EXP Add() ; // + 
  //    EXP Sub() ; // -
  //    EXP Mul() ; // *
  //    EXP Div() ; // / 
  //    EXP Not() ; // not 
  //    EXP And() ; // and 
  //    EXP Or() ;  // or 
      EXP BiggerThan() ; // > 
  //    EXP BiggerEqual() ; // >= 
  //    EXP LessThan() ; // < 
  //    EXP LessEqual() ; // <=  
  //    EXP Equal() ; // = 
  //    EXP String_append() ; // string-append  
  //    EXP String_Bigger( ); // string> 
  //    EXP String_Less() ;   // string< 
  //    EXP String_Equal() ; // string=
  //    EXP Eqv_qmark() ; // eqv? 
  //    EXP Equal_qmark() ; // equal? 
  //    EXP Begin() ; // begin
  //    EXP If() ; // if 
  //    EXP Cond() ; // cond 
  //    EXP Clean_Environment() ; // clean-environment 

} // class 

void Functions::BiggerThan()
{

} // BiggerThan()