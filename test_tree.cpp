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
        // cerr << "\033[1;33mset_node value: " << token.value << "\033[0m" << endl;
        return tmp;
    }

    Node_Token* insert(Node_Token *parent, vector<Token> &tokenbuffer, int &index, bool create_node = false) {
        int size = tokenbuffer.size();
        Node_Token *node = nullptr;
        Token none_token = set_token(".", DOT, -1);
        // Token none_token = set_token("None", None, -1);
        Token nil_token = set_token("#f", NIL, -1);
        cerr << "\033[1;32mindex: " << index << "\033[0m" << endl;

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
        // cerr << "\033[1;32m" << "index++ " << "\033[0m" << endl;
        // !next_token
        Token next_token;
        if (index < size) next_token = tokenbuffer.at(index);
        else next_token = set_token("None", None, -1);
        cerr << "next_token: " << next_token.value << endl;
        // *********************************************************
        
        
        // current is s-exp or not
        if (is_s_exp(current_token)) {
            // cerr << "\033[1;35mis_s_exp\033[0m" << endl;
            
            if (current_token.type == Left_PAREN) {
                cerr << "\033[1;35m" << "** Left_PAREN **" << "\033[0m" << endl;
                // ! I think will not enter in here
                if (next_token.type == Right_PAREN)
                    cout << "\033[1;35m" << "** I think will not enter in here **" << "\033[0m" << endl;
                // !
                // (s-exp)
                else {
                    // 當前 node 內會將 "(" 存成 "."
                    current_token.value = ".";
                    current_token.type = DOT;

                    if (create_node) { // 新建空節點
                        cerr << "\033[1;35m" << "** create_node == T **" << "\033[0m" << endl;
                        node = set_node(none_token, nullptr, nullptr, parent); // 空節點

                        Node_Token *left_atom = set_node(current_token, nullptr, nullptr, node);
                        node->left = left_atom;
                        left_atom->left = insert(node, tokenbuffer, index);
                        cerr << "\033[1;32m" << "** in create_node == T **  \nback to ('s , start right_child" << "\033[0m" << endl;
                        cerr << "current_token.value: " << current_token.value << endl;
                        cerr << "current_token.type: " << current_token.type << endl;
                        cerr << "current_token.level: " << current_token.level << endl;
                        cerr << "current_token.line: " << current_token.line << endl;
                        cerr << "current_token.column: " << current_token.column << endl;

                        left_atom->right = insert(node, tokenbuffer, index, true);
                    }
                    else { // 當前 node 內會將 "(" 存成 "."
                        node = set_node(current_token, nullptr, nullptr, parent);
                        // node->token.value = ".";
                        // node->token.type = DOT;
                        node->left = insert(node, tokenbuffer, index);
                        cerr << "\033[1;32m" << "back to ('s , start right_child" << "\033[0m" << endl;
                        cerr << "current_token.value: " << current_token.value << endl;
                        cerr << "current_token.type: " << current_token.type << endl;
                        cerr << "current_token.level: " << current_token.level << endl;
                        cerr << "current_token.line: " << current_token.line << endl;
                        cerr << "current_token.column: " << current_token.column << endl;
                        // node->right = insert(node, tokenbuffer, index, true);
                    }
                    
                    cerr << "\033[1;32m" << "back to ('s , start right_child" << "\033[0m" << endl;
                    node->right = insert(node, tokenbuffer, index, true);
                }
                
            }
            else if (!create_node && current_token.type == QUOTE) {
                cerr << "\033[1;35m" << "** QUOTE **" << "\033[0m" << endl;
                node = set_node(none_token, nullptr, nullptr, parent);
                // *                   .  <-node
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
                    // 中文：如果上個 token 是 '(' or '.'，則不建立新節點，'.'可藉由 create_node 控制，因此判斷式不特別判斷'.'
                    // !create_node if is someone's right child
                    node = set_node(current_token, nullptr, nullptr, parent);
                    if (!create_node && next_token.type == Right_PAREN) {
                        index++;
                    }
                }
                else if (create_node && last_token.type != DOT) {
                    cerr << "\033[1;35m" << "** create_node == T **" << "\033[0m" << endl;
                    // 上一個 token 不是 '(' or '.'，則當前 token 為右子樹，需新建空節點
                    node = set_node(none_token, nullptr, nullptr, parent); // 空節點

                    if (current_token.type == QUOTE) {
                        cerr << "\033[1;32m" << "** start bulid side quote tree **" << "\033[0m" << endl;
                        int start = index;
                        int end = index;

                        cerr << "tokenbuffer.at(start).level: " << tokenbuffer.at(start).level << endl;
                        do { 
                            end++; 
                            cerr << "tokenbuffer.at(end).level: " << tokenbuffer.at(end).level << endl;
                        }
                        while(tokenbuffer.at(end).type != Right_PAREN || tokenbuffer.at(end).level!=tokenbuffer.at(start).level);
                        
                        
                        vector<Token> side_token(tokenbuffer.begin() + start-1, tokenbuffer.begin() + end);
                        for (auto &t : side_token) {
                            cerr << "side_token: " << t.value << endl;
                        }
                        start = 0 ;
                        node->left = insert(node, side_token, start);
                        if (tokenbuffer.at(end).type == Right_PAREN)
                            index = end+1; // skip ')'
                        else
                            index = end;
                        // cerr << "********* index: " << index << endl;
                        cerr << "\033[1;32m" << "** end bulid side quote tree **" << "\033[0m" << endl;
                    }
                    else {
                        Node_Token *left_atom = set_node(current_token, nullptr, nullptr, node);
                        node->left = left_atom;
                    }
                    
                    node->right = insert(node, tokenbuffer, index, true);

                }
                // if (last_token.type == DOT) {
                //     cerr << "\033[1;35m" << "** last_token.type == DOT **" << "\033[0m" << endl;
                //     cerr << "node->parent->token.value : " << node->parent->token.value << endl;
                //     node->parent->token = set_token(last_token.value, last_token.type, last_token.level, last_token.line, last_token.column);
                //     cerr << "node->parent->token.value : " << node->parent->token.value << endl;
                // }
                
            }
            
        }
        // Right_PAREN
        else if (current_token.type == Right_PAREN) {
            cerr << "\033[1;35m---------------is_Right_PAREN-------------\033[0m" << endl;
            node = set_node(nil_token, nullptr, nullptr, parent); // 為 leaf node，不需再往下 insert
            // node->parent->token.value = ".";
            // node->parent->token.type = DOT;
            // node->left = insert(node, tokenbuffer, index);
            // node->right = insert(node, tokenbuffer, index, true);
            
        }
        // DOT
        else if (current_token.type == DOT) {
            cerr << "\033[1;35mis_DOT\033[0m" << endl;
            // 回到當前 node 的 parent->right 去 insert nil 進去，並將其 left、right 指向 nullptr
            // cerr << "parent->token.value: " << parent->token.value << endl;
            // cerr << "parent->token.line: " << parent->token.line << endl;
            // cerr << "parent->token.column: " << parent->token.column << endl;

            // if (last_token.type != Right_PAREN)
            //     node = insert(parent, tokenbuffer, index, false);
            if (next_token.type != Left_PAREN)
                node = insert(parent, tokenbuffer, index, false);
            else {

            
                cerr << "\033[1;32m" << "** start bulid side dot tree **" << "\033[0m" << endl;
                int start = index;
                int end = index;

                // cerr << "tokenbuffer.at(start).level: " << tokenbuffer.at(start).level << endl;
                cerr << "tokenbuffer.at(start).value: " << tokenbuffer.at(start).value << endl;
                if (next_token.type == Left_PAREN) {
                    while (tokenbuffer.at(end).type != Right_PAREN || tokenbuffer.at(end).level!=next_token.level)
                        end++;
                }
                // else 
                
                
                vector<Token> side_token(tokenbuffer.begin() + start, tokenbuffer.begin() + end+1);
                for (auto &t : side_token) {
                    cerr << "side_token: " << t.value << endl;
                }
                start = 0 ;
                node = insert(node, side_token, start);
                if (tokenbuffer.at(end).type == Right_PAREN)
                    index = end+1; // skip ')'
                else
                    index = end;
                cerr << "\033[1;32m" << "** end bulid side dot tree **" << "\033[0m" << endl;
            }

        }
        return node;
        
    }
    void print_token(Token &t) {
        // if (t.value == "None")
        //     cerr << "\033[1;32m" << t.value << "\033[0m" << "\n";
        
        if (t.value == "#f")
            cout << "nil" << "\n";
        else
            cout << t.value << "\n";
    }

    void print_perfix_tree(Node_Token *node) {
        if (node == nullptr) return;
        
        print_token(node->token);
        cerr << "node->left" << endl;
        print_perfix_tree(node->left);
        cerr << "node->right" << endl;
        print_perfix_tree(node->right);
    }

    void print_infix_tree(Node_Token *node) {
        if (node == nullptr) return;
        
        // cerr << "node->left" << endl;
        print_infix_tree(node->left);
        print_token(node->token);
        // cerr << "node->right" << endl;
        print_infix_tree(node->right);
    }

    public:
    AST_Tree() : root(nullptr) {}

    void build_AST(vector<Token> tokenBuffer) {
        tokens = tokenBuffer;

        cerr << "\033[1;34menter build_AST\033[0m" << endl;
        int index = 0;

        if (!tokens.empty()) { // at least 3 tokens && tokenBuffer.size() > 2
            root = insert(root, tokens, index);
        }

            
        // cerr << "\033[1;34mend build_AST\033[0m" << endl;

        print();
    }

    void print() {
        cerr << "\033[1;34menter print_perfix_tree\033[0m" << endl;
        print_perfix_tree(root);
        cerr << "\033[1;34mend print_perfix_tree\033[0m" << endl;
        cout << "\n";
        cerr << "\033[1;34menter print_infix_tree\033[0m" << endl;
        print_infix_tree(root);
        cerr << "\033[1;34mend print_infix_tree\033[0m" << endl;
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

    bool check_exit() {
        if (root == nullptr) return false;
        else if (root->left == nullptr) return false;
        else if (root->right == nullptr) return false;
        else if (root->left->token.value == "exit" && root->right->token.type == NIL)
            return true;
        
        return false;
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
    // vector <pair<int, pair<Token*, bool>>> dot_appear; // first: current_level, second: appear or not
    vector <pair<Token*, bool>> dot_appear; // first: pointer to ( , second: appear or not

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

    void renew_dot_appear(TokenType t, Token* p) {
        // for (; currnt_level <= level; currnt_level++)
        //     dot_appear.push_back({currnt_level, false});

        if (t == DOT) {
            for (auto &dot : dot_appear) {
                if (dot.first == p) {
                    dot.second = true;
                    break;
                }
            }
        }
        else if (t == Left_PAREN)
            dot_appear.push_back({p, false});
    }

    bool check_dot_appear(Token* p) {
        int size = dot_appear.size();
        for (auto &dot : dot_appear) {
            if (dot.first == p) {
                return dot.second;
            }
        }
        return false;
    }

public:
    SyntaxAnalyzer() {
        currnt_level = 0;
        // dot_appear.push_back({0, false});
    }

    void build_tree(vector<Token> &v) {

        tree.build_AST(v);
        if (tree.check_exit())
            throw Error(UNEXPECTED_EXIT, "exit", "exit", 0, 0);

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
                // 一對 () 內只能有一個 DOT
                for (int i = index_prev ; i >= 0; i--) {
                    if (tokenBuffer.at(i).type == Left_PAREN) {
                        if (check_dot_appear(&tokenBuffer.at(i)) == true)
                            error = UNEXPECTED_END_PAREN;
                        break;
                    }
                }
                // if (check_dot_appear(tokenBuffer.at(index_curr).level) == true) 
                //     error = UNEXPECTED_END_PAREN;

            }
            // right Paren
            if (tokenBuffer.at(index_curr).type == Right_PAREN) {
                // Right_PAREN 前必須是完整 ATOM 或是 S-EXP
                // 避免 ') or .) 
                if (tokenBuffer.at(index_prev).type == QUOTE || tokenBuffer.at(index_prev).type == DOT) {
                    error = UNEXPECTED_TOKEN;
                }
            }
            // Left_PAREN
            if (tokenBuffer.at(index_curr).type == Left_PAREN) {
                renew_dot_appear(tokenBuffer.at(index_curr).type, &tokenBuffer.at(index_curr));
            }
            
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
        // tree.clear_tree(tree.get_root());

    }


};

