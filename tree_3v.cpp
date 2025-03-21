#include <iostream>
#include <string>
#include <cmath>
#include <vector>
using namespace std;

/*
OurScheme 是基於 Lisp/Scheme 的一種語言
語法以 S-Expression（S-Exp）為基礎
由原子（Atoms）和清單（Lists）組成
區分大小寫  apple != Apple
支援數字、字串、符號（Symbols）、布林值等

<S-exp> ::= <ATOM>
            | Left-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] Right-PAREN
            | QUOTE <S-exp>
<ATOM> ::= SYMBOL | INT | FLOAT | STRING | NIL | T | Left-PAREN Right-PAREN

{} appear 0 or more
[] appear 0 or 1
*/

/*
! Proj1 完成步驟
* 建立 REPL（Read-Eval-Print-Loop）
    *while 迴圈不斷讀取輸入直到 (exit) 或 EOF。
* 實作 ReadSExp()
    *解析 token，構建 AST（Abstract Syntax Tree）。
* 實作 PrintSExp()
    *遵循 pretty-print 規則 來輸出 S-Expression。
* 實作錯誤處理
    *未預期的 token、括號錯誤、EOF、字串錯誤都要報錯並跳過當行。
*/

enum TokenType {
    INT = 1, // accept +&-, e.g., '123', '+123', '-123'
    FLOAT = 2, // '123.567', '123.', '.567', '+123.4', '-.123'
    STRING = 2222, // need use "", cannot separate by line

    Left_PAREN = 3, // ( begin of list
    Right_PAREN = 9, // ) end of list

    DOT = 1000, // . dot and dot pair
    NIL = 10, // ();false; null
    T = 4, // true
    QUOTE = 1111, // '
    SYMBOL = 5, // 以 identifier 表示的資料，變數名稱, a type of atom
           // a consecutive sequence of printable characters that are
           // not numbers, strings, #t or nil, and do not contain 
           // '(', ')', single-quote, double-quote, semi-colon and 
           // white-spaces ; 
           // Symbols are case-sensitive 
           // (i.e., uppercase and lowercase are different);
    None = 0

};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    int level;

    Token() : type(SYMBOL), value(""), line(0), column(0), level(0) {}
};

struct Node_Token {
    Token token; // token info, value, type

    Node_Token *parent;
    Node_Token *left;
    Node_Token *right;
    Node_Token() : parent(nullptr), left(nullptr), right(nullptr) {}
};

enum errorType {
    // ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
    UNEXPECTED_TOKEN = 0, // atom or '(' expected

    // ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
    UNEXPECTED_CLOSE_PAREN = 1, // unexpected close parenthesis error, error (1 2 . 3 4) , correct is (1 2 . (3 4))
    UNEXPECTED_END_PAREN = 2,
    // ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
    UNEXPECTED_STRING = 3, // unexpected string error ("hello)

    //ERROR (no more input) : END-OF-FILE encountered
    UNEXPECTED_EOF = 123, // END-OF-FILE encountered
    
    // UNEXPECTED_DOT // unexpected dot error (1 2 . 3 4) , correct is (1 2 . (3 4))
    UNEXPECTED_EXIT = 5, // (exit)
    Error_None = 99999
};

/* error message 
> ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 1 is >>)<<
*/
class Error {

public:
    errorType type;
    string message;
    string expected;
    string current;    
    int line;
    int column;

    Error() : type(Error_None), message(""), line(0), column(0), expected(""), current("") {}
    Error(errorType t, string e_token, string c_token, int l, int c) {
        type = t;
        // expected = e_token;
        // current = c_token;
        line = l;
        column = c;

        if (t == UNEXPECTED_TOKEN)
            message = "ERROR (unexpected token) : " + e_token + " expected when token at Line " + to_string(line) + " Column " + to_string(column) + " is >>" + c_token + "<<";
        else if (t == UNEXPECTED_CLOSE_PAREN)
            message = "ERROR (unexpected token) : " + e_token + " expected when token at Line " + to_string(line) + " Column " + to_string(column) + " is >>" + c_token + "<<";
        else if (t == UNEXPECTED_END_PAREN)
            message = "ERROR (unexpected token) : " + e_token + " expected when token at Line " + to_string(line) + " Column " + to_string(column) + " is >>" + c_token + "<<";
        else if (t == UNEXPECTED_STRING)
            message = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + to_string(line) + " Column " + to_string(column+1);
        else if (t == UNEXPECTED_EOF)
            message = "ERROR (no more input) : END-OF-FILE encountered";
        else if (t == UNEXPECTED_EXIT)
            message = "exit";
    }
};

/*
 in order to parse the input, build a parser tree
*/
class AST_Tree {
    private:
    // SyntaxAnalyzer syntax;

    Node_Token *root;
    vector<Token> tokens;


    bool is_s_exp(Token &token) {
        // bool legal = true;
        if (token.type == SYMBOL || token.type == INT || token.type == FLOAT || token.type == STRING || token.type == NIL || token.type == T) // || type == Left_PAREN || type == Right_PAREN
            return true;
        else if (token.type == Left_PAREN)
            return true;
        else if (token.type == QUOTE)
            return true;
        
        // token.type == Right_PAREN || token.type == DOT || token.type == None
        return false;
    }

