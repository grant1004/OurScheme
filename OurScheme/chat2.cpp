#include <iostream>
#include <vector>
#include <string>

using namespace std;

// 定? dot pair ?型
struct DotPair {
    string car;
    DotPair* cdr;
    DotPair(string c, DotPair* d): car(c), cdr(d) {}
};

// ?列表??? dot pair
DotPair* list_to_dot_pair(vector<string>& lst) {
    if (lst.empty()) {
        return NULL;
    }
    if (lst.size() == 1) {
        return new DotPair(lst[0], NULL);
    }
    vector<string> cdr_lst(lst.begin()+1, lst.end());
    return new DotPair(lst[0], list_to_dot_pair(cdr_lst));
}

// ?出 dot pair
void print_dot_pair(DotPair* pair) {
    if (pair == NULL) {
        cout << "NIL";
        return;
    }
    cout << "(" << pair->car << " . ";
    print_dot_pair(pair->cdr);
    cout << ")";
}

// 示例
int main() {
    vector<string> lst ; 
    // ( a ( b c ) ( d e )) 
    lst.push_back("a");
    lst.push_back("(");
    lst.push_back("b");
    lst.push_back("c");
    lst.push_back(")");
    lst.push_back("(");
    lst.push_back("d");
    lst.push_back(".");
    lst.push_back("e");
    lst.push_back(")");
    
    DotPair* pair = list_to_dot_pair(lst);
    print_dot_pair(pair);
    cout << endl;
    return 0;
}