class LexicalAnalyzer {
private:
    
    int line, column; // '(" ", "\t"， "\")保留
    int start_line, start_column;
    int leftParen = 0, rightParen = 0; // count '(' and ')'
    int level;
    bool is_EOF;
    int count_escape;
    // pair<bool, bool> dot_appear = {false, false}; // first: is_dot, second: enter the second atom

    int error_line, error_column;
    // string ExpectedToken, CurrentToken;

    void reset_Line_And_Column(int l, int c) {
        line = l;
        column = c;
        // cerr << "reset line: " << line << " column: " << column << endl;
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

    bool is_PAREN(char token) { // check whether the token is '(' or ')'
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
            }
            else if (str[i] == '.') {
                dot_count++;
                if (dot_count > 1)
                    return false;
                if (i == 0 || !isdigit(str[i - 1]))
                    tmp.insert(tmp.begin(), '0'); // Insert '0' before the dot if no digit before it
            }
            else if (!isdigit(str[i]) && str[i] != '.')
                return false;
            i++;
        }

        // Ensure the float has three decimal places
        int dot_pos = tmp.find('.');
        if (tmp.size()-1 == dot_pos) // 小數點在最後一位
            tmp = tmp + "000";

        if (sign == "-")
            tmp = sign + tmp;
        str = tmp;
        cerr << "END : float str: " << str << endl;
        return true;
    }

    TokenType check_token_type(string &str) {
        TokenType type;
        if (str == "#f" || str == "nil") // str == "NIL" || str == "nil"
            type = NIL;
        else if (str == "t" || str == "#t")
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
        count_escape = 0;

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
            else {
                getchar(); // ignore '\n'
                reset_Line_And_Column(line+1, 1);
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
                    error = UNEXPECTED_STRING;
                    return;
                }
                else if (c_peek == '\\') {
                    count_escape++;
                    string tmp = "";
                    tmp.push_back(getchar()); // push_back '\'
                    reset_Line_And_Column(line, column+1); // column++;
                    c_peek = cin.peek();
                    if (c_peek == 'n' || c_peek == 't' || c_peek == 'r' || c_peek == '0' || c_peek == '\\' || c_peek == '\"' ) {
                        tmp.push_back(getchar()); // push_back the char behind '\'
                        if (tmp == "\\n")
                            tmpstr.push_back('\n');
                        else if (tmp == "\\t")
                            tmpstr.push_back('\t');
                        else if (tmp == "\\r")
                            tmpstr.push_back('\r');
                        else if (tmp == "\\0")
                            tmpstr.push_back('\0');
                        else if (tmp == "\\\\")
                            tmpstr.push_back('\\');
                        else if (tmp == "\\\"")
                            tmpstr.push_back('\"');
                        reset_Line_And_Column(line, column+1); // column++;
                    }
                    else if (c_peek == EOF) {
                        error = UNEXPECTED_EOF;
                        return;
                    }
                    else if (is_enter(c_peek)) {
                        error = UNEXPECTED_STRING;
                        return;
                    }
                    else
                        tmpstr.push_back('\\');
                    
                }
                else {
                    tmpstr.push_back(getchar()); // push_back the char in the string
                    reset_Line_And_Column(line, column+1);
                }
                
                c_peek = cin.peek();
            }
            tmpstr.push_back(getchar()); // push_back second "
            reset_Line_And_Column(line, column+1);

            
            c_peek = cin.peek();
            // if (is_enter(c_peek)) // make sure don't follow \n
            //     getchar(); // ignore '\n'
            // else if (c_peek == EOF) {
            //     error = UNEXPECTED_EOF;
            //     return;
            // }

            
        }
        else
            cerr << "\033[1;31munknown error in read_whole_string\033[0m" << endl;
    }


    void print_token(Token &t) {
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
        reset_Line_And_Column(start_line, start_column);
        leftParen = 0;
        rightParen = 0;
        level = 0;
        error_line = 0;
        error_column = 0;
        start_line = 1;
        start_column = 1;
        count_escape = 0;

        // ExpectedToken = "";
        // CurrentToken = "";
        // is_EOF = false;
    }

    void handle_error(errorType e, char c_peek) {
        Error error;
        string current = tokenBuffer.back().value;
        int error_line = tokenBuffer.back().line;
        int error_column = tokenBuffer.back().column;
        start_column = 1;
        start_line = 1;

        if (e != UNEXPECTED_EOF) { //  && !is_enter(c_peek)
            string trash;
            getline(cin, trash);
            for (char c : trash) {
                if (c == EOF) {
                    ungetc(c, stdin); // push EOF back to the buffer
                    break;
                }
            }

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
                error = Error(e, "unset yet", current, error_line, error_column-1);
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
    LexicalAnalyzer(): start_line(1), start_column(1), leftParen(0), rightParen(0), level(0), is_EOF(false), count_escape(0) {
        reset_Line_And_Column(1, 1);
        // level = 0;
        // is_EOF = false;
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
                // if (is_enter(c_peek)){
                //     getchar(); // skip the char of '\n'
                //     reset_Line_And_Column(line+1, 1);
                // }

                return str;
            }
            
            // not UNEXPECTED_EOF
            // handle whitespace, tab
            else if (is_space(c_peek)) {
                getchar(); // skip the char of ' ', '\t'
                reset_Line_And_Column(line, column+1);

                // set_token_line_and_column(tmptoken, line, column-str.size(), " ");

                end = true;
                continue;
            }
            // handle enter
            else if (is_enter(c_peek)) {
                getchar(); // skip the char of '\n'
                // reset_Line_And_Column(line, column+1);
                // set_token_line_and_column(tmptoken, line, column-str.size(), "\n");

                reset_Line_And_Column(line+1, 1);
                
                end = true;
                continue;
            }
            // handle the parentheses
            else if (is_PAREN(c_peek)) {
                str.push_back(getchar()); // push_back the '(' or ')'
                reset_Line_And_Column(line, column+1); // column++;
                set_token_line_and_column(tmptoken, line, column-str.size(), str);
                if (rightParen > leftParen) {
                    error = UNEXPECTED_CLOSE_PAREN;
                    // return str;
                }
                return str;
            }
            // handle comment, Semicolon(;)
            // ! meet eof not finish
            else if (is_comment(c_peek)) {
                string comment = "";
                read_whole_string(comment, error, ';');
                cerr << "\033[1;32mcomment: " << comment << "\033[0m" << endl;

                if (error == UNEXPECTED_EOF) {
                    set_token_line_and_column(tmptoken, 0, 0, "eof");
                    return "is_comment_eof";
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
                set_token_line_and_column(tmptoken, line, column-str.size()-count_escape, str);
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
                // if (str == "(" || str == ")") 
                //     return str;
                str.push_back(getchar());
                reset_Line_And_Column(line, column+1); // column++;
            }
           
        }

        if (str.empty())
            // cout << "\033[1;32m----------empty-----------\033[0m" << endl;
            // 只輸入空白字元或換行符時，不會回傳空字串
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
                if (tmpstr == "t")
                    tmptoken.value = "#t";
                
                tokenBuffer.push_back(tmptoken);
                
                
                // no error, judge the syntax
                if (error == Error_None)
                    syntax.check_syntax(tokenBuffer, error);
                
                if (error != Error_None) {
                    // ! print test
                    print_vector(tokenBuffer);
                    throw error;
                }

                // turn () to nil
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
  
        if (finish_input) {
            cerr << "\033[1;32mfinish_input\033[0m" << endl;
            print_vector(tokenBuffer);

            while (true) {
                c_peek = cin.peek();

                if (c_peek == EOF) {
                    // is_EOF = true;
                    cerr << "after finish_input, throw error UNEXPECTED_EOF" << endl;
                    // c_peek = getchar(); // skip the EOF
                    // ungetc(c_peek, stdin); // push EOF back to the buffer
                    return; // 交給下次呼叫 get_token() 處理此 EOF，因為要先建樹
                }
                else if (is_enter(c_peek)) {
                    cerr << "after finish_input, is_enter" << endl;
                    getchar(); // skip the char of '\n'}
                    return;
                }
                else if (is_space(c_peek)) {
                    cerr << "after finish_input, is_space" << endl;
                    start_column++;
                    getchar(); // skip the char of ' '
                    continue;
                }
                else if (is_comment(c_peek)) {
                    cerr << "after finish_input, is_comment" << endl;
                    string trash;
                    getline(cin, trash);
                    for (char c : trash) {
                        if (c == EOF)
                            ungetc(c, stdin); // push EOF back to the buffer
                    }
        
                    cerr << "\033[1;33mthrow trash in get_token: " << trash << "\033[0m" << endl;
                    return;
                }
                else {
                    cerr << "\033[1;31m"<< "c_peek: " << c_peek << "\033[0m" << endl;
                    return;
                }

            }
        }
        else {
            cerr << "\033[1;31mmeet unexpected error\033[0m" << endl;
        }
        // ! print test
        

        // reset_error_info();
        
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
    bool is_exit = false;

    string expr, input;
    int line, column = 0;
    Node_Token *head = nullptr;
    Node_Token *tail = nullptr;

    // clear the cin buffer
    cin.clear();
    cin.ignore(1024,'\n');

    cout << "Welcome to OurScheme!" << endl;
    
    while (!Lexical.Get_is_EOF() && !is_exit) { // while (true)
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
                // Lexical.tokenBuffer.clear();
                Lexical.reset();
            }

        } catch (Error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    cerr << "\033[1;31m" << "UNEXPECTED_TOKEN" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    cerr << "\033[1;31m" << "UNEXPECTED_CLOSE_PAREN" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_END_PAREN:
                    cerr << "\033[1;31m" << "UNEXPECTED_END_PAREN" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_STRING:
                    cerr << "\033[1;31m" << "UNEXPECTED_STRING" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EOF:
                    cerr << "\033[1;31m" << "UNEXPECTED_EOF" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EXIT:
                    cerr << "\033[1;31m" << "UNEXPECTED_EXIT" << "\033[0m" << endl;
                    cout << endl;
                    Lexical.reset();
                    is_exit = true;
                    break;
                default:
                    break;
            }
        }
        
    }
        
    cout << "Thanks for using OurScheme!" << endl;
    return 0;
}