    Token set_token(string str, TokenType type, int level, int line = 0, int column = 0) {
        Token tmp;
        tmp.value = str;
        tmp.type = type;
        tmp.level = level;
        tmp.line = line;
        tmp.column = column;
        return tmp;
    }

    Node_Token* set_node(Token token, Node_Token *l, Node_Token *r, Node_Token *p) {
        Node_Token *tmp = new Node_Token();
        tmp->token = token;
        tmp->left = l;
        tmp->right = r;
        tmp->parent = p; //! ????????????????????????
        cerr << "\033[1;33mset_node value: " << token.value << "\033[0m" << endl;
        return tmp;
    }

    // Node_Token* insert_quote

    Node_Token* insert(Node_Token *parent, vector<Token> &tokenbuffer, int &index, bool create_node = false, int level = 0) {
        int size = tokenbuffer.size();
        Node_Token *node = nullptr;
        Token none_token = set_token("None", None, -1);
        Token nil_token = set_token("#f", NIL, -1);
        cout << "\033[1;32mindex: " << index << "\033[0m" << endl;

        if (index > size) {
            cerr << "\033[1;31m* 2. touch bottom *\033[0m" << endl;
            return nullptr;
        }
        else if (index == size) {
            // index++;
            cerr << "\033[1;31m* 1. touch bottom *\033[0m" << endl;
            node = set_node(nil_token, nullptr, nullptr, parent);
            return node;
        }

        // **************************** setting token ****************************
        // !last_token
        Token last_token;
        if (index > 0) last_token = tokenbuffer.at(index - 1);
        else  last_token = set_token("None", None, -1);
        cerr << "last_token: " << last_token.value << endl;
        // !current_token
        Token current_token = tokenbuffer.at(index);
        cerr << "current_token: " << current_token.value << endl;
        index++;
        cerr << "\033[1;32m" << "index++ " << "\033[0m" << endl;
        // !next_token
        Token next_token;
        if (index < size) next_token = tokenbuffer.at(index);
        else next_token = set_token("None", None, -1);
        cerr << "next_token: " << next_token.value << endl;
        // *********************************************************
        
        
        // current is s-exp or not
        if (is_s_exp(current_token)) {
            cerr << "\033[1;35mis_s_exp\033[0m" << endl;
            
            if (current_token.type == Left_PAREN) {
                //? **************
                // 當前 node 內會存 "("
                cerr << "\033[1;35m" << "** Left_PAREN **" << "\033[0m" << endl;
                // ! I think will not enter in here
                if (next_token.type == Right_PAREN) {
                    cerr << "\033[1;35m" << "** I think will not enter in here **" << "\033[0m" << endl;
                    // node 設為 nil，並將其 left、right 指向 nullptr，回傳給 parent->left or parent->right
                    node = set_node(nil_token, nullptr, nullptr, parent); // 為 leaf node，不需再往下 insert
                    // cerr << "set_node value: " << nil_token.value << endl;
                    index++; // 回傳給 parent 後 skip 下個 ")"
                    cerr << "\033[1;32m" << "index++ " << "\033[0m" << endl;
                    node->parent->right = insert(node->parent, tokenbuffer, index, true);
                    
                }
                // !
                // (s-exp)
                else {
                    // 當前 node 內會存 "("
                    node = set_node(current_token, nullptr, nullptr, parent);
                    /*
                    if (next_token.type == QUOTE) {
                        cerr << "\033[1;32m" << "** start bulid side quote tree **" << "\033[0m" << endl;
                        int start = index;
                        int end = index;
                        do { end++; }
                        while(tokenbuffer.at(end).type != Right_PAREN && tokenbuffer.at(end).level!=tokenbuffer.at(start).level);
                    
                        vector<Token> side_token(tokenbuffer.begin() + start-1, tokenbuffer.begin() + end);
                        for (auto &t : side_token) {
                            cerr << "side_token: " << t.value << endl;
                        }
                        node->left = insert(node, tokenbuffer, index);
                        cerr << "\033[1;32m" << "** end bulid side quote tree **" << "\033[0m" << endl;
                        
                    }
                    */
                    
                    node->left = insert(node, tokenbuffer, index);
                    
                    
                    cerr << "\033[1;32m" << "back to ('s , start right_child" << "\033[0m" << endl;
                    node->right = insert(node, tokenbuffer, index, true);
                }
                
            }
            else if (!create_node && current_token.type == QUOTE) {
                cerr << "\033[1;35m" << "** QUOTE **" << "\033[0m" << endl;
                node = set_node(none_token, nullptr, nullptr, parent);
                // *                   . 
                // *                 /  \
                // *left_quote->  quote *none  <-right_quote
                // *             /    \
                // *          null   null
                Node_Token *left_quote = new Node_Token;
                // current_token.value = "quote";
                left_quote = set_node(current_token, nullptr, nullptr, node);
                node->left = left_quote;

                Node_Token *right_quote = new Node_Token;
                right_quote = set_node(none_token, nullptr, nullptr, node);
                node->right = right_quote;
                right_quote->parent = node;

                // *      none  <-right_quote
                // *     /   \
                //   
                right_quote->left=insert(right_quote, tokenbuffer, index);
                // index++;
                right_quote->right=insert(right_quote, tokenbuffer, index);
                // index++;
            }
            // ATOM := SYMBOL | INT | FLOAT | STRING | NIL | T
            else {
                cerr << "\033[1;35m" << "** ATOM := SYMBOL | INT | FLOAT | STRING | NIL | T  **" << "\033[0m" << endl;
                // ((()()))=((#f #f))

                if (size == 1 || !create_node || last_token.type == QUOTE) {
                    cerr << "\033[1;35m" << "** create_node == F **" << "\033[0m" << endl;
                // if (size == 1 || last_token.type == Left_PAREN || last_token.type == QUOTE) {
                    // 中文：如果上個 token 是 '(' or '.'，則不建立新節點
                    // !create_node if is someone's right child
                    node = set_node(current_token, nullptr, nullptr, parent);
                    // index++;
                    // node->right = insert(node, tokenbuffer, index);
                }
                // else if (last_token.type != Left_PAREN && last_token.type != DOT) {
                else if (create_node && last_token.type != DOT) {
                    cerr << "\033[1;35m" << "** create_node == T **" << "\033[0m" << endl;
                    // 上一個 token 不是 '(' or '.'，則當前 token 為右子樹，需新建空節點
                    node = set_node(none_token, nullptr, nullptr, parent); // 空節點

                    if (current_token.type == QUOTE) {
                        cerr << "\033[1;32m" << "** start bulid side quote tree **" << "\033[0m" << endl;
                        int start = index;
                        int end = index;
                        do { end++; }
                        while(tokenbuffer.at(end).type != Right_PAREN && tokenbuffer.at(end).level!=tokenbuffer.at(start).level);
                    
                        
                        vector<Token> side_token(tokenbuffer.begin() + start-1, tokenbuffer.begin() + end);
                        // for (auto &t : side_token) {
                        //     cerr << "side_token: " << t.value << endl;
                        // }
                        start = 0 ;
                        node->left = insert(node, side_token, start);
                        index = end+1;
                        cerr << "********* index: " << index << endl;
                        cerr << "\033[1;32m" << "** end bulid side quote tree **" << "\033[0m" << endl;
                    }
                    else {
                        Node_Token *left_atom = set_node(current_token, nullptr, nullptr, node);
                        node->left = left_atom;
                    }
                    

                    node->right = insert(node, tokenbuffer, index, true);

                }
                /*if (next_token.type == QUOTE) {
                    cerr << "\033[1;32m" << "** start bulid side quote tree **" << "\033[0m" << endl;
                    int start = index;
                    int end = index;
                    do { end++; }
                    while(tokenbuffer.at(end).type != Right_PAREN && tokenbuffer.at(end).level!=tokenbuffer.at(start).level);
                
                    vector<Token> side_token(tokenbuffer.begin() + start-1, tokenbuffer.begin() + end);
                    // for (auto &t : side_token) {
                    //     cerr << "side_token: " << t.value << endl;
                    // }
                    node->left = insert(node, tokenbuffer, index);
                    cerr << "\033[1;32m" << "** end bulid side quote tree **" << "\033[0m" << endl;
                    
                }*/
                // else {
                //     node->right = insert(node, tokenbuffer, index);
                //     // index++;
                // }
                
            }
            
        }
        // Right_PAREN
        else if (current_token.type == Right_PAREN) {
            cerr << "\033[1;35m---------------is_Right_PAREN-------------\033[0m" << endl;
            node = set_node(nil_token, nullptr, nullptr, parent); // 為 leaf node，不需再往下 insert
            

            if (last_token.type == DOT) { // (a . b), ')' 前是 b，不會是 '.'
                cerr << "\033[1;35m---------------will not reach here-------------\033[0m" << endl;
            }
            
            
            // cerr << "set_node value: " << current_token.value << endl;
        }
        // DOT
        else if (current_token.type == DOT) {
            cerr << "\033[1;35mis_DOT\033[0m" << endl;
            // 回到當前 node 的 parent->right 去 insert nil 進去，並將其 left、right 指向 nullptr
            node->parent->right = insert(node->parent, tokenbuffer, index);

        }

        return node;
        

    }


    void print_token(Token &t) {
        for(auto &c : t.value) {
            if (c == ' ')
                cout << "nul ";
            else
                cout << c;
        }
        cout << " line: " << t.line << " ";
        cout << "column: " << t.column << " ";
        cout << "level: " << t.level << " ";
        cout << "type: ";
        switch (t.type) {
            case INT: cout << "INT"; break;
            case FLOAT: cout << "FLOAT"; break;
            case STRING: cout << "STRING"; break;
            case Left_PAREN: cout << "Left_PAREN"; break;
            case Right_PAREN: cout << "Right_PAREN"; break;
            case DOT: cout << "DOT"; break;
            case NIL: cout << "NIL"; break;
            case T: cout << "T"; break;
            case QUOTE: cout << "QUOTE"; break;
            case SYMBOL: cout << "SYMBOL"; break;
            default: cout << "UNKNOWN"; break;
        }
        cout << " \n";
    }

    void print_tree(Node_Token *node) {
        if (node == nullptr) return;
        
        print_token(node->token);
        cerr << "node->left" << endl;
        print_tree(node->left);
        // print_token(node->token); // in-order traversal
        cerr << "node->right" << endl;
        print_tree(node->right);
    }


    public:
    AST_Tree() : root(nullptr) {}

    void build_AST(vector<Token> tokenBuffer) {
        tokens = tokenBuffer;

        cout << "\033[1;34menter build_AST\033[0m" << endl;
        int index = 0;

        if (!tokens.empty()) { // at least 3 tokens && tokenBuffer.size() > 2
            root = insert(root, tokens, index);
        }

            
        cout << "\033[1;34mend build_AST\033[0m" << endl;

        print();
    }

    void print() {
        cout << "\033[1;34menter print_tree\033[0m" << endl;
        print_tree(root);
        cout << "\033[1;34mend print_tree\033[0m" << endl;

    }

    Node_Token *get_root() {
        return root;
    }
    
    void clear_tree(Node_Token *node) {
        if (node == nullptr) {
            delete node;
            node = nullptr;
            return;
        }
        clear_tree(node->left);
        clear_tree(node->right);
        delete node;
    }

    ~AST_Tree() {
        clear_tree(root);
        tokens.clear();
    }
};
        
class SyntaxAnalyzer {
private:
    AST_Tree tree;
    int currnt_level = 0;
    vector <pair<int, bool>> dot_appear; // first: current_level, second: appear or not

    // vector<Token> tokenBuffer;

    bool is_ATOM(TokenType type) {
        // <ATOM> ::= SYMBOL | INT | FLOAT | STRING | NIL | T | Left-PAREN Right-PAREN
        if (type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T) // || type == Left_PAREN || type == Right_PAREN
            return true;
        // QUOTE、DOT
        return false;
    }

    bool is_s_exp(Token &token) {
        // bool legal = true;
        if (is_ATOM(token.type)) 
            return true;
        else if (token.type == QUOTE || token.type == Left_PAREN || token.type == Right_PAREN) 
            return true;

        return false;
    }

    void renew_dot_appear(TokenType t, int level) {
        for (; currnt_level <= level; currnt_level++)
            dot_appear.push_back({currnt_level, false});
        

        // cout << "start loop" << endl;
        // for (int i = 0; i < dot_appear.size(); i++) {
        //     cout << "level: " << dot_appear.at(i).first << " appear: " << dot_appear.at(i).second << endl;
        // }
        // cout << "end loop" << endl;

        // if (currnt_level < level) {
        //     currnt_level = level;
        //     dot_appear.push_back({level, false});
        // }

        if (t == DOT)
            dot_appear.at(level).second = true;
    }

    bool check_dot_appear(int level) {
        int size = dot_appear.size();
        if (level == size)  // 目前 level=0 時，若 size=0 表當前 level 還未進入過，且未出現過 dot
            dot_appear.push_back({level, false}); // 進入當前 level
        
        else if (level < size)  // check whether dot appear in current level or not
            return dot_appear.at(level).second;
        else
            cerr << "\033[1;31mundefined error\033[0m" << endl;
        
        return false;
    }

public:
    SyntaxAnalyzer() {
        currnt_level = 0;
        // dot_appear.push_back({0, false});
    }

    void build_tree(vector<Token> &v) {
        
        tree.build_AST(v);
        // tree.print();
        // tree.clear_tree(tree.get_root());
        
        
    }

    void check_syntax(vector<Token> &tokenBuffer, errorType &error) {
        int token_size = tokenBuffer.size();

        // cerr << "\033[1;34menter check_syntax\033[0m" << endl;
        // cerr << "tokenBuffer.size(): " << tokenBuffer.size() << endl;
        if (tokenBuffer.size() == 1) {
            // cerr << "tokenBuffer.size() == 1" << endl;
            // . 
            if (!is_ATOM(tokenBuffer.at(0).type) && tokenBuffer.at(0).type != QUOTE  && tokenBuffer.at(0).type != Left_PAREN) {
                // cerr << "tokenBuffer.at(0).type: " << tokenBuffer.at(0).type << endl;
                error = UNEXPECTED_TOKEN;
            }
            
            
        }
        else if (token_size > 1) {
            // cerr << "token_size > 1" << endl;
            int index_curr = token_size - 1;
            int index_prev = token_size - 2;
            if (tokenBuffer.at(index_prev).value == "\'" && !is_s_exp(tokenBuffer.at(index_curr))) {
                error = UNEXPECTED_TOKEN;
            }

            // DOT
            if (tokenBuffer.at(index_curr).type == DOT) {
                // DOT 前後必須是完整 ATOM 或是 S-EXP
                // 避免 ('. or (. 
                if (tokenBuffer.at(index_prev).type == QUOTE || tokenBuffer.at(index_prev).type == Left_PAREN)
                    error = UNEXPECTED_TOKEN;
                if (check_dot_appear(tokenBuffer.at(index_curr).level) == true) 
                    error = UNEXPECTED_END_PAREN;

            }
            // right Paren
            if (tokenBuffer.at(index_curr).type == Right_PAREN) {
                // Right_PAREN 前必須是完整 ATOM 或是 S-EXP
                // 避免 ') or .) 
                if (tokenBuffer.at(index_prev).type == QUOTE || tokenBuffer.at(index_prev).type == DOT) {
                    error = UNEXPECTED_TOKEN;
                }
                // () 時, 改成(nil)
                /*if (tokenBuffer.at(index_prev).type == Left_PAREN) {
                    Token nil_token;
                    nil_token.type = NIL;
                    nil_token.value = "nil";
                    nil_token.line = tokenBuffer.at(index_prev).line;
                    nil_token.column = tokenBuffer.at(index_prev).column + 1;
                    nil_token.level = tokenBuffer.at(index_prev).level + 1;
                    tokenBuffer.insert(tokenBuffer.begin() + index_curr, nil_token);
                    index_curr++;
                }*/
            }
            renew_dot_appear(tokenBuffer.at(index_curr).type, tokenBuffer.at(index_curr).level);
        }

        // cerr << "\033[1;34mend check_syntax\033[0m" << endl;
    }

    void print_space(int n) {
        for (int i = 0; i < n; i++)
            cout << " ";
    }

    void pretty_print(vector<Token> &tokenBuffer) {
        int level = 0;
        cerr << "\033[1;34menter pretty_print\033[0m" << endl;
        for (int i = 0; i < tokenBuffer.size(); i++) {
            Token current_Token = tokenBuffer.at(i);
            if (current_Token.type == Left_PAREN) {
                print_space(level);
                cout << "( " ;
                level++;
            }
            else if (current_Token.type == Right_PAREN) {
                level--;
                print_space(level);
                cout << ")" << endl;
            }
            else {
                // print_space(level);
                cout << current_Token.value << endl;
            }
        }
        cerr << "\033[1;34mend pretty_print\033[0m" << endl;
    }

    ~SyntaxAnalyzer() {

    }


};

class LexicalAnalyzer {
private:
    
    int line, column; // '(" ", "\t"， "\")保留
    int leftParen = 0, rightParen = 0; // count '(' and ')'
    int level;
    bool is_EOF;
    // bool dot_appear = false;
    // pair<bool, bool> dot_appear = {false, false}; // first: is_dot, second: enter the second atom

    int error_line, error_column;
    // string ExpectedToken, CurrentToken;

    void reset_Line_And_Column(int l, int c) {
        line = l;
        column = c;
        // cerr << "reset line: " << g_line << " column: " << g_column << endl;
    }

    bool is_space(char token) { // check whether the token is ' ' or '\t'
        if (token == ' ' || token == '\t')
            return true;
        return false;
    }

    bool is_enter(char token) { // check whether the token is '\n' or '\r'
        if (token == '\n' || token == '\r')
            return true;
        return false;
    }

    bool is_PAREN(char token, bool &finish_input) { // check whether the token is '(' or ')'
        bool is_PAREN = false;

        if (token == '(' || token == ')') {
            is_PAREN = true;
            if (token == '(') {// count '('
                leftParen++;
                level++;
            }
            else {// count ')'
                rightParen++;
                level--;
            }
        }

        return is_PAREN;
    }
    
    bool is_comment(char token ) {
        if ( token == ';' )
            return true;
        return false;
    }

    bool is_special_symbol(char token) {
        if (token == '(' || token == ')' || token == '\'' || token == '\"' || token == ';' || token == ' ' || token == '\t' || token == '\n' || token == '\r' || token == EOF)
            return true;
        return false;
    }

    bool is_int(string &str) {
        if (str.empty())
            return false;
        // + or -
        else if (str.size() == 1 && (str == "+" || str == "-"))
            return false;

        string tmp = str;
        int i = 0;
        while (i < str.size()) {
            // + or - appear at the first character
            if (i == 0 && (str.at(0) == '+' || str.at(0) == '-')) {
                if (str.at(0) == '+')
                    tmp = str.substr(1); // Remove the '+' sign from the string
                cerr << "int str tmp: " << tmp << endl;
                
                i++;
                continue;
            }
            if (!isdigit(str.at(i)))
                return false;
            i++;
        }

        str = tmp;
        // cerr << "END : int str: " << str << endl;
        return true;
    }

    bool is_float(string &str) {
        if (str.empty())
            return false;

        string tmp = str;
        string sign = "";
        int i = 0;
        int dot_count = 0;
        // - or +
        if (str.size() == 1 && (str.at(0) == '+' || str.at(0) == '-'))
            return false;
        // -. or +.
        else if (str == "-." || str == "+.")
            return false;

        while (i < str.size()) {
            if (i == 0 && (str.at(0) == '+' || str.at(0) == '-')) {
                sign = str.at(0); // save the sign
                tmp = str.substr(1); // Remove the sign from the string
                // i++;
                // continue;
            }
            else if (str[i] == '.') {
                dot_count++;
                if (dot_count > 1)
                    return false;
                if (i == 0 || !isdigit(str[i - 1]))
                    tmp.insert(tmp.begin(), '0'); // Insert '0' before the dot if no digit before it
                // i++;
                // continue;
            }
            else if (!isdigit(str[i]) && str[i] != '.')
                return false;
            i++;
        }
        
        // 四捨五入到小數點後第三位
        float float_value = stof(tmp);
        cout << "float_value: " << float_value << endl;
        float_value = round(float_value * 1000.0) / 1000.0;
        tmp = to_string(float_value);

        // 小數點後大於三位數, 只取到小數點後第三位
        int dot_pos = tmp.find('.');
        if (tmp.size() - dot_pos > 4)
            tmp = tmp.substr(0, dot_pos + 4);

        if (sign == "-")
            tmp = sign + tmp;

        str = tmp;
        // cerr << "END : float str: " << str << endl;
        return true;
    }

    TokenType check_token_type(string &str) {
        TokenType type;
        if (str == "#f") // str == "NIL" || str == "nil"
            type = NIL;
        else if (str == "T")
            type = T;
        else if (str == "'") // || str == "QUOTE"
            type = QUOTE;
        else if (str == "(") 
            type = Left_PAREN;
        else if (str == ")") 
            type = Right_PAREN;
        else if (str == ".") 
            type = DOT;
        else if (is_int(str))
            type = INT;
        else if (is_float(str))
            type = FLOAT;
        else if (str[0] == '\"')
            type = STRING;
        else
            type = SYMBOL;

        return type;
    }

    void read_whole_string(string &tmpstr, errorType &error, char start_char) {
        char c_peek = cin.peek();

        if (start_char == ';') {
            while(!is_enter(c_peek) && c_peek != EOF) {
                // c_peek = cin.peek();
                if (c_peek == EOF) {
                    error = UNEXPECTED_EOF;
                    // return;
                }
                else{
                    tmpstr.push_back(getchar());
                    reset_Line_And_Column(line, column+1); // column++;
                }
                
                c_peek = cin.peek();
            }
            if (c_peek == EOF) {
                error = UNEXPECTED_EOF;
                return;
            }
            else if (is_enter(c_peek) && leftParen!=rightParen) {
                getchar(); // ignore '\n'
                reset_Line_And_Column(line+1, 1);
            }
            else {
                getchar(); // ignore '\n'
                reset_Line_And_Column(1, 1);
            }
            
        }
        else if (start_char == '\"') {
            tmpstr.push_back(getchar()); // push_back first "
            reset_Line_And_Column(line, column+1);
            c_peek = cin.peek();
            while(c_peek!= '\"') {
                
                if (c_peek == EOF) {
                    error = UNEXPECTED_EOF;
                    return;
                }
                else if (is_enter(c_peek)) {
                    // reset_Line_And_Column(line, column+1); // ! ???????
                    error = UNEXPECTED_STRING;
                    // string trash;
                    // getline(cin, trash); // ignore the whole line
                    return;
                }
                tmpstr.push_back(getchar()); // push_back the char in the string
                reset_Line_And_Column(line, column+1);
                c_peek = cin.peek();
            }
            tmpstr.push_back(getchar()); // push_back second "
            reset_Line_And_Column(line, column+1);

            // make sure don't follow \n
            c_peek = cin.peek();
            if (is_enter(c_peek))
                getchar(); // ignore '\n'
            
        }
        else
            cerr << "\033[1;31munknown error in read_whole_string\033[0m" << endl;
    }

    void print_token(Token &t) {
        // for(auto &c : t.value) {
        //     if (c == ' ')
        //         cerr << " ";
        //     else
        //         cerr << c;
        // }
        cerr << "value: " << t.value << " ";
        cerr << " line: " << t.line << " ";
        cerr << "column: " << t.column << " ";
        cerr << "level: " << t.level << " ";
        cerr << "type: ";
        switch (t.type) {
            case INT: cerr << "INT"; break;
            case FLOAT: cerr << "FLOAT"; break;
            case STRING: cerr << "STRING"; break;
            case Left_PAREN: cerr << "Left_PAREN"; break;
            case Right_PAREN: cerr << "Right_PAREN"; break;
            case DOT: cerr << "DOT"; break;
            case NIL: cerr << "NIL"; break;
            case T: cerr << "T"; break;
            case QUOTE: cerr << "QUOTE"; break;
            case SYMBOL: cerr << "SYMBOL"; break;
            default: cerr << "UNKNOWN"; break;
        }
        cerr << " \n";
    }
    void print_vector(vector<Token> &v) {
        cerr << "\033[1;34menter print_vector\033[0m" << endl;
        for (int i = 0; i < v.size(); i++)
            print_token(v.at(i));
        cerr << "\033[1;34mend print_vector\033[0m" << endl;
    }

    void set_token_line_and_column(Token &tmptoken, int l, int c, string str) {
        tmptoken.value = str;
        tmptoken.type = check_token_type(tmptoken.value);
        tmptoken.line = l;
        tmptoken.column = c;
        if (tmptoken.value == "(")
            tmptoken.level = level - 1; // level 從0開始
        else
            tmptoken.level = level;
    }

    void reset_error_info() {
        reset_Line_And_Column(1, 1);
        leftParen = 0;
        rightParen = 0;
        level = 0;
        error_line = 0;
        error_column = 0;
        // ExpectedToken = "";
        // CurrentToken = "";
        // is_EOF = false;
    }

    void handle_error(errorType e, char c_peek) {
        Error error;
        string current = tokenBuffer.back().value;
        int error_line = tokenBuffer.back().line;
        int error_column = tokenBuffer.back().column;

        if (e != UNEXPECTED_EOF && !is_enter(c_peek)) {
            string trash;
            getline(cin, trash); // ignore the whole line
            cerr << "\033[1;33mthrow trash: " << trash << "\033[0m" << endl;
        }
        switch (e) {
            case UNEXPECTED_TOKEN:
                error = Error(e, "atom or '('", current, error_line, error_column);
                break;
            case UNEXPECTED_CLOSE_PAREN:
                error = Error(e, "atom or '('", current, error_line, error_column);
                break;
            case UNEXPECTED_END_PAREN:
                error = Error(e, "')'", current, error_line, error_column);
                break;
            case UNEXPECTED_STRING:
                error = Error(e, "unset yet", current, error_line, error_column);
                break;
            case UNEXPECTED_EOF:
                is_EOF = true;
                // cerr << "2. throw error UNEXPECTED_EOF" << endl;
                error = Error(e, "eof", "eof", 0, 0);
                break;
            case UNEXPECTED_EXIT:
                error = Error(e, "exit", "exit", 0, 0);
                break;
            default:
                break;
        }
        throw error;
    }
    
public:
    vector<Token> tokenBuffer; // 暫存read data，去掉" "、"\n"、"\t"、""、    ?"\r"
    LexicalAnalyzer() {
        reset_Line_And_Column(1, 1);
        level = 0;
        is_EOF = false;
        // ExpectedToken("");
        // CurrentToken("");
    }

    string Get_Str(char &c_peek, Token &tmptoken, errorType &error, bool end = false) {
        // int leftCount = 0, rightCount = 0; // count '(' and ')'
        string str = "";
        char c = '\0';
        // char c_peek = '\0';
        
        while (!end) {
            c_peek = cin.peek(); // peek the next char
            if (c_peek == EOF) {
                // cerr << "1. throw error UNEXPECTED_EOF" << endl;
                error = UNEXPECTED_EOF;
                set_token_line_and_column(tmptoken, 0, 0, "eof");
                return "eof";
            }
            // 如果目前str內有symbol，下個讀入的如果是特殊字元，則先回傳當前str作為token
            if (!str.empty() && is_special_symbol(c_peek)) {
                set_token_line_and_column(tmptoken, line, column-str.size(), str);
                // 若下個字元是換行符
                if (is_enter(c_peek)){
                    getchar(); // skip the char of '\n'
                    reset_Line_And_Column(line+1, 1);
                }
                return str;
            }
            
            // not UNEXPECTED_EOF
            // handle whitespace, tab
            else if (is_space(c_peek)) {
                getchar(); // skip the char of ' ', '\t'
                reset_Line_And_Column(line, column+1);

                set_token_line_and_column(tmptoken, line, column-str.size(), " ");

                end = true;
                continue;
            }
            // handle enter
            else if (is_enter(c_peek)) {
                getchar(); // skip the char of '\n'
                reset_Line_And_Column(line, column+1);
                set_token_line_and_column(tmptoken, line, column-str.size(), "\n");

                reset_Line_And_Column(line+1, 1);
                
                end = true;
                continue;
            }
            // handle the parentheses
            else if (is_PAREN(c_peek, end)) {
                str.push_back(getchar()); // push_back the '(' or ')'
                reset_Line_And_Column(line, column+1); // column++;
                set_token_line_and_column(tmptoken, line, column-str.size(), str);
                if (rightParen > leftParen) {
                    error = UNEXPECTED_CLOSE_PAREN;
                    // return str;
                }
                // return str;
            }
            // handle comment, Semicolon(;)
            // ! meet eof not finish
            else if (is_comment(c_peek)) {
                string comment = "";
                read_whole_string(comment, error, ';');
                cerr << "\033[1;32mcomment: " << comment << "\033[0m" << endl;

                if (error == UNEXPECTED_EOF) {
                    set_token_line_and_column(tmptoken, 0, 0, "eof");
                    return "eof";
                }
            
            }
            // handle double quote
            // ! meet eof not finish
            else if (c_peek == '\"') {
                read_whole_string(str, error, '\"');
                if (error == UNEXPECTED_EOF) {
                    cerr << "\033[1;31mUNEXPECTED_EOF  str: " << str << "\033[0m" << endl;
                    continue;
                }
                else if (error == UNEXPECTED_STRING) {
                    // error = UNEXPECTED_STRING;
                    set_token_line_and_column(tmptoken, line, column, str);
                    return str;
                    // continue;
                }
                // str.push_back(getchar()); // push_back the first "
                // column++;
                set_token_line_and_column(tmptoken, line, column-str.size(), str);
                end = true;
                continue;

            }
            // handle quote
            else if (c_peek == '\'') {
                str.push_back(getchar()); // push_back the first "
                // level++;
                reset_Line_And_Column(line, column+1); // column++;
                set_token_line_and_column(tmptoken, line, column-str.size(), str);
                return str;
            }
            // handle other string
            // ! meet eof not finish
            else{
                if (str == "(" || str == ")") 
                    return str;
                str.push_back(getchar());
                reset_Line_And_Column(line, column+1); // column++;
            }
           
        }

        if (str.empty())
            // cout << "\033[1;32m----------empty-----------\033[0m" << endl;
            return Get_Str(c_peek, tmptoken, error);
        
        else
            return str;
        
    }

    void Get_Token(bool &finish_input, errorType &error, bool return_when_end = false) {
        Token tmptoken;
        string tmpstr = "";
        vector<pair<int, bool>> dot_appear;
        SyntaxAnalyzer syntax;

        char c_peek = '\0';
        do {
            try {
                // int tmp_line = line;
                // int tmp_column = column;
                tmpstr = Get_Str(c_peek, tmptoken, error); // get the string from cin
                // if (tmpstr == "#f")
                //     tmptoken.value = "nil";
                
                // cerr << "tmpstr: " << tmptoken.value << endl;
                tokenBuffer.push_back(tmptoken);
                
                // cerr << "\033[1;31m1. error: " << error << "\033[0m" << endl;
                
                // no error, judge the syntax
                if (error == Error_None) {
                    syntax.check_syntax(tokenBuffer, error);
                }

                
                // if (tokenBuffer)
                // ! print test
                if (error != Error_None) {
                    // cerr << "\033[1;31m2. error: " << error << "\033[0m" << endl;
                    print_vector(tokenBuffer);
                    throw error;
                }

                if (tokenBuffer.size() >= 2) {
                    if (tokenBuffer.at(tokenBuffer.size()-2).type == Left_PAREN && tokenBuffer.at(tokenBuffer.size()-1).type == Right_PAREN) {
                        Token nil_token;
                        nil_token.type = NIL;
                        nil_token.value = "#f";
                        nil_token.line = tokenBuffer.at(tokenBuffer.size()-2).line;
                        nil_token.column = tokenBuffer.at(tokenBuffer.size()-2).column;
                        nil_token.level = tokenBuffer.at(tokenBuffer.size()-2).level;
                        
                        tokenBuffer.pop_back();
                        tokenBuffer.pop_back();
                        tokenBuffer.push_back(nil_token);
                    }
                }
                
            }
            catch (errorType error) {
                // ! when error occur, finish Get_Token(), go to main() print error message
                finish_input = true;
                
                handle_error(error, c_peek);
                // return; // ! return when error occur, go to main() print error message
            }
            
            if (tokenBuffer.size() == 1 && tokenBuffer.at(0).type == QUOTE)
                finish_input = false;
            else if ((level == 0 && tokenBuffer.size() > 0) || (leftParen != 0 && leftParen == rightParen))
                finish_input = true;
            
        }
        while(!finish_input);

        // ! print test
        print_vector(tokenBuffer);
    }

    bool Get_is_EOF(){
        return is_EOF;
    }

    void reset() {
        tokenBuffer.clear();
        reset_error_info();
    }

    ~LexicalAnalyzer() {
        reset();
    }
};

int main() {
    LexicalAnalyzer Lexical; //詞法分析器
    bool is_Syntax_legal = true;
    SyntaxAnalyzer Syntax; //語法分析器

    string expr, input;
    int line, column = 0;
    Node_Token *head = nullptr;
    Node_Token *tail = nullptr;

    // clear the cin buffer
    cin.clear();
    cin.ignore(1024,'\n');

    cout << "Welcome to OurScheme!" << endl;
    
    while (!Lexical.Get_is_EOF()) { // while (true)
        bool finish_input = false;
        try {
            cout << "\n> ";
            errorType E = Error_None;
            Lexical.Get_Token(finish_input, E);

            if (finish_input) {
                // cerr << "\033[1;32mfinish_input\033[0m" << endl;
                // bulid parser tree
                Syntax.build_tree(Lexical.tokenBuffer);

                // Syntax.pretty_print(Lexical.tokenBuffer);

                // reset lexical vector
                Lexical.reset();
            }

        } catch (Error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    // cerr << "\033[1;31m" << "UNEXPECTED_TOKEN" << "\033[0m" << endl;
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    // cerr << "\033[1;31m" << "UNEXPECTED_CLOSE_PAREN" << "\033[0m" << endl;
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_END_PAREN:
                    // cerr << "\033[1;31m" << "UNEXPECTED_END_PAREN" << "\033[0m" << endl;
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_STRING:
                    // cerr << "\033[1;31m" << "UNEXPECTED_STRING" << "\033[0m" << endl;
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EOF:
                    // cerr << "\033[1;31m" << "UNEXPECTED_EOF" << "\033[0m" << endl;
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EXIT:
                    Lexical.reset();
                    break;
                default:
                    break;
            }
        }
        
    }
        
    cout << "Thanks for using OurScheme!" << endl;
    return 0;
}

/*

# Step 1: 印出歡迎訊息

Print "Welcome to OurScheme!"

Print "\n"

Print "> "  # 提示使用者輸入



# Step 2: 進入 REPL 迴圈

Repeat:

    # 讀取使用者輸入的 S-Expression

    expr = ReadSExp()

    

    # 如果讀取到 EOF（檔案結尾），跳出迴圈

    If expr == END-OF-FILE:

        Print "ERROR (no more input) : END-OF-FILE encountered"

        Break

    

    # 若使用者輸入的是 '(exit)'，結束迴圈

    If expr == "(exit)":

        Break

    

    # 格式化並輸出 S-Expression

    PrintSExp(expr)

    

    # 顯示新的提示符號，等待下一次輸入

    Print "> "



# Step 3: 結束程式

Print "\n"

Print "Thanks for using OurScheme!"

*/