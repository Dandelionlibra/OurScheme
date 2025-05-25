#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <sstream>
using namespace std;

enum TokenType {
    INT = 1, // accept +&-, e.g., '123', '+123', '-123'
    FLOAT = 2, // '123.567', '123.', '.567', '+123.4', '-.123'
    STRING = 2222, // need use "", cannot separate by line

    Left_PAREN = 3, // ( begin of list
    Right_PAREN = 9, // ) end of list

    DOT = 1000, // . dot and dot pair
    NIL = -99999999, // ();false; null
    T = 4, // true
    QUOTE = 1111, // '
    SYMBOL = 5, // 以 identifier 表示的資料，變數名稱, a type of atom
           // a consecutive sequence of printable characters that are
           // not numbers, strings, #t or nil, and do not contain 
           // '(', ')', single-quote, double-quote, semi-colon and 
           // white-spaces ; 
           // Symbols are case-sensitive 
           // (i.e., uppercase and lowercase are different);
    USERERROR= 6, // user error
    None = 0

};
enum errorType {
    // ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
    UNEXPECTED_TOKEN = 1, // atom or '(' expected

    // ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
    UNEXPECTED_CLOSE_PAREN = 2, // unexpected close parenthesis error, error (1 2 . 3 4) , correct is (1 2 . (3 4))
    UNEXPECTED_END_PAREN = 4,
    // ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
    UNEXPECTED_STRING = 3, // unexpected string error ("hello)

    //ERROR (no more input) : END-OF-FILE encountered
    UNEXPECTED_EOF = 123, // END-OF-FILE encountered

    UNEXPECTED_EXIT = 5, // (exit)

    incorrect_number_of_arguments = 48,
    incorrect_argument_type = 49,
    undefined_function = 555,
    unbound_symbol = 666,
    unbound_parameter = 667,
    unbound_condition = 678,
    no_return_value = 668,
    no_return_value_inernal = 6688,
    unbound_test_condition = 669,
    division_by_zero = 0,
    non_list = 777,

    defined = 888,
    error_level_define = 889,
    error_define_format = 890,

    cleaned = 800,
    error_level_cleaned = 801,

    error_level_exit = 999,
    // procedure = 1000,
    Error_None = 99999
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    int level;
    bool is_function;

    Token() : type(SYMBOL), value(""), line(0), column(0), level(0), is_function(false) {}
};

struct Node_Token {
    Token token; // token info, value, type

    Node_Token *parent;
    Node_Token *left;
    Node_Token *right;
    Node_Token() : parent(nullptr), left(nullptr), right(nullptr) {}
};

set<string> bulid_in_func = { // only bulid-in function
    "define", "cons", "lambda", "list",
    "let", "set!",
    "car", "cdr",
    "atom?", "pair?", "list?", "null?", "integer?", "real?", "number?", "string?", "boolean", "symbol?",
    "+", "-", "*", "/",
    "not", "and", "or",
    "eqv?", "equal?",
    "=", "<", ">", "<=", ">=",
    "string-append", "string>?", "string<?", "string=?",
    "begin", "if", "cond",
    "clean-environment",
    "'", "quote", "exit",
    "verbose?", "verbose",

    "create-error-object", "error-object?", 
    "read", "write", "display-string", 
    "newline", "symbol->string", "number->string",
    "eval", "set!"
};
// set <string> func; // all function name, unclude self-defined function
unordered_map<string, Node_Token*> defined_table; // store the variable name, value and defined func
set <Node_Token*> pointer_gather; // store the variable name and value
bool verbose_mode = true; // verbose mode, default is true

// error message 
class Error {

public:
    errorType type;
    string message;
    string expected;
    string current;    
    int line;
    int column;
    Node_Token* root;

    Error() : type(Error_None), message(""), line(0), column(0), expected(""), current("") {}
    Error(errorType t, string e_token, string c_token, int l, int c, Node_Token* r = nullptr) {
        type = t;
        expected = e_token;
        current = c_token;
        line = l;
        column = c;
        root = r;

        if (e_token == "#f") e_token = "nil";

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
        else if (t == incorrect_number_of_arguments)
            message = "ERROR (incorrect number of arguments) : " + e_token;
        else if (t == incorrect_argument_type)
            message = "ERROR (" + e_token + " with incorrect argument type) : "; // Node_Token* r
        else if (t == undefined_function)
            message = "ERROR (attempt to apply non-function) : "; // Node_Token* r
        else if (t == unbound_symbol) 
            message = "ERROR (unbound symbol) : " + e_token;
        else if (t == unbound_parameter)
            message = "ERROR (unbound parameter) : "; // Node_Token* r
        else if (t == no_return_value || t == no_return_value_inernal)
            message = "ERROR (no return value) : "; // Node_Token* r
        else if (t == unbound_test_condition)
            message = "ERROR (unbound test condition) : "; // Node_Token* r
        else if (t == unbound_condition)
            message = "ERROR (unbound condition) : "; // Node_Token* r
        else if (t == division_by_zero)
            message = "ERROR (division by zero) : " + e_token;
        else if (t == non_list)
            message = "ERROR (non-list) : "; // Node_Token* r
        else if (t == defined)
            message = e_token + " defined";
        else if (t == error_level_define || t == error_level_cleaned || t == error_level_exit)
            message = "ERROR (level of " + c_token + ")";
        else if (t == error_define_format)
            message = "ERROR (" + e_token + " format) : "; // Node_Token* r
        else if (t == cleaned)
            message = "environment cleaned";
        
    }
    Node_Token * get_sub_error_tree() {
        return root;
    }
};

void clear_pointer_gather() {
    for (auto it = pointer_gather.begin(); it != pointer_gather.end(); ) {
        delete *it;
        it = pointer_gather.erase(it);
    }
}


// in order to parse the input, build a parser tree
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
        pointer_gather.insert(tmp);
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
        // cerr << "\033[1;32mindex: " << index << "\033[0m" << endl;

        if (index > size) {
            // cerr << "\033[1;31m* 2. touch bottom *\033[0m" << endl;
            return nullptr;
        }
        else if (index == size) {
            // index++;
            // cerr << "\033[1;31m* 1. touch bottom *\033[0m" << endl;
            node = set_node(nil_token, nullptr, nullptr, parent);
            return node;
        }

        // **************************** setting token ****************************
        // !last_token
        Token last_token;
        if (index > 0) last_token = tokenbuffer.at(index - 1);
        else  last_token = set_token("None", None, -1);
        // cerr << "last_token: " << last_token.value << endl;
        // !current_token
        Token current_token = tokenbuffer.at(index);
        // cerr << "current_token: " << current_token.value << endl;
        index++;
        // cerr << "\033[1;32m" << "index++ " << "\033[0m" << endl;
        // !next_token
        Token next_token;
        if (index < size) next_token = tokenbuffer.at(index);
        else next_token = set_token("None", None, -1);
        // cerr << "next_token: " << next_token.value << endl;
        // *********************************************************
        
        if (current_token.type == Left_PAREN) {
            // cerr << "\033[1;35m" << "** Left_PAREN  **" << "\033[0m" << endl;
            current_token = set_token(".", DOT, current_token.level, current_token.line, current_token.column);
            if (!create_node) {
                // cerr << "\033[1;35m" << "** create_node == false  **" << "\033[0m" << endl;
                
                node = set_node(current_token, nullptr, nullptr, parent);
        
                node->left = insert(node, tokenbuffer, index);
                // cerr << "\033[1;32m" << "1. back to ('s , start right_child" << "\033[0m" << endl;
                node->right = insert(node, tokenbuffer, index, true);
            }
            else {
                // cerr << "\033[1;35m" << "** create_node == true  **" << "\033[0m" << endl;
                node = set_node(none_token, nullptr, nullptr, parent); // 空節點
                //index--; // back to current ( in next step
                //// node->left = insert(node, token, index)
                // 
                Node_Token *left_paren = set_node(current_token, nullptr, nullptr, node);
                node->left = left_paren;
                left_paren->left = insert(node, tokenbuffer, index);
                // cerr << "\033[1;32m" << "2. back to ('s , start right_child" << "\033[0m" << endl;
                left_paren->right = insert(node, tokenbuffer, index, true);
                node->right = insert(node, tokenbuffer, index, true);
            }
            
        }
        else if (current_token.type == QUOTE) {
            // cerr << "\033[1;35m" << "** QUOTE **" << "\033[0m" << endl;
            
            
                // *                   .  <-node
                // *                 /  \
                // *left_quote->  quote *none  <-right_quote (save paren)
                // *             /    \
                // *          null   null
                // ?
                node = set_node(none_token, nullptr, nullptr, parent);
                Node_Token *left_quote = new Node_Token();
                pointer_gather.insert(left_quote);
                // current_token.value = "quote";
                left_quote = set_node(current_token, nullptr, nullptr, node);
                node->left = left_quote;

                Node_Token *right_quote = new Node_Token();
                pointer_gather.insert(right_quote);
                right_quote = set_node(none_token, nullptr, nullptr, node);
                node->right = right_quote;
                // ?

                // *        none  <-right_quote
                // *       /   \
                // *    token  nil
                // *    /   \    
                cerr << "\033[1;32m" << "** start bulid QUOTE side PAREN tree **" << "\033[0m" << endl;
                right_quote->left=insert(right_quote, tokenbuffer, index);
                right_quote->right=set_node(nil_token, nullptr, nullptr, right_quote);

                cerr << "\033[1;32m" << "** end bulid QUOTE side PAREN tree **" << "\033[0m" << endl;
            
            
            if (!create_node) 
                cerr << "\033[1;33m" << "**out create_node == false  **" << "\033[0m" << endl;
            
            else {
                // cerr << "\033[1;33m" << "**out create_node == true  **" << "\033[0m" << endl;
                Node_Token *empty = set_node(none_token, node, nullptr, parent);
                node->parent = empty;
                empty->right = insert(empty, tokenbuffer, index, true);
                return empty;
            }

        }
        else if (current_token.type == DOT) {
            // cerr << "\033[1;35m" << "** DOT  **" << "\033[0m" << endl;
            parent->token = current_token;
            // *
            if (next_token.type != Left_PAREN) {
                // cerr << "\033[1;35m" << "** next_token != Left_PAREN  **" << "\033[0m" << endl;
                node = set_node(next_token, nullptr, nullptr, parent);
                // insert(parent, tokenbuffer, index, true); // parent's right child
                // * (2 . 3)
                // *      ↑ next_token
                index++; // ignore next_token
                index++; // ignore ')'
            }
            else {
                // cerr << "\033[1;32m" << "** start bulid side quote tree **" << "\033[0m" << endl;
                int start = index; // (
                int end = index;

                while (tokenbuffer.at(end).type != Right_PAREN || tokenbuffer.at(end).level!=next_token.level)
                    end++;
                vector<Token> side_token(tokenbuffer.begin() + start, tokenbuffer.begin() + end+1);
                // for (auto &t : side_token) {
                //     cerr << "\033[1;33m" << "side_token: " << t.value << "\033[0m" << endl;
                // }
                    
                start = 0 ;
                node = insert(parent, side_token, start);
                index = index+start+1; // +1

                // cerr << "\033[1;32m" << "** end bulid side dot tree **" << "\033[0m" << endl;

            }//(1 . (2 . (3 . 4)))

        }
        else if (current_token.type == Right_PAREN) {
            // cerr << "\033[1;35m---------------is_Right_PAREN-------------\033[0m" << endl;
            node = set_node(nil_token, nullptr, nullptr, parent); // 為 leaf node，不需再往下 insert
            
        }
        // 5
        else {
            // cerr << "\033[1;35m" << "** ATOM := SYMBOL | INT | FLOAT | STRING | NIL | T  **" << "\033[0m" << endl;

            if (!create_node) {
                // cerr << "\033[1;35m" << "** create_node == false  **" << "\033[0m" << endl;

                node = set_node(current_token, nullptr, nullptr, parent);
            }
            else {
                // cerr << "\033[1;35m" << "** create_node == true  **" << "\033[0m" << endl;
                node = set_node(none_token, nullptr, nullptr, parent); // 空節點
                Node_Token *left_atom = set_node(current_token, nullptr, nullptr, node);
                node->left = left_atom;
                node->right = insert(node, tokenbuffer, index, true);
                
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
    AST_Tree(Node_Token* r) : root(r) {}

    void build_AST(vector<Token> tokenBuffer) {
        root = nullptr;
        tokens = tokenBuffer;
        // cerr << "\033[1;34menter build_AST\033[0m" << endl;
        int index = 0;

        if (!tokens.empty())
            root = insert(root, tokens, index);
        
        // define_trees.push_back(root);
        // cerr << "\033[1;34mend build_AST\033[0m" << endl;
        //print();
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
        // clear_tree(root);
        tokens.clear();
    }
};


class SyntaxAnalyzer {
private:
    AST_Tree tree;
    Node_Token *root;
    
    // int currnt_level = 0;
    
    vector <pair<Token, bool>> dot_appear; // first: pointer to ( , second: (dot appear or not, following data appear or not)

    
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

    bool compare_token(Token &t1, Token &t2) {
        if (t1.value == t2.value && t1.type == t2.type && t1.level == t2.level && t1.line == t2.line && t1.column == t2.column)
            return true;
        return false;
    }
    void renew_dot_appear(TokenType t, Token p) {
        if (t == DOT) {
            for (int i = dot_appear.size()-1; i >= 0; i--) {
                if (compare_token(dot_appear.at(i).first, p)) {
                    dot_appear.at(i).second = true;
                    break;
                }
            }
        }
        else if (t == Left_PAREN)
            dot_appear.push_back({p, false});
    }

    bool check_dot_appear(Token p) {
        int size = dot_appear.size();
        for (auto &dot : dot_appear) {
            if (compare_token(dot.first, p)) {
                // cerr << "dot.first.value: " << dot.first.value << endl;
                return dot.second;
            }
        }
        return false;
    }
    
    void print_space(int n) {
        for (int i = 0; i < 2*n; i++)
            cout << " ";
    }

    void pretty_print(Node_Token *node, int &countquote) {
        if (node == nullptr) {
            return;
        }

        // cerr << "\033[1;32m" << "countquote: " << countquote << "\033[0m" << endl;
        // **************************** setting token ****************************
        Token none_token;
        none_token.type = None;
        // !parent_token
        Token parent_token;
        if (node->parent!=nullptr) parent_token = node->parent->token;
        else  parent_token = none_token;
        // cerr << "parent_token: " << parent_token.value << endl;
        // !current_token
        Token current_token = node->token;
        // cerr << "current_token: " << current_token.value << endl;
        // index++;
        // cerr << "\033[1;32m" << "index++ " << "\033[0m" << endl;
        // !left_token
        Token left_token;
        if (node->left!=nullptr) left_token = node->left->token;
        else left_token = none_token;
        // cerr << "left_token: " << left_token.value << endl;
        // !right_token
        Token right_token;
        if (node->right!=nullptr) right_token = node->right->token;
        else right_token = none_token;
        // cerr << "right_token: " << right_token.value << endl;
        // *********************************************************

        if (current_token.type == DOT) {
            // cerr << "\033[1;35m" << "** outer DOT **" << "\033[0m" << endl;
            // if ((node->parent!=nullptr && node != node->parent->right)) {
                // print_space(countquote);
            // }
            
            cout << "( ";
            countquote++;
            // cerr << "\033[1;32m" << "countquote++: " << countquote << "\033[0m" << endl;

            if (left_token.type == QUOTE) {
                // cerr << "\033[1;35m" << "** QUOTE **" << "\033[0m" << endl;
                // print_space(countquote);
                cout << "quote";// << endl;//
            }
            else if (left_token.type == DOT) {
                // cerr << "\033[1;35m" << "** inner DOT **" << "\033[0m" << endl;
                
                pretty_print(node->left, countquote);
                
            }
            else if (left_token.type == NIL) { // (root == node && current_token.type == NIL) ||
                // cerr << "\033[1;35m" << "** NIL **" << "\033[0m" << endl;
                // print_space(countquote);
                cout << "nil";// << endl;//

            }
            else { // ATOM
                // cerr << "\033[1;35m" << "** 1.ATOM **" << "\033[0m" << endl;
                // deal float
                if (left_token.type == FLOAT)
                    cout << process_float(left_token);// << endl;
                else {
                    // print_space(countquote);
                    // if (func.find(left_token.value) != func.end())
                    if (left_token.is_function)
                        cout << "#<procedure " + left_token.value + ">";// << endl;
                    else
                        cout << left_token.value;// << endl;
                }
                
            }
            Node_Token *tmp = node->right;
            // cerr << "** begin while **" <<endl;
            while (tmp != nullptr) {
                if (tmp->token.type == DOT && tmp->left->token.type == DOT) { //  && tmp->right->token.type == DOT
                    // cerr << "\033[1;35m" << "** while DOT 1 **" << "\033[0m" << endl;
                    cout << "\n";
                    print_space(countquote);
                    pretty_print(tmp->left, countquote);
                }
                else if (tmp->token.type == DOT) { // left_token.type != DOT
                    // cerr << "\033[1;35m" << "** while DOT 2 **" << "\033[0m" << endl;
                    // if (tmp->left->token.type != NIL) {
                        // deal float
                        cout << "\n";
                        print_space(countquote);

                        if (tmp->left->token.type == FLOAT)
                            cout << process_float(tmp->left->token);// << endl;
                        else if (tmp->left->token.type == NIL)
                            cout << "nil";// << endl;
                        else{
                            // if (func.find(tmp->left->token.value) != func.end())
                            if (tmp->left->token.is_function)
                                cout << "#<procedure " + tmp->left->token.value + ">";// << endl;
                            else
                                cout << tmp->left->token.value;// << endl;
                        }
                        
                    // }
                    // else if (tmp->left->token.type == NIL) {
                    //     cerr << "\033[1;35m" << "** while DOT 3 **" << "\033[0m" << endl;
                    //     print_space(countquote);
                    //     cout << "nil" << endl;
                    // }
                }
                else if (tmp->token.type == NIL) {
                    // cerr << "\033[1;35m" << "** while NIL **" << "\033[0m" << endl;
                    pretty_print(tmp, countquote);
                }
                else { // ATOM
                    // cerr << "\033[1;35m" << "** while atom **" << "\033[0m" << endl;
                    // pretty_print(tmp->left, countquote);
                    cout << "\n";
                    print_space(countquote);
                    cout << "." << endl;
                    
                    pretty_print(tmp, countquote);
                    // cout << tmp->left << endl;
                    // cout << ")" << endl;
                    
                    // break;
                    
                }
                tmp = tmp->right;
                // cerr << "tmp->right: " << tmp->right->token.value << endl;

            }
            countquote--;
            cout << "\n";
            print_space(countquote);
            cout << ")"; //<< endl;
            
        }
        else { //ATOM, leaf node
            // cerr << "\033[1;35m" << "** 2.ATOM **" << "\033[0m" << endl;
            if (current_token.type != NIL) {
                // deal float
                print_space(countquote);

                if (current_token.type == FLOAT)
                    cout << process_float(current_token);// << endl;
                else {
                    // cerr << "\033[1;35m" << "** 3.ATOM **" << "\033[0m" << endl;
                    // if (func.find(current_token.value) != func.end())
                    if (current_token.is_function)
                        cout << "#<procedure " + current_token.value + ">";// << endl;
                    else
                        cout << current_token.value;// << endl;
                }
            }


        }


    }


public:
    SyntaxAnalyzer() {
        // currnt_level = 0;
    }
    
    string process_float(Token &token) {
        string str = token.value;
        string sign = "";

        if (token.value.at(0) == '+' || token.value.at(0) == '-') {
            sign = token.value.at(0); // save the sign
            str = token.value.substr(1); // Remove the sign from the string
        }


        // 四捨五入到小數點後第三位
        // cerr << "str: " << str << endl;
        double float_value = stod(str);
        // cerr << "float_value: " << float_value << endl;
        str = to_string(round(float_value * 1000.0) / 1000.0);
        // cerr << "str: " << str << endl;

        // 小數點後大於三位數, 只取到小數點後第三位
        int dot_pos = str.find('.');
        if (str.size() - dot_pos > 4)
            str = str.substr(0, dot_pos + 4);

        if (sign == "-")
            str = sign + str;

        // cerr << "str: " << str << endl;
        return str;
    }
    
    void set_root() {
        root = tree.get_root();
    }
    Node_Token *get_root() {
        return root;
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
                // 當 dot後讀到 ATOM 或跟 dot level 一樣的 ')'時，have_data=true，其後不能再出現
                // 避免 ('. or (. 
                if (tokenBuffer.at(index_prev).type == QUOTE || tokenBuffer.at(index_prev).type == Left_PAREN)
                    error = UNEXPECTED_TOKEN;
                // 一對 () 內只能有一個 DOT
                // 找比自己(dot or atom)當前level低1的(
                for (int i = index_prev ; i >= 0; i--) {
                    // cerr << "tokenBuffer.at(i).value: " << tokenBuffer.at(i).value << endl;
                    if (tokenBuffer.at(i).type == Left_PAREN && tokenBuffer.at(i).level == tokenBuffer.at(index_curr).level - 1) {
                        if (check_dot_appear(tokenBuffer.at(i))) {
                            // cerr << "\033[1;31mcheck_dot_appear == true\033[0m" << endl;
                            // cerr << "check_dot_appear: " << tokenBuffer.at(i).value << endl;
                            error = UNEXPECTED_END_PAREN;
                        }
                        else //if(tokenBuffer.at(index_curr).type == DOT) {
                            // cerr << "\033[1;31mcheck_dot_appear == false\033[0m" << endl;
                            renew_dot_appear(DOT, tokenBuffer.at(i));
                        //}
                            
                        break;
                    }
                }

            }
            // right Paren
            else if (tokenBuffer.at(index_curr).type == Right_PAREN) {
                // Right_PAREN 前必須是完整 ATOM 或是 S-EXP
                // 避免 ') or .) 
                if (tokenBuffer.at(index_prev).type == QUOTE || tokenBuffer.at(index_prev).type == DOT)
                    error = UNEXPECTED_TOKEN;
                
                // 一對 () 內只能有一個 DOT
                // 在與當前 ')' 前，相同 level 的'('間，觀察 level+1 的元素中 DOT 後出現第一個 ATOM、) 後的元素是否有非')'的元素
                // !ERROR (unexpected token) : ')' expected
            }
            else { // ATOM
                int c = 0;
                for (int i = token_size-1 ; i >= 0 ; i--) {
                    // cerr << "\033[1;33m1. tokenBuffer.at(i).value: " << tokenBuffer.at(i).value << "\033[0m" << endl;
                    if (tokenBuffer.at(i).type == Left_PAREN && tokenBuffer.at(i).level == tokenBuffer.at(index_curr).level-1)
                        break;
                    // 檢查與自己level相同的元素即可，但不含'\''、')'
                    else if (tokenBuffer.at(i).level == tokenBuffer.at(index_curr).level) {
                        if (tokenBuffer.at(i).type == Right_PAREN || tokenBuffer.at(i).type == QUOTE)
                            continue;
                        
                        if (tokenBuffer.at(i).type == DOT) {
                            if (c>1)
                                error = UNEXPECTED_END_PAREN;
                            break;
                        }
                        else
                            c++;
                        
                    }
                }
            }

            renew_dot_appear(tokenBuffer.at(index_curr).type, tokenBuffer.at(index_curr));
        }

        // cerr << "\033[1;34mend check_syntax\033[0m" << endl;
    }

    void build_tree(vector<Token> &v, bool check = true) {
        tree.build_AST(v);

        if (check && tree.check_exit())
            throw Error(UNEXPECTED_EXIT, "exit", "exit", 0, 0);
    }
    
    void print() {
        // root = tree.get_root();
        if (root->token.type == NIL) {
            cout << "nil" << endl;
            return;
        }
        int countquote = 0;

        // cerr << "\033[1;34menter pretty_print\033[0m" << endl;
        pretty_print(root, countquote);
        // cerr << "\033[1;34mend pretty_print\033[0m" << endl;
    }
    void print(Node_Token * r) {
        // root = tree.get_root();
        if (r == nullptr) return;
        else if (r->token.type == NIL) {
            cout << "nil";// << endl;
            return;
        }
        int countquote = 0;

        // cerr << "\033[1;34menter error pretty_print\033[0m" << endl;
        pretty_print(r, countquote);
        // cerr << "\033[1;34mend error pretty_print\033[0m" << endl;
    }

    ~SyntaxAnalyzer() {
        // tree.clear_tree(tree.get_root());
        root = nullptr;
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
        // cerr << "END : float str: " << str << endl;
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

    string Get_Str(char &c_peek, Token &tmptoken, errorType &error, bool &finish_input, bool end = false) {
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
            // if ()

            // ungetc(c, stdin); // push EOF back to the buffer




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
                // cerr << "\033[1;32mcomment: " << comment << "\033[0m" << endl;

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
            return Get_Str(c_peek, tmptoken, error, finish_input);
        
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
                tmpstr = Get_Str(c_peek, tmptoken, error, finish_input); // get the string from cin
                if (tmpstr == "t")
                    tmptoken.value = "#t";
                tmptoken.is_function = false; // ! reset is_function flag
                tokenBuffer.push_back(tmptoken);
                // cerr << "\033[1;32m" << "tmpstr: " << tmpstr << "\033[0m" << endl;

                // no error, judge the syntax
                if (error == Error_None)
                    syntax.check_syntax(tokenBuffer, error);
                
                if (error != Error_None) {
                    // ! print test
                    // print_vector(tokenBuffer);
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

                // cerr << "\033[1;32m" << "tmptoken.value: " << tmptoken.value << "\033[0m" << endl;
                
            }
            catch (errorType error) {
                // ! when error occur, finish Get_Token(), go to main() print error message
                finish_input = true;
                
                handle_error(error, c_peek);
                // return; // ! return when error occur, go to main() print error message
            }
            
            if (tokenBuffer.at(0).type == QUOTE) {
                if (leftParen == 0) {    
                    int i = 0;
                    for( ; i < tokenBuffer.size(); i++) {
                        if (tokenBuffer.at(i).type != QUOTE) {
                            finish_input = true;
                            break; 
                        }
                    }
                }
                else if (leftParen == rightParen)
                    finish_input = true;

                
            }
            // if (tokenBuffer.size() == 1 && tokenBuffer.at(0).type == QUOTE)
            //     finish_input = false;
            else if ((level == 0 && tokenBuffer.size() > 0) || (leftParen != 0 && leftParen == rightParen))
                finish_input = true;
            
        }
        while(!finish_input);
  
        if (finish_input) {
            // cerr << "\033[1;32mfinish_input\033[0m" << endl;
            // print_vector(tokenBuffer);

            while (true) {
                c_peek = cin.peek();

                if (c_peek == EOF) {
                    // is_EOF = true;
                    // cerr << "after finish_input, throw error UNEXPECTED_EOF" << endl;
                    // c_peek = getchar(); // skip the EOF
                    // ungetc(c_peek, stdin); // push EOF back to the buffer
                    return; // 交給下次呼叫 get_token() 處理此 EOF，因為要先建樹
                }
                else if (is_enter(c_peek)) {
                    // cerr << "after finish_input, is_enter" << endl;
                    getchar(); // skip the char of '\n'}
                    return;
                }
                else if (is_space(c_peek)) {
                    // cerr << "after finish_input, is_space" << endl;
                    start_column++;
                    getchar(); // skip the char of ' '
                    continue;
                }
                else if (is_comment(c_peek)) {
                    // cerr << "after finish_input, is_comment" << endl;
                    string trash;
                    getline(cin, trash);
                    for (char c : trash) {
                        if (c == EOF)
                            ungetc(c, stdin); // push EOF back to the buffer
                    }
                    start_column = 1;
        
                    // cerr << "\033[1;33mthrow trash in get_token: " << trash << "\033[0m" << endl;
                    return;
                }
                else {
                    // cerr << "\033[1;31m"<< "c_peek: " << c_peek << "\033[0m" << endl;
                    return;
                }

            }
        }
        else {
            cerr << "\033[1;31mmeet unexpected error\033[0m" << endl;
        }
        /*if (finish_input) {
            cerr << "\033[1;32mfinish_input\033[0m" << endl;
            print_vector(tokenBuffer);
            string temp;

            while (true) {
                c_peek = cin.peek();

                if (!temp.empty()) {
                    while (true) {
                        c_peek = cin.peek();
                        if (c_peek == EOF) {
                            c_peek = getchar(); // skip the EOF
                            for (char c : temp)
                                ungetc(c, stdin); // push stored content back to the buffer
                            ungetc(c_peek, stdin);
                        }
                        else if (is_enter(c_peek) || is_space(c_peek) || is_comment(c_peek)) {
                            
                            return;
                        }
                        else
                            temp.push_back(getchar());
                    }
                    
                    return;
                }
                else if (c_peek == EOF) {
                    // is_EOF = true;
                    cerr << "after finish_input, throw error UNEXPECTED_EOF" << endl;
                    if (!temp.empty()) {
                        c_peek = getchar(); // skip the EOF
                        for (char c : temp) {
                            ungetc(c, stdin); // push stored content back to the buffer
                        }
                    }
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
                    start_column = 1;
        
                    cerr << "\033[1;33mthrow trash in get_token: " << trash << "\033[0m" << endl;
                    return;
                }
                else {
                    cerr << "\033[1;31m"<< "c_peek: " << c_peek << "\033[0m" << endl;
                    temp.push_back(getchar());
                    // return;
                }

            }
        }
        else {
            cerr << "\033[1;31mmeet unexpected error\033[0m" << endl;
        }*/
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

void push_bulid_in_func_in_defined_table() {
    for (const auto& func_name : bulid_in_func) {
        if (defined_table.find(func_name) == defined_table.end()) { // Check if the function is not already defined
            Node_Token* func_node = new Node_Token();
            pointer_gather.insert(func_node);
            func_node->token.type = SYMBOL;
            func_node->token.value = func_name;
            func_node->token.is_function = true;
            defined_table[func_name] = func_node;
        }
    }
}

SyntaxAnalyzer syn;

class FunctionExecutor {
    private:
    int count_args(Node_Token *args) {
        int count = 0;
        while (args->left != nullptr) {
            // cerr << "\033[1;35m" << "args->left: " << args->left->token.value << "\033[0m" << endl;
            args = args->right;
            count++;
        }
        // cerr << "\033[1;35m" << "count_args: " << count << "\033[0m" << endl;
        // cerr << "\033[1;35m-----------------------------------\033[0m" << endl;
        return count;
    }
    bool is_ATOM(TokenType type) {
        // <ATOM> ::= SYMBOL | INT | FLOAT | STRING | NIL | T | Left-PAREN Right-PAREN
        if (type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T) // || type == Left_PAREN || type == Right_PAREN
            return true;
        // QUOTE、DOT
        return false;
    }
    
    bool is_reserved_word(string str) {
        if (bulid_in_func.find(str) != bulid_in_func.end())
            return true;

        return false;
    }
    bool is_equ_address(Node_Token *arg1, Node_Token *arg2) {
        if (arg1 == nullptr && arg2 == nullptr) 
            return true;
        else if (arg1 == nullptr && arg2 != nullptr)
            return false;
        else if (arg1 != nullptr && arg2 == nullptr)
            return false;
        else if (arg1->token.type == INT || arg1->token.type == FLOAT || arg1->token.type == NIL || arg1->token.type == T) {
            if (arg1->token.type == arg2->token.type && arg1->token.value == arg2->token.value)
                return is_equ_address(arg1->left, arg2->left) && is_equ_address(arg1->right, arg2->right);
            else
                return false;
        }
        else if (arg1 != arg2)
            return false;
        else 
            return is_equ_address(arg1->left, arg2->left) && is_equ_address(arg1->right, arg2->right);

        return is_equ_address(arg1->left, arg2->left) && is_equ_address(arg1->right, arg2->right);
    }
    bool is_equ(Node_Token *arg1, Node_Token *arg2) {
        if (arg1 == nullptr && arg2 == nullptr) 
            return true;
        else if (arg1 == nullptr && arg2 != nullptr)
            return false;
        else if (arg1 != nullptr && arg2 == nullptr)
            return false;
        
        else if (arg1->token.type != arg2->token.type || arg1->token.value != arg2->token.value)
            return false;
        else 
            return is_equ(arg1->left, arg2->left) && is_equ(arg1->right, arg2->right);

        return is_equ(arg1->left, arg2->left) && is_equ(arg1->right, arg2->right);
    }
    Node_Token* sequence(Node_Token *cur, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = nullptr;
        while (cur != nullptr && cur->token.type != NIL) {
            try {
                node = evalution(cur->left, e, local_defined_table);
            }
            catch (Error err) {
                if (e == no_return_value) { //  || e == no_return_value_inernal
                    if (cur->right == nullptr || cur->right->token.type == NIL) {
                        throw Error(no_return_value, cur->left->token.value, "no return value", 0, 0, cur);
                        // throw err; // If it's the last s-exp
                    }
                    else {
                        // cerr << "\033[1;31m" << "in sequence no return error: " << err.message << "\033[0m" << endl;
                        // cerr 
                        e = Error_None; // otherwise, ignore the error
                        
                    }
                }
                else throw err;
            }
            cur = cur->right;
        }
        // cerr << "\033[1;35m" << "node: " << node->token.value << "\033[0m" << endl;
        return node;
    }
    public:
    Node_Token* define_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        cerr << "\033[1;33m" << "define_func" << "\033[0m" << endl;
        vector<Node_Token*> arg_list;
        int c = count_args(args);
        if (cur->parent != nullptr){
            e = error_level_define;
            throw Error(error_level_define, instr, "DEFINE", 0, 0);
        }
        else if (c < 2) {
            e = error_define_format;
            throw Error(error_define_format, "DEFINE", "error_define_format", 0, 0, cur);
        }
        else if (c > 2 || args->left->token.type == DOT) {
            if (args->left->token.type == DOT && args->left->left->token.type != QUOTE)  // (define (a) b c)
                return define_func_more(instr, cur, args, e, local_defined_table);
            // (define a (b) (c))
            e = error_define_format;
            throw Error(error_define_format, "DEFINE", "error_define_format", 0, 0, cur);
        }
        else {

            Node_Token *t = args;
            // ! while (t != nullptr && t->token.type != NIL)
            for (int i = 0 ; i < 2 ; i++) {
                Node_Token *parameter = t->left;
                
                if (i == 0) {
                    // cerr << "1.     !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1"<<endl;
                    if (parameter->token.type == SYMBOL) {
                        // none reserved word
                        if (is_reserved_word(parameter->token.value)) {
                            e = error_define_format;
                            throw Error(error_define_format, "DEFINE", "error_define_format", 0, 0, cur);
                        }
                        arg_list.push_back(parameter);
                    }
                    else if (is_ATOM(parameter->token.type)) { // || parameter->token.type == DOT
                        // cerr << "2.     !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1"<<endl;

                        e = error_define_format;
                        throw Error(error_define_format, "DEFINE", "error_define_format", 0, 0, cur);
                    }
                }
                else {
                    try {
                        arg_list.push_back(evalution(parameter, e, local_defined_table));
                    }
                    catch (Error err) {
                        // cerr << "\033[1;35m" << "error: " << error << "\033[0m" << endl;
                        if (e == no_return_value) {
                            e = no_return_value_inernal;
                            // throw Error(no_return_value_inernal, instr, "unbound parameter", 0, 0, parameter);
                            throw Error(no_return_value_inernal, err.current, "no_return_value_inernal", 0, 0, err.root);
                        }
                        else
                            throw err;
                    }
                }

                if (i == 1) {
                    defined_table[arg_list.at(0)->token.value] = arg_list.at(1);
                    e = defined;
                    throw Error(defined, arg_list.at(0)->token.value, "defined la la", 0, 0, cur);
                
                }
                
                t = t->right;
            }
            
            
        }
        return nullptr;
    }

    Node_Token* define_func_more(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        cerr << "\033[1;33m" << "define_func_more" << "\033[0m" << endl;
        vector<Node_Token*> arg_list;
        int c = count_args(args);
        //                  * <- cur
        //                /   \
        //             define  . <- args
        //                   /   \
        //                 .       .
        //                / \     / \
        //               a  nil expr1 .
        //                           / \
        //                       expr2  nil

        Node_Token* func = new Node_Token();
        pointer_gather.insert(func);
        func->token.value = args->left->left->token.value; // function name
        func->token.type = args->left->left->token.type; // function name
        // cerr << "func->token.value: " << func->token.value << endl;
        // cerr << "func->token.type: " << func->token.type << endl;

        if (func->token.type == SYMBOL) {
            // cerr << "func->token.type == SYMBOL" << endl;
            if (is_reserved_word(func->token.value)) {
                // cerr << "func->token.type == SYMBOL && is_reserved_word(func->token.value)" << endl;
                e = error_define_format;
                throw Error(error_define_format, "DEFINE", "error_define_format", 0, 0, cur);
            }
        }
        else if ( is_ATOM(func->token.type) || func->token.type == DOT || func->token.type == QUOTE) {
            e = error_define_format;
            throw Error(error_define_format, "DEFINE", "error_define_format", 0, 0, cur);
        }

        // cerr << "\033[1;33m" << "set " << func->token.value <<" is_function = true " << "\033[0m" << endl;
        func->token.is_function = true;
        func->left = args->left->right; // parameter list, 有可能是nil, none args
        func->right = args->right; // expression, 有可能有多個 expr

        //                  a <- func
        //                /   \
        //              nil     .
        //                     / \
        //                  expr1  .
        //                        / \
        //                   expr2  nil

        defined_table[func->token.value] = func; // store the function name and function body
        e = defined;
        throw Error(defined, func->token.value, "defined la la", 0, 0, cur);

        return nullptr;
    }

    Node_Token* self_defined_function(string instr, Node_Token *cur, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        cerr << "\033[1;33m" << "--------- enter self_defined_function ---------" << "\033[0m" << endl;
        Node_Token* node;
        vector<Node_Token*> arg_list;
        unordered_map<string, Node_Token*> new_table; // = local_defined_table

        // *defined local variable
        Node_Token* func_name = cur->left; // function name
        string func_name_str;
        Node_Token* func_args; // 參數列表
        Node_Token* func_exprs; // 表達式列表
        if (local_defined_table.find(func_name->token.value) != local_defined_table.end()) {
            func_args = local_defined_table[func_name->token.value]->left;
            func_exprs = local_defined_table[func_name->token.value]->right;
            func_name_str = local_defined_table[func_name->token.value]->token.value;
        }
        else if (defined_table.find(func_name->token.value) != defined_table.end()) {
            func_args = defined_table[func_name->token.value]->left;
            func_exprs = defined_table[func_name->token.value]->right;
            func_name_str = defined_table[func_name->token.value]->token.value;
        }
        else {
            e = undefined_function;
            throw Error(undefined_function, func_name->token.value, "undefined function", 0, 0, cur);
        }
        
        // count args
        // cerr << "\033[1;31mstart counting \033[0m" << endl;
        if (count_args(func_args) != count_args(cur->right)) { // 兩個以上參數
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, func_name_str, "incorrect_number_of_arguments", 0, 0, cur);
        }
        // cerr << "\033[1;31mend counting \033[0m" << endl;
        


        Node_Token* args = cur->right;
        // lambda_args->token.type != NIL && lambda_args != nullptr && defined_args->token.type != NIL && defined_args != nullptr
        // cerr << "\033[1;32m" << " *** test1 *** "  << "\033[0m" << endl;
        while (args->token.type != NIL && args != nullptr) {
            // cerr << "\033[1;35m" << "lambda_args: " << args->left->token.value << "\033[0m" << endl;
            try {
                arg_list.push_back(evalution(args->left, e, local_defined_table));
                // cerr << "\033[1;36m" << "lambda_args: " << args->left->token.value << "\033[0m" << endl;
                // cerr << "\033[1;36m" << "arg_list: " << arg_list.at(arg_list.size()-1)->token.value << "\033[0m" << endl;
            }
            catch (Error err) {
                // cerr << "\033[1;35m" << "error: " << error << "\033[0m" << endl;
                if (e == no_return_value) {
                    e = unbound_parameter;
                    throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                }
                else
                    throw err;
            }
            args = args->right;
        }

        for (auto arg : arg_list) {
            if (func_args->token.type == NIL || func_args == nullptr) {
                e = incorrect_number_of_arguments;
                string func_name_str;
                if (defined_table.find(func_name->token.value) != defined_table.end())
                    func_name_str = defined_table[func_name->token.value]->token.value;
                else func_name_str = func_name->token.value;
                
                throw Error(incorrect_number_of_arguments, func_name_str, "incorrect_number_of_arguments", 0, 0, cur);
            }
            new_table[func_args->left->token.value] = arg;
            func_args = func_args->right;
        }

        if (func_args->token.type != NIL) {
            e = incorrect_number_of_arguments;
            string func_name_str;
                if (defined_table.find(func_name->token.value) != defined_table.end())
                    func_name_str = defined_table[func_name->token.value]->token.value;
                else func_name_str = func_name->token.value;

            throw Error(incorrect_number_of_arguments, func_name_str, "incorrect_number_of_arguments", 0, 0, cur);
        }
        // cerr << "\033[1;32m" << " *** test2 *** "  << "\033[0m" << endl;
        try {
            return sequence(func_exprs, e, new_table);
        }
        catch (Error err) {
            if (e == no_return_value) {
                // e = no_return_value_inernal;
                throw Error(no_return_value, func_name->token.value, "no return value", 0, 0, func_exprs);
            }
            
            throw Error(err.type, err.expected, err.message, 0, 0, err.root);
        }
        // return node;
    }

    Node_Token* cons(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;

        if (count_args(args) != 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                // cerr << "\033[1;35m" << "parameter: " << parameter->token.value << "\033[0m" << endl;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                }
                catch (Error err) {
                    // cerr << "\033[1;35m" << "error: " << error << "\033[0m" << endl;
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }

                t = t->right;
            }
            // Create a new node for the cons cell
            node->token.type = DOT;
            node->token.value = ".";
            node->left = arg_list.at(0);
            // cerr << "\033[1;35m" << "arg_list.at(0): " << arg_list.at(0)->token.value << "\033[0m" << endl;
            node->right = arg_list.at(1);
            // cerr << "\033[1;35m" << "arg_list.at(1): " << arg_list.at(1)->token.value << "\033[0m" << endl;
        }

        return node;
    }
    Node_Token* lambda(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        // cerr << "\033[1;33m" << "--------- enter lambda ---------" << "\033[0m" << endl;

        // cerr << "\033[1;35m" << "cur->left: " << cur->left->token.value << "\033[0m" << endl;
        if (cur->left->token.value != "lambda" && defined_table.find(cur->left->token.value) != defined_table.end()) {
            // cerr << "\033[1;35m" << "cur->left: " << cur->left->token.value << "\033[0m" << endl;
            if (defined_table[cur->left->token.value]->token.is_function) {
                // Node_Token* func_Token = defined_table[cur->left->token.value];
                Node_Token* t = defined_table[cur->left->token.value];
                // cerr << "\033[1;33m" << "--------- judge enter execute_lambda ---------" << "\033[0m" << endl;

                if (t != nullptr && t->token.value == "lambda") {
                    // cerr << "\033[1;33m" << "--------- enter execute_lambda ---------" << "\033[0m" << endl;
                    // 若為 lambda function, 則
                    //                  lambda <- t.token
                    //                  /    \
                    //          tmp -> *      * 
                    //               / \    /   \
                    //            arg1 *  body1  *
                    //                / \      /   \
                    //              arg2 nil  body2 nil
                    return execute_lambda(cur, t, cur->right, e, local_defined_table); // execute lambda function
                }
            }
        }
        else if (cur->left->token.value != "lambda" && local_defined_table.find(cur->left->token.value) != local_defined_table.end()) {
            // cerr << "\033[1;35m" << "cur->left: " << cur->left->token.value << "\033[0m" << endl;
            if (local_defined_table[cur->left->token.value]->token.is_function) {
                // Node_Token* func_Token = defined_table[cur->left->token.value];
                Node_Token* t = local_defined_table[cur->left->token.value];
                // cerr << "\033[1;33m" << "--------- judge enter execute_lambda ---------" << "\033[0m" << endl;

                if (t != nullptr && t->token.value == "lambda") {
                    // cerr << "\033[1;33m" << "--------- enter execute_lambda ---------" << "\033[0m" << endl;
                    // 若為 lambda function, 則
                    //                  lambda <- t.token
                    //                  /    \
                    //          tmp -> *      * 
                    //               / \    /   \
                    //            arg1 *  body1  *
                    //                / \      /   \
                    //              arg2 nil  body2 nil
                    return execute_lambda(cur, t, cur->right, e, local_defined_table); // execute lambda function
                }
            }
        }

        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        
        node->token.type = SYMBOL;
        node->token.value = "lambda";
        node->token.is_function = true;


        if (count_args(args) < 2) { // 兩個以上參數
            
            e = error_define_format;
            // cur->left->token.is_function = false;
            throw Error(error_define_format, "LAMBDA", "error_define_format", 0, 0, cur);
        }
        else if (args->left->token.type != DOT && args->left->token.type != NIL ) { // (lambda x  (y z)) -> error
            e = error_define_format;
            throw Error(error_define_format, "LAMBDA", "error_define_format", 0, 0, cur);
        }

        else {
            node->left = args->left; // 參數列表
            node->right = args->right; // expression
            Node_Token* left_node;
            if (args->left->token.type == NIL) left_node = nullptr; // ( lambda () (y)( z) ) == ( lambda nil (y)( z) ) == ( lambda #f (y)( z) )
            else {
                left_node = node->left;
                if (left_node->left->token.type == NIL) { // ( lambda (nil) (y)( z) ) == ( lambda (#f) (y)( z) )
                    e = error_define_format;
                    throw Error(error_define_format, "LAMBDA", "error_define_format", 0, 0, cur);
                }
            }

            while (left_node != nullptr && left_node->token.type != NIL) {
                if (left_node->left->token.type != SYMBOL) {
                    if (left_node->left->token.type == DOT)
                        left_node->left = evalution(left_node->left, e, local_defined_table);
                    else {
                        e = error_define_format;
                        throw Error(error_define_format, "LAMBDA", "error_define_format", 0, 0, cur);
                    }
                }
                // 排除保留字
                else if (is_reserved_word(left_node->left->token.value)) {
                    e = error_define_format;
                    throw Error(error_define_format, "LAMBDA", "error_define_format", 0, 0, cur);
                }
                
                left_node = left_node->right;
            }

        }

        return node;
    }
    Node_Token* execute_lambda(Node_Token *cur, Node_Token *lambda, Node_Token *defined_args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        cerr << "\033[1;33m" << "--------- enter execute_lambda ---------" << "\033[0m" << endl;
        Node_Token* node;
        vector<Node_Token*> arg_list;
        unordered_map<string, Node_Token*> new_table; //  = local_defined_table

        int init_c = count_args(lambda->left);
        int input_c = count_args(defined_args); // lambda args
        // cerr << "\033[1;35m" << "init_c: " << init_c << "\033[0m" << endl;
        // cerr << "\033[1;35m" << "input_c: " << input_c << "\033[0m" << endl;
        if (input_c != init_c) { // 參數量不相同
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, lambda->token.value, "incorrect_number_of_arguments", 0, 0, lambda);
        }
        
        // *defined local variable
        
        while (defined_args->token.type != NIL && defined_args != nullptr) {
            // cerr << "\033[1;35m" << "lambda_args: " << lambda_args->left->token.value << "\033[0m" << endl;
            try {
                arg_list.push_back(evalution(defined_args->left, e, local_defined_table));
            }
            catch (Error err) {
                if (e == no_return_value) {
                    e = unbound_parameter;
                    throw Error(unbound_parameter, defined_args->left->token.value, "unbound_parameter", 0, 0, err.root);
                }

                throw err;
            }
            defined_args = defined_args->right;
        }

        Node_Token* lambda_args = lambda->left;
        for (auto arg : arg_list) {
            if (lambda_args->token.type == NIL || lambda_args == nullptr) {
                e = incorrect_number_of_arguments;
                throw Error(incorrect_number_of_arguments, lambda->token.value, "incorrect_number_of_arguments", 0, 0, lambda);
            }
            // cerr << "\033[1;31m" << "lambda_args: " << lambda_args->left->token.value << "\033[0m" << endl;
            // cerr << "\033[1;31m" << "arg: " << arg->token.value << "\033[0m" << endl;
            new_table[lambda_args->left->token.value] = arg;
            lambda_args = lambda_args->right;
        }

        if (lambda_args->token.type != NIL) {
            if (e != Error_None) 
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, lambda->token.value, "incorrect_number_of_arguments", 0, 0, lambda);
        }


        try {
            return sequence(lambda->right, e, new_table);
        }
        catch (Error err) {
            if (e == no_return_value) {
                // e = no_return_value_inernal;
                throw Error(no_return_value, lambda->token.value, "no return value", 0, 0, cur);
            }
            
            throw Error(err.type, err.expected, err.message, 0, 0, err.root);
        }
    }
    
    Node_Token* let(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        // cerr << "\033[1;33m" << "--------- enter let ---------" << "\033[0m" << endl;
        Node_Token* node;
        // pointer_gather.insert(node);
        // vector<Node_Token*> arg_list;
        unordered_map<string, Node_Token*> new_table = local_defined_table;
        
        
        if (count_args(args) < 2) { // 兩個以上參數
            // cerr << "\033[1;35m" << "error: 1. error_define_format\033[0m" << endl;
            e = error_define_format;
            // cur->left->token.is_function = false;
            throw Error(error_define_format, "LET", "error_define_format", 0, 0, cur);
        }
        
        Node_Token* first_arg = args->left;
        if (first_arg->token.type != DOT && first_arg->token.type != NIL) { // 第一個參數必須是 expression
            e = error_define_format;
            throw Error(error_define_format, "LET", "error_define_format", 0, 0, cur);
        }


        while (first_arg != nullptr && first_arg->token.type != NIL) {
            Node_Token* tmp = first_arg->left;
            if (tmp == nullptr || tmp->token.type != DOT || tmp->left->token.type != SYMBOL) {
                e = error_define_format;
                throw Error(error_define_format, "LET", "error_define_format", 0, 0, cur);
            }
            // else
            if (is_reserved_word(tmp->left->token.value)) {
                e = error_define_format;
                throw Error(error_define_format, "LET", "error_define_format", 0, 0, cur);
            }

            if (count_args(tmp) != 2) { // (x 1 2) -> error, (x 1) -> ok
                e = error_define_format;
                throw Error(error_define_format, "LET", "error_define_format", 0, 0, cur);
            }
            first_arg = first_arg->right;
        }
        
        // 對區域變數進行綁定
        first_arg = args->left;
        while (first_arg != nullptr && first_arg->token.type != NIL) {
            Node_Token* tmp = first_arg->left;

            try {
                // cerr << "\033[1;35m" << "4. arg: " << tmp->left->token.value << "\033[0m" << endl;
                new_table[tmp->left->token.value] = evalution(tmp->right->left, e, local_defined_table);
            }
            catch (Error err) {
                // cerr << "\033[1;31m" << "******* 1. let error *******" << "\033[0m" << endl;
                if (e == no_return_value) {
                    e = no_return_value_inernal;
                    throw Error(no_return_value_inernal, err.current, "no_return_value_inernal", 0, 0, err.root); // arg->right->left
                    // throw Error(no_return_value, instr, "no_return_value", 0, 0, cur); // arg->right->left
                }
                throw err;
            }
            
            first_arg = first_arg->right;
        }

        Node_Token* other_arg = args->right;

        try {
            return sequence(other_arg, e, new_table);
        }
        catch (Error err) {
            if (e == no_return_value) {
                // e = no_return_value_inernal;
                throw Error(no_return_value, other_arg->token.value, "no return value", 0, 0, cur);
            }
            
            throw Error(err.type, err.expected, err.message, 0, 0, err.root);
        }
    }
    // ! (list '(4 5))
    Node_Token* list_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        node->token.type = DOT;
        node->token.value = ".";

        if (count_args(args) < 1) {
            // throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
            node->token.type = NIL;
            node->token.value = "#f";
            return node;
        }
        else {
            Node_Token* current = node;
            Node_Token* head = node;

            while (args != nullptr && args->token.type != NIL) {
                Node_Token* parameter = args->left;
                Node_Token* evaluated = nullptr;

                try {
                    evaluated = evalution(parameter, e, local_defined_table);
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }

                current->left = evaluated;
                if (args->right != nullptr && args->right->token.type != NIL) {
                    Node_Token* next = new Node_Token();
                    pointer_gather.insert(next);
                    next->token.type = DOT;
                    next->token.value = ".";
                    current->right = next;
                    current = next;
                }
                else {
                    current->right = new Node_Token();
                    pointer_gather.insert(current->right);
                    current->right->token.type = NIL;
                    current->right->token.value = "#f";
                }

                args = args->right;
            }

            return head;
        }
    }
    Node_Token* car(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *parameter = args->left;
            try {
                parameter = evalution(args->left, e, local_defined_table);
            }
            catch (Error err) {
                if (e == no_return_value) {
                    e = unbound_parameter;
                    throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                }
                else
                    throw err;
            }

            if (parameter->token.type != DOT) {
                e = incorrect_argument_type;
                throw Error(incorrect_argument_type, instr, parameter->token.value, 0, 0, parameter);
            }
            else 
                return parameter->left;
            
        }

        // return nullptr;
    }
    Node_Token* cdr(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *parameter = args->left;
            try {
                parameter = evalution(args->left, e, local_defined_table);
            }
            catch (Error err) {
                if (e == no_return_value) {
                    e = unbound_parameter;
                    throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                }
                else
                    throw err;
            }

            if (is_ATOM(parameter->token.type)) {
                e = incorrect_argument_type;
                throw Error(incorrect_argument_type, instr, parameter->token.value, 0, 0, parameter);
            }
            else 
                return parameter->right;
            
        }
    }
    
    Node_Token* judge_elements(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        node->token.value = "#f";
        node->token.type = NIL;

        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            
            if (args != nullptr && args->token.type != NIL) {
                Node_Token *parameter = args->left;
                try {
                    parameter = evalution(args->left, e, local_defined_table);
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }
                if (instr == "atom?") {
                    if (is_ATOM(parameter->token.type) || parameter->token.type == USERERROR) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "pair?") {
                    if (parameter->token.type == DOT) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "list?") {
                    if (parameter->token.type == DOT) {
                        Node_Token *tmp = parameter;
                        while (tmp->right != nullptr)
                            tmp = tmp->right;
                        
                        if (tmp->token.type == NIL) {
                            node->token.value = "#t";
                            node->token.type = T;
                        }
                        
                    }
                }
                else if (instr == "null?") {
                    if (parameter->token.type == NIL) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "integer?") {
                    if (parameter->token.type == INT) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "real?" || instr == "number?") {
                    if (parameter->token.type == INT || parameter->token.type == FLOAT) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "string?") {
                    if (parameter->token.type == STRING || parameter->token.type == USERERROR) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "boolean?") {
                    if (parameter->token.type == T || parameter->token.type == NIL) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }
                else if (instr == "symbol?") {
                    if (parameter->token.type == SYMBOL) {
                        node->token.value = "#t";
                        node->token.type = T;
                    }
                }

            }
            
        }
        return node;
    }
    
    // (/ + - -)
    // > ERROR (/ with incorrect argument type) : #<procedure +>
    Node_Token* implement_arithmetic(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;
        bool float_flag = false;

        if (count_args(args) < 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        if (parameter->left->token.type == DOT)
                            throw err;
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }

                if (arg_list.back()->token.type == FLOAT)
                    float_flag = true;
                else if (arg_list.back()->token.type == DOT) {
                    e = incorrect_argument_type;
                    throw Error(incorrect_argument_type, instr, "incorrect_argument_type", 0, 0, arg_list.back());
                }
                else if (arg_list.back()->token.type != INT && arg_list.back()->token.type != FLOAT) {
                    e = incorrect_argument_type;
                    throw Error(incorrect_argument_type, instr, arg_list.back()->token.value, 0, 0, arg_list.back());
                }

                t = t->right;
            }
        }

        if (float_flag) {
            double sum = stod(arg_list.at(0)->token.value);
            for (int i = 1 ; i < arg_list.size() ; i++) {
                if (instr == "+")
                    sum += stod(arg_list.at(i)->token.value);
                else if (instr == "-")
                    sum -= stod(arg_list.at(i)->token.value);
                else if (instr == "*")
                    sum *= stod(arg_list.at(i)->token.value);
                else if (instr == "/") {
                    double divisor = stod(arg_list.at(i)->token.value);
                    if (divisor == 0.0) {
                        e = division_by_zero;
                        throw Error(division_by_zero, "/", "division by zero", 0, 0);
                    }
                    sum /= divisor;
                }
            }
            node->token.value = to_string(sum);
            node->token.type = FLOAT;
        }
        else {
            int sum = stoi(arg_list.at(0)->token.value);
            for (int i = 1 ; i < arg_list.size() ; i++) {
                if (instr == "+")
                    sum += stoi(arg_list.at(i)->token.value);
                else if (instr == "-")
                    sum -= stoi(arg_list.at(i)->token.value);
                else if (instr == "*")
                    sum *= stoi(arg_list.at(i)->token.value);
                else if (instr == "/") {
                    double divisor = stoi(arg_list.at(i)->token.value);
                    if (divisor == 0.0) {
                        e = division_by_zero;
                        throw Error(division_by_zero, "/", "division by zero", 0, 0);
                    }
                    sum /= divisor;
                }
                
            }
            node->token.value = to_string(sum);
            node->token.type = INT;
        }

        return node;
    }

    Node_Token* implement_logical(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;
        node->token.value = "#t";
        node->token.type = T;

        if (instr == "not" && count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else if ((instr == "and" || instr == "or") && count_args(args) < 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                    if (instr == "not") {
                        if (arg_list.back()->token.type != NIL) {
                            node->token.value = "#f";
                            node->token.type = NIL;
                        }
                    }
                    /*
                    第一個被計算為 nil 則回傳 nil
                    如果不是 nil 則回傳最後一個計算出的值
                    */
                    else if (instr == "and") {
                        if (arg_list.back()->token.type == NIL)
                            return arg_list.back();
                        else 
                            node = arg_list.back();
                    }
                    /* 
                    第一個被計算為非 nil 則直接回傳
                    如果前面都是 nil 則回傳最後一個的值
                    */
                    else if (instr == "or") {
                        if (arg_list.back()->token.type != NIL)
                            return arg_list.back();
                        else
                            node = arg_list.back();
                    }
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_condition; // unbound condition
                        throw Error(unbound_condition, instr, "unbound condition", 0, 0, err.root);
                    }
                    else
                        throw err;
                }

                t = t->right;
            }
        }

        return node;
    }

    Node_Token* compare_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;
        bool float_flag = false;
        node->token.value = "#t";
        node->token.type = T;

        if (count_args(args) < 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }

                if(arg_list.back()->token.type == FLOAT)
                    float_flag = true;
                else if (arg_list.back()->token.type != INT && arg_list.back()->token.type != FLOAT) {
                    // ERROR (+ with incorrect argument type) : #t
                    e = incorrect_argument_type;
                    throw Error(incorrect_argument_type, instr, arg_list.back()->token.value, 0, 0, arg_list.back());
                }
                t = t->right;
            }
        }

        if (float_flag) {
            double current = stod(arg_list.at(0)->token.value);
            for (int i = 1 ; i < arg_list.size() ; i++) {
                if (instr == "=") {
                    if (current != stod(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == "<") {
                    if (current >= stod(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == ">") {
                    if (current <= stod(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == "<=") {
                    if (current > stod(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == ">=") {
                    if (current < stod(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                current = stod(arg_list.at(i)->token.value);
            }
        }
        else {
            int current = stoi(arg_list.at(0)->token.value);
            for (int i = 1 ; i < arg_list.size() ; i++) {
                if (instr == "=") {
                    if (current != stoi(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == "<") {
                    if (current >= stoi(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == ">") {
                    if (current <= stoi(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == "<=") {
                    if (current > stoi(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                else if (instr == ">=") {
                    if (current < stoi(arg_list.at(i)->token.value)) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                current = stoi(arg_list.at(i)->token.value);
            }
        }

        return node;
    }
    Node_Token* str_operator(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;
        node->token.value = "#t";
        node->token.type = T;

        if (count_args(args) < 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                // cerr << "\033[1;35m" << "parameter: " << parameter->token.value << "\033[0m" << endl;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                    if (arg_list.back()->token.type != STRING) {
                        e = incorrect_argument_type;
                        throw Error(incorrect_argument_type, instr, arg_list.back()->token.value, 0, 0, arg_list.back());
                    }
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }
                t = t->right;
            }
        }

        for (int i = 0 ; i < arg_list.size() ; i++) {
            if (instr == "string-append") {
                if (i == 0) {
                    node->token.value = arg_list.at(i)->token.value.substr(1, arg_list.at(i)->token.value.size() - 2);
                    node->token.type = STRING;
                }
                else node->token.value += arg_list.at(i)->token.value.substr(1, arg_list.at(i)->token.value.size() - 2);

                if (i == arg_list.size()-1)
                    node->token.value = "\"" + node->token.value + "\"";
                
            }
            else if (instr == "string>?") {
                if (i == 0) continue;
                else {
                    for (int j = 0 ; j < arg_list.at(i-1)->token.value.size() && j < arg_list.at(i)->token.value.size() ; j++) {
                        if (arg_list.at(i-1)->token.value[j] < arg_list.at(i)->token.value[j]) {
                            node->token.value = "#f";
                            node->token.type = NIL;
                            return node;
                        }
                        else if (arg_list.at(i-1)->token.value[j] > arg_list.at(i)->token.value[j])
                            break;
                        
                    }
                    if (arg_list.at(i-1)->token.value.size() <= arg_list.at(i)->token.value.size() &&
                        arg_list.at(i-1)->token.value == arg_list.at(i)->token.value.substr(0, arg_list.at(i-1)->token.value.size())) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                }
                
            }
            else if (instr == "string<?") {
                if (i == 0) continue;
                else {
                    for (int j = 0 ; j < arg_list.at(i-1)->token.value.size() && j < arg_list.at(i)->token.value.size() ; j++) {
                        if (arg_list.at(i-1)->token.value[j] > arg_list.at(i)->token.value[j]) {
                            node->token.value = "#f";
                            node->token.type = NIL;
                            return node;
                        }
                        else if (arg_list.at(i-1)->token.value[j] < arg_list.at(i)->token.value[j])
                            break;
                        
                    }
                    if (arg_list.at(i-1)->token.value.size() >= arg_list.at(i)->token.value.size() &&
                        arg_list.at(i-1)->token.value == arg_list.at(i)->token.value.substr(0, arg_list.at(i-1)->token.value.size())) {
                        node->token.value = "#f";
                        node->token.type = NIL;
                        return node;
                    }
                    
                }
            }
            else if (instr == "string=?") {
                if (i == 0) continue;
                else if (arg_list.at(i)->token.value != arg_list.at(i-1)->token.value) {
                    node->token.value = "#f";
                    node->token.type = NIL;
                    return node;
                }
            }
        }

        return node;
    }

    Node_Token* eqv(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;
        node->token.value = "#t";
        node->token.type = T;

        if (count_args(args) != 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }
                t = t->right;
            }
        }

        if (!is_equ_address(arg_list.at(0), arg_list.at(1))) {
            node->token.value = "#f";
            node->token.type = NIL;
        }

        return node;
    }
    Node_Token* equal(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        vector<Node_Token*> arg_list;
        node->token.value = "#t";
        node->token.type = T;

        if (count_args(args) != 2) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            Node_Token *t = args;
            while (t != nullptr && t->token.type != NIL) {
                Node_Token *parameter = t->left;
                try {
                    arg_list.push_back(evalution(parameter, e, local_defined_table));
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }
                t = t->right;
            }
        }
        
        if (!is_equ(arg_list.at(0), arg_list.at(1))) {
            node->token.value = "#f";
            node->token.type = NIL;
        }

        return node;
    }
    Node_Token* begin_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node;
        vector<Node_Token*> arg_list;

        if (count_args(args) < 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            // Return the last evaluated argument
            try {
                return sequence(args, e, local_defined_table);
            }
            catch (Error err) {
                cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                // if (e == no_return_value) {
                //     e = unbound_parameter;
                //     throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                // }
                // else
                    throw err;
                
            }
        }


    }

    Node_Token* if_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        vector<Node_Token*> arg_list;

        int c = count_args(args);
        if (c != 2 && c != 3) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        else {
            // (if 條件式 判斷式 判斷式)
            Node_Token *judge_elements;
            
            try {
                judge_elements = evalution(args->left, e, local_defined_table);
                // cout << "\033[1;35m" << "judge_elements: " << judge_elements->token.value << "\033[0m" << endl;
                // cout << "\033[1;35m" << "judge_elements type: " << judge_elements->token.type << "\033[0m" << endl;
            }
            catch (Error err) {
                if (e == no_return_value) { // 條件式為 no_return_value 回傳 unbound_test_condition
                    e = unbound_test_condition;
                    throw Error(unbound_test_condition, instr, "unbound_test_condition", 0, 0, err.root);
                }
                else
                    throw err;
            }
            
            Node_Token *exp = args->right;
            // while (exp != nullptr && exp->token.type != NIL) {
            Node_Token *ans;
            if (c == 2) { 
                cerr << "\033[1;31m" << "c == 2" << "\033[0m" << endl;
                // try {
                //     ans = evalution(exp->left, e, local_defined_table);
                // }
                // catch (Error err) {
                //     throw err;
                // }

                if (judge_elements->token.type == NIL) {
                    e = no_return_value;
                    throw Error(no_return_value, instr, "no_return_value", 0, 0, cur);
                }
                // else return ans; // 直接 return right 的執行結果
                return evalution(exp->left, e, local_defined_table);
            }
            else if (c == 3) {
                cerr << "\033[1;31m" << "c == 3" << "\033[0m" << endl;
                try {
                    if (judge_elements->token.type != NIL) {
                        ans = evalution(exp->left, e, local_defined_table);
                        return ans;
                    }
                    else {
                        ans = evalution(exp->right->left, e, local_defined_table);
                        return ans;
                    } 
                }
                catch (Error err) {
                    throw err;
                }

            }

        }

        return nullptr;
    }
    Node_Token* cond_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);

        int c = count_args(args);
        if (count_args(args) < 1) {
            e = error_define_format;
            throw Error(error_define_format, instr, "cond_func", 0, 0, cur);
        }
        
        Node_Token *t = args;
        while (t != nullptr && t->token.type != NIL) {
            int h = 0;
            if (is_ATOM(t->left->token.type)) {
                e = error_define_format;
                throw Error(error_define_format, instr, "cond_func", 0, 0, cur);
            }
            else {
                Node_Token *tmp = t->left;
                while (tmp != nullptr && tmp->token.type != NIL) {
                    h++; // increment height
                    tmp = tmp->right;
                }
                if (h < 2) {
                    e = error_define_format;
                    throw Error(error_define_format, instr, "cond_func", 0, 0, cur);
                }
            }
            t = t->right;
        }

        t = args;
        while (t != nullptr && t->token.type != NIL) {
            Node_Token *parameter = t->left;

            // 判斷最後的 argument
            if (t->right->token.type == NIL) {
                // cerr << "\033[1;35m" << "last argument: " << parameter->left->token.value << "\033[0m" << endl;
                // try {
                Node_Token* tmp = parameter->left;
                if (parameter->left->token.type == DOT) {
                    try {
                        tmp = evalution(parameter->left, e, local_defined_table); // 這邊要 evalution 參數的左邊，設 tmp 為了避免改到原本的樹
                    }
                    catch (Error err) {
                        if (e == no_return_value) {
                            e = unbound_test_condition;
                            throw Error(unbound_test_condition, instr, "unbound_test_condition", 0, 0, err.root);
                        }
                        // else
                        throw err;
                    }
                    
                }
                    
                if (tmp->token.type == SYMBOL && tmp->token.value == "else") {
                    cerr << "\033[1;36m" << "--------------------- enter else -------------------: " << "\033[0m" << endl;
                    return sequence(parameter->right, e, local_defined_table); // 直接 return right 的執行結果
                }
                else {// 非 else
                    Node_Token* success;
                    try {
                        success = evalution(tmp, e, local_defined_table);
                    }
                    catch (Error err) { // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                        if (e == no_return_value) {
                            e = unbound_test_condition;
                            throw Error(unbound_test_condition, instr, "unbound_test_condition", 0, 0, err.root);
                        }
                        else
                            throw err;
                    }
                    
                    // cerr << "\033[1;31m" << "success: " << success->token.value << "\033[0m" << endl;
                    // cerr << "\033[1;31m" << "success: " << success->token.type << "\033[0m" << endl;
                    if (success->token.type != NIL)
                        return sequence(parameter->right, e, local_defined_table);
                }

            }
            // 非最後的 argument
            else {
                // 可執行
                Node_Token* success = nullptr;
                try {
                    success = evalution(parameter->left, e, local_defined_table);
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_test_condition;
                        throw Error(unbound_test_condition, instr, "unbound_test_condition", 0, 0, err.root);
                    }
                    else
                        throw err;
                }
                if (success->token.type != NIL) {
                    cerr << "\033[1;31m" << "success: " << success->token.value << "\033[0m" << endl;
                    return sequence(parameter->right, e, local_defined_table); // 直接 return right 的執行結果
                }
            }


            t = t->right;
        }
        e = no_return_value;
        throw Error(no_return_value, instr, "no_return_value", 0, 0, cur);
    }
    Node_Token* clean_environment(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (cur->parent != nullptr){
            e = error_level_cleaned;
            throw Error(error_level_cleaned, "CLEAN-ENVIRONMENT", "CLEAN-ENVIRONMENT", 0, 0);
        }
        else if (count_args(args) != 0) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, "clean-environment", "incorrect_number_of_arguments", 0, 0);
        }
        else {
            // func.clear();
            defined_table.clear();
            clear_pointer_gather();
            e = cleaned;
            throw Error(cleaned, "environment cleaned", "lalala", 0, 0);
        }
        return nullptr;
    }

    Node_Token* quote_func(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, "quote", "incorrect_number_of_arguments", 0, 0);
        }
        else 
            return args->left; // Directly return the quoted expression as is
        
    }
    Node_Token* exit_func(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (cur->parent != nullptr){
            e = error_level_exit;
            throw Error(error_level_exit, "EXIT", "EXIT", 0, 0);
        }
        else if (count_args(args) != 0) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, "exit", "incorrect_number_of_arguments", 0, 0);
        }
        else {
            e = UNEXPECTED_EXIT;
            throw Error(UNEXPECTED_EXIT, "exit", "exit", 0, 0);
        }
        return nullptr;
    }

    Node_Token* verbose_func(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        node->token.value = "#t";
        node->token.type = T;

        if (instr == "verbose") {
            if (count_args(args) != 1) {
                e = incorrect_number_of_arguments;
                throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
            }
            else {
                Node_Token *parameter = args->left;
                try {
                    Node_Token* left = evalution(parameter, e, local_defined_table);
                    if (left->token.type == NIL) {
                        verbose_mode = false;
                        node->token.value = "#f";
                        node->token.type = NIL;
                    }
                    else verbose_mode = true;
                }
                catch (Error err) {
                    if (e == no_return_value) {
                        e = unbound_parameter;
                        throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
                    }
                    else
                        throw err;
                }
            }
        }
        else if (instr == "verbose?") {
            if (count_args(args) != 0) {
                e = incorrect_number_of_arguments;
                throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
            }
            else {
                if (!verbose_mode) {
                    node->token.value = "#f";
                    node->token.type = NIL;
                }
            }
        }
        return node;
    }
    
    Node_Token* err_obj(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        node->token.value = "#t";
        node->token.type = T;

        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }

        Node_Token *parameter;
        try{
            parameter = evalution(args->left, e, local_defined_table);
        }
        catch (Error err) {
            if (e == no_return_value) {
                e = unbound_parameter;
                throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
            }
            else
                throw err;
        }

        if (instr == "create-error-object") {
            if (parameter->token.type != STRING) {
                e = incorrect_argument_type;
                throw Error(incorrect_argument_type, instr, parameter->token.value, 0, 0, parameter);
            }

            node->token.value = parameter->token.value;
            node->token.type = USERERROR;
            node->token.is_function = false;
            node->left = nullptr;
            node->right = nullptr;
            node->parent = nullptr;

        }
        else if (instr == "error-object?") {
            if (parameter->token.type != USERERROR) {
                node->token.value = "#f";
                node->token.type = NIL;
            }
        }

        return node;

    }
    
    Node_Token* read(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 0) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, "read", "incorrect_number_of_arguments", 0, 0);
        }
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        

        LexicalAnalyzer Lexical; //詞法分析器
        SyntaxAnalyzer Syntax; //語法分析器

        // bool is_exit = false;
        
        bool finish_input = false;
        try {
            // cout << "\n> ";
            errorType E = Error_None;
            Lexical.Get_Token(finish_input, E);

            if (finish_input) {
                // cerr << "\033[1;32mfinish_input\033[0m" << endl;
                // bulid parser tree
                // cerr << "\033[1;34menter build_tree\033[0m" << endl;
                Syntax.build_tree(Lexical.tokenBuffer, false);
                Syntax.set_root();
                // cerr << "\033[1;34mend build_tree\033[0m" << endl;
                // Syntax.print(); // !Debug

                return Syntax.get_root(); // return the root of the parser tree
            }

        } catch (Error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    cerr << "\033[1;31m" << "In read UNEXPECTED_TOKEN" << "\033[0m" << endl;
                    // cout << e.message << endl;
                    node->token.type = USERERROR;
                    node->token.value = "\"" + e.message + "\"";
                    
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    cerr << "\033[1;31m" << "In read UNEXPECTED_CLOSE_PAREN" << "\033[0m" << endl;
                    // cout << e.message << endl;
                    node->token.type = USERERROR;
                    node->token.value = "\"" + e.message + "\"";
                    
                    break;
                case UNEXPECTED_END_PAREN:
                    cerr << "\033[1;31m" << "In read UNEXPECTED_END_PAREN" << "\033[0m" << endl;
                    // cout << e.message << endl;
                    node->token.type = USERERROR;
                    node->token.value = "\"" + e.message + "\"";
                    
                    break;
                case UNEXPECTED_STRING:
                    cerr << "\033[1;31m" << "In read UNEXPECTED_STRING" << "\033[0m" << endl;
                    // cout << e.message << endl;
                    node->token.type = USERERROR;
                    node->token.value = "\"" + e.message + "\"";
                    
                    break;
                case UNEXPECTED_EOF:
                    cerr << "\033[1;31m" << "In read UNEXPECTED_EOF" << "\033[0m" << endl;
                    // cout << e.message << endl;
                    node->token.type = USERERROR;
                    node->token.value = "\"" + e.message + "\"";
                    
                    break;
                // case UNEXPECTED_EXIT:
                //     cerr << "\033[1;31m" << "In read UNEXPECTED_EXIT" << "\033[0m" << endl;
                //     cout << endl;
                    
                //     // is_exit = true;
                //     break;
                default:
                    break;
            }
        }

        return node;
    }

    Node_Token* write(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }
        Node_Token* node;

       

        try {
            node = evalution(args->left, e, local_defined_table);
            // if instruction is "write", the argement can be any type
            if (instr == "write")
                syn.print(node); // Print the evaluated value
            
            // if instruction is "display-string", the argement must be a string or error object
            else if (instr == "display-string") {
                if (node->token.type != STRING && node->token.type != USERERROR) {
                    e = incorrect_argument_type;
                    throw Error(incorrect_argument_type, "display-string", "incorrect_argument_type", 0, 0, args->left);
                }
                else {
                    string value = node->token.value;
                    // Remove the surrounding quotes
                    value = value.substr(1, value.size() - 2); // Remove the surrounding quotes
                    cout << value; // Print the string without quotes
                }
                
            }

        }
        catch (Error err) {
            if (e == no_return_value) {
                e = unbound_parameter;
                throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
            }
            else
                throw err;
        }

        return node; // Return the evaluated value
    }

    Node_Token* newline_func(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        node->token.value = "#f";
        node->token.type = NIL;

        if (count_args(args) != 0) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, "newline", "incorrect_number_of_arguments", 0, 0);
        }
        else {
            cout << endl; // Print a newline
        }

        return node;
    }

    Node_Token* To_String(string instr, Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, instr, "incorrect_number_of_arguments", 0, 0);
        }

        Node_Token* node = new Node_Token();
        pointer_gather.insert(node);
        node->token.type = STRING;

        Node_Token* parameter;
        try {
            parameter = evalution(args->left, e, local_defined_table);
        } catch (Error err) {
            if (e == no_return_value) {
                e = unbound_parameter;
                throw Error(unbound_parameter, instr, "unbound parameter", 0, 0, err.root);
            }
            throw err;
        }

        if (instr == "symbol->string") {
            // Only accept symbol
            if (parameter->token.type != SYMBOL) {
                e = incorrect_argument_type;
                throw Error(incorrect_argument_type, instr, parameter->token.value, 0, 0, parameter);
            }

            node->token.value = "\"" + parameter->token.value + "\""; // Surround with quotes
            
            
        }
        else if (instr == "number->string") {
            if (parameter->token.type != INT && parameter->token.type != FLOAT) {
                e = incorrect_argument_type;
                throw Error(incorrect_argument_type, instr, parameter->token.value, 0, 0, parameter);
            }
            string s;
            if (parameter->token.type == FLOAT)
                s = syn.process_float(parameter->token);
            else if (parameter->token.type == INT)
                s = parameter->token.value;
            node->token.value = "\"" + s + "\""; // Surround with quotes
        }

        return node;
    }

    Node_Token* eval(Node_Token *cur, Node_Token *args, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        if (count_args(args) != 1) {
            e = incorrect_number_of_arguments;
            throw Error(incorrect_number_of_arguments, "eval", "incorrect_number_of_arguments", 0, 0);
        }

        Node_Token* tmp = evalution(args->left, e, local_defined_table);
        return evalution(tmp, e, local_defined_table); // Evaluate the argument to get the expression to be evaluated

        // // If the argument is a quoted expression, evaluate the quoted content
        // if (expr->token.type == DOT && expr->left && expr->left->token.type == QUOTE) {
        //     // (eval '(...)) => evaluate the quoted content
        //     return evalution(expr->right->left, e, local_defined_table);
        // }
        // // If the argument is a string or atom, just return it as is
        // if (is_ATOM(expr->token.type)) {
        //     return expr;
        // }
        // // Otherwise, evaluate the argument as an expression
        
        // return evalution(expr, e, local_defined_table);
        
    }

    Node_Token* evalution(Node_Token *cur, errorType &e, unordered_map<string, Node_Token*> &local_defined_table) {
        // cerr << "-------- enter evalution --------" <<endl;
        if (cur == nullptr) return nullptr;

        string func_name;
        vector<Node_Token*> args;

        // Handle atoms
        if (is_ATOM(cur->token.type)) { //  != DOT &&  != QUOTE
            if (cur->token.type == SYMBOL) {
                // cerr << "\033[1;35m" << "---- judge define or not ----\ncur->token.value:" << cur->token.value << "\033[0m" << endl;
                if (local_defined_table.find(cur->token.value) != local_defined_table.end()) {
                    return local_defined_table[cur->token.value]; // Return the local bound symbol
                }
                else if (defined_table.find(cur->token.value) != defined_table.end()) {
                    return defined_table[cur->token.value]; // Return the bound symbol
                }
                else {
                    // cerr << "\033[1;35m" << "---- judge failed ----\ncur->token.value:" << cur->token.value << "\033[0m" << endl;
                    e = unbound_symbol;
                    throw Error(unbound_symbol, cur->token.value, "unbound symbol", cur->token.line, cur->token.column, cur);
                }
            }
            return cur; // Return the atom itself
        }

        // judge pure list or not, the rightmost node must be NIL so that can continue
        // if (...) is not a (pure) list
        Node_Token *t = cur;
        while (t != nullptr && t->token.type != NIL) {
            t = t->right;
        }
        if (t == nullptr || t->token.type != NIL) {
            // cerr << "\033[1;35m" << "--- pure list ---" << "\033[0m" << endl;
            e = non_list;
            throw Error(non_list, "non-list", "non-list", 0, 0, cur);
            // return cur; // Return the atom itself
        }

        // if first argument of (...) is an atom ☆, which is not a symbol
        // ERROR (attempt to apply non-function) : ☆
        Node_Token* func_Token = cur->left;
        func_name = func_Token->token.value;
        // cerr << "\033[1;33m" << "func_name: " << func_name << "\033[0m" << endl;
        // cerr << "\033[1;33m" << "func_Token->token.value: " << func_Token->token.value << "\033[0m" << endl;
        // cerr << "\033[1;33m" << "func_Token->token.type: " << func_Token->token.type << "\033[0m" << endl;

        if ( is_ATOM(func_Token->token.type) && func_Token->token.type != SYMBOL) {
            e = undefined_function;
            throw Error(undefined_function, func_Token->token.value, func_Token->token.value ,func_Token->token.line, func_Token->token.column, func_Token);
        }
        else if (func_Token->token.type == SYMBOL || func_Token->token.type == QUOTE) {
            // cerr << "\033[1;35m" << "is_ATOM(func_Token->token.type): " << func_Token->token.value << "\033[0m" << endl;
            // if (func_Token->token.type != SYMBOL ) { // || bulid_in_func.find(func_name) == bulid_in_func.end()
            // }
            if (local_defined_table.find(func_name) != local_defined_table.end()) { // check whether the first argument is a function
                // cerr << "\033[1;32m" << "In local_defined_table: " << func_name << "\033[0m" << endl;
                Node_Token* tmp;
                if (local_defined_table.find(func_name) != local_defined_table.end()) {
                    tmp = local_defined_table[func_name]; // get the function token
                    func_name = local_defined_table[func_name]->token.value;
                    // cerr << "\033[1;35m" << "is function: " << func_name << "\033[0m" << endl;
                    // cerr << "\033[1;35m" << "exec: " << func_Token->token.is_function << "\033[0m" << endl;
                }
                
                // not an executable function
                if (!tmp->token.is_function) {
                    // func_Token->token.value = func_name; // copy the function token
                    e = undefined_function;
                    throw Error(undefined_function, func_name, func_name ,func_Token->token.line, func_Token->token.column, tmp);
                }
                else {
                    // cerr << "\033[1;33m" << "********* set the function token to be executable *********"  << endl;
                    // func_Token->token.is_function = true; // set the function token to be executable
                }
            }
            else if (defined_table.find(func_name) != defined_table.end()) { // check whether the first argument is a function
                // cerr << "\033[1;32m" << "In defined_table: " << func_name << "\033[0m" << endl;
                Node_Token* tmp;
                if (defined_table.find(func_name) != defined_table.end()) {
                    tmp = defined_table[func_name]; // get the function token
                    func_name = defined_table[func_name]->token.value;
                    // cerr << "\033[1;35m" << "is function: " << func_name << "\033[0m" << endl;
                    // cerr << "\033[1;35m" << "exec: " << func_Token->token.is_function << "\033[0m" << endl;
                }
                
                // not an executable function
                if (!tmp->token.is_function) {
                    // func_Token->token.value = func_name; // copy the function token
                    // tmp->left = func_Token->left;
                    // tmp->right = func_Token->right;
                    e = undefined_function;
                    throw Error(undefined_function, func_name, func_name ,func_Token->token.line, func_Token->token.column, tmp);
                }
                // else {
                //     cerr << "\033[1;33m" << "********* set the function token to be executable *********"  << endl;
                //     // func_Token->token.is_function = true; // set the function token to be executable
                // }
            }
            
            if (func_name == "create-error-object")
                return err_obj("create-error-object", cur, cur->right, e, local_defined_table);
            else if (func_name == "error-object?")
                return err_obj("error-object?", cur, cur->right, e, local_defined_table);
            else if (func_name == "read")
                return read(cur, cur->right, e, local_defined_table);
            else if (func_name == "write" || func_name == "display-string")
                return write(func_name, cur, cur->right, e, local_defined_table);
            else if (func_name == "newline")
                return newline_func(cur, cur->right, e, local_defined_table);
            else if (func_name == "symbol->string" || func_name == "number->string")
                return To_String(func_name, cur, cur->right, e, local_defined_table);
            else if (func_name == "eval")
                return eval(cur, cur->right, e, local_defined_table);
            // else if (func_name == "set!")
            //     return


            
            else if (func_name == "define")
                return define_func("define", cur, cur->right, e, local_defined_table);
            else if (func_name == "cons")
                return cons("cons", cur, cur->right, e, local_defined_table);
            else if (func_name == "lambda")
                return lambda(cur, cur->right, e, local_defined_table);
            else if (func_name == "let")
                return let("let", cur, cur->right, e, local_defined_table);
            
            else if (func_name == "list")
                return list_func("list", cur, cur->right, e, local_defined_table);
            else if (func_name == "car")
                return car("car", cur, cur->right, e, local_defined_table);
            else if (func_name == "cdr")
                return cdr("cdr", cur, cur->right, e, local_defined_table);

            else if (func_name == "atom?")
                return judge_elements("atom?", cur, cur->right, e, local_defined_table);
            else if (func_name == "pair?")
                return judge_elements("pair?", cur, cur->right, e, local_defined_table);
            else if (func_name == "list?")
                return judge_elements("list?", cur, cur->right, e, local_defined_table);
            else if (func_name == "null?")
                return judge_elements("null?", cur, cur->right, e, local_defined_table);
            else if (func_name == "integer?")
                return judge_elements("integer?", cur, cur->right, e, local_defined_table);
            else if (func_name == "real?")
                return judge_elements("real?", cur, cur->right, e, local_defined_table);
            else if (func_name == "number?")
                return judge_elements("number?", cur, cur->right, e, local_defined_table);
            else if (func_name == "string?")
                return judge_elements("string?", cur, cur->right, e, local_defined_table);
            else if (func_name == "boolean?")
                return judge_elements("boolean?", cur, cur->right, e, local_defined_table);
            else if (func_name == "symbol?")
                return judge_elements("symbol?", cur, cur->right, e, local_defined_table);

            else if (func_name == "+")
                return implement_arithmetic("+", cur, cur->right, e, local_defined_table);
            else if (func_name == "-")
                return implement_arithmetic("-", cur, cur->right, e, local_defined_table);
            else if (func_name == "*")
                return implement_arithmetic("*", cur, cur->right, e, local_defined_table);
            else if (func_name == "/")
                return implement_arithmetic("/", cur, cur->right, e, local_defined_table);

            else if (func_name == "not")
                return implement_logical("not", cur, cur->right, e, local_defined_table);
            else if (func_name == "and")
                return implement_logical("and", cur, cur->right, e, local_defined_table);
            else if (func_name == "or")
                return implement_logical("or", cur, cur->right, e, local_defined_table);

            else if (func_name == "=")
                return compare_func("=", cur, cur->right, e, local_defined_table);
            else if (func_name == "<")
                return compare_func("<", cur, cur->right, e, local_defined_table);
            else if (func_name == ">")
                return compare_func(">", cur, cur->right, e, local_defined_table);
            else if (func_name == "<=")
                return compare_func("<=", cur, cur->right, e, local_defined_table);
            else if (func_name == ">=")
                return compare_func(">=", cur, cur->right, e, local_defined_table);

            else if (func_name == "string-append")
                return str_operator("string-append", cur, cur->right, e, local_defined_table);
            else if (func_name == "string>?")
                return str_operator("string>?", cur, cur->right, e, local_defined_table);
            else if (func_name == "string<?")
                return str_operator("string<?", cur, cur->right, e, local_defined_table);
            else if (func_name == "string=?")
                return str_operator("string=?", cur, cur->right, e, local_defined_table);

            else if (func_name == "eqv?")
                return eqv("eqv?", cur, cur->right, e, local_defined_table);
            else if (func_name == "equal?")
                return equal("equal?", cur, cur->right, e, local_defined_table);
            else if (func_name == "begin")
                return begin_func("begin", cur, cur->right, e, local_defined_table);

            else if (func_name == "if")
                return if_func("if", cur, cur->right, e, local_defined_table);
            else if (func_name == "cond")
                return cond_func("COND", cur, cur->right, e, local_defined_table);

            else if (func_name == "clean-environment")
                return clean_environment(cur, cur->right, e, local_defined_table);
            else if (func_name == "quote" || func_Token->token.type == QUOTE) // 
                return quote_func(cur, cur->right, e, local_defined_table);
            else if (func_name == "exit")
                return exit_func(cur, cur->right, e, local_defined_table);
            else if (func_name == "verbose" || func_name == "verbose?") 
                return verbose_func(func_name, cur, cur->right, e, local_defined_table);
                
            else if (defined_table.find(func_name) != defined_table.end()) {
                cerr << "-------- self defined function --------\n";
            //     cerr << "func_name: " << func_name << endl;
                try {
                    return self_defined_function(func_name, cur, e, local_defined_table); // execute self defined function
                }
                catch (Error err) {
                    if (e == no_return_value)
                        throw Error(no_return_value, func_name, err.expected, 0, 0, cur);
                    throw err;
                }
            }
            else if (local_defined_table.find(func_name) != local_defined_table.end()) {
                cerr << "-------- self local_defined_table function --------\n";
            //     cerr << "func_name: " << func_name << endl;
                try {
                    return self_defined_function(func_name, cur, e, local_defined_table); // execute self defined function
                }

                catch (Error err) {
                    if (e == no_return_value)
                        throw Error(no_return_value, func_name, err.expected, 0, 0, cur);
                    throw err;
                }
            }
            
            else { // undefined function
                // cerr << "-------- undefined function --------\n";
                
                e = unbound_symbol;
                throw Error(unbound_symbol, func_name, func_name ,func_Token->token.line, func_Token->token.column, cur);
            }

        }
        else { // the first argument of ( ... ) is ( 。。。 ), i.e., it is ( ( 。。。 ) ...... )
            // evaluate ( 。。。 )
            Node_Token* t;
            try {
                t = evalution(func_Token, e, local_defined_table);
            }
            catch (Error err) {
                if (e == no_return_value)
                    throw Error(no_return_value, func_name, err.expected, 0, 0, err.root);
                throw err;
            }
            cerr << "\033[1;33m" << "--------- judge enter execute_lambda ---------" << "\033[0m" << endl;
            // if (t != nullptr) {
            //     cerr << "\033[1;33m" << "t->token.value: " << t->token.value << "\033[0m" << endl;
            //     cerr << "\033[1;33m" << "t->left->token.type: " << t->left->token.type << "\033[0m" << endl;
            // }
            if (t != nullptr && t->token.value == "lambda") {
                cerr << "\033[1;33m" << "--------- enter execute_lambda ---------" << "\033[0m" << endl;
                // 若為 lambda function, 則
                //                  lambda <- t.token
                //                  /    \
                //          tmp -> *      * 
                //               / \    /   \
                //            arg1 *  body1  *
                //                / \      /   \
                //              arg2 nil  body2 nil
                
                try {
                    return execute_lambda(cur, t, cur->right, e, local_defined_table); // execute lambda function
                }
                catch (Error err) {
                    if (e == no_return_value)
                        throw Error(no_return_value, func_name, err.expected, 0, 0, cur);
                    throw err;
                }
            }
            

            // check whether the evaluated result (of ( 。。。 )) is an internal function
            if (local_defined_table.find(t->token.value) != local_defined_table.end()) {
                // cerr << "\033[1;34m" << "func.find(t->token.value) != func.end(): " << "\033[0m" << endl;
                Node_Token* node = new Node_Token();
                node->token.type = DOT;
                node->token.value = ".";
                node->left =t;
                node->right = cur->right;
                try {
                    return evalution(node, e, local_defined_table);
                }
                catch (Error err) {
                    if (e == no_return_value)
                        throw Error(no_return_value, func_name, err.expected, 0, 0, cur);
                    throw err;
                }
            }
            else if (defined_table.find(t->token.value) != defined_table.end()) {
                // cerr << "\033[1;34m" << "defined_table.find(t->token.value) != defined_table.end(): " << "\033[0m" << endl;
                Node_Token* node = new Node_Token();
                node->token.type = DOT;
                node->token.value = ".";
                node->left = defined_table[t->token.value];
                node->right = cur->right;
                try {
                    return evalution(node, e, local_defined_table);
                }
                catch (Error err) {
                    if (e == no_return_value)
                        throw Error(no_return_value, func_name, err.expected, 0, 0, cur);
                    throw err;
                }
            }
            else {
                // lambda ?????
                e = undefined_function;
                throw Error(undefined_function, t->token.value, t->token.value ,0, 0, t);
            }

        }
        
        return nullptr;
    }


};



int main() {
    // func = bulid_in_func;
    push_bulid_in_func_in_defined_table(); // push bulid_in_func to defined_table
    LexicalAnalyzer Lexical; //詞法分析器
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
        bool encounter_eof;
        bool buffer_remain;
        try {
            cout << "\n> ";
            errorType E = Error_None;
            Lexical.Get_Token(finish_input, E);

            if (finish_input) {
                // cerr << "\033[1;32mfinish_input\033[0m" << endl;
                // bulid parser tree
                // cerr << "\033[1;34menter build_tree\033[0m" << endl;
                Syntax.build_tree(Lexical.tokenBuffer);
                Syntax.set_root();
                // cerr << "\033[1;34mend build_tree\033[0m" << endl;
                // Syntax.print(); // !Debug

                // cerr << "\033[1;34menter execute\033[0m" << endl;
                FunctionExecutor func_executor;
                // Node_Token* result = nullptr;
                unordered_map<string, Node_Token*> local_defined_table;
                Node_Token* result = func_executor.evalution(Syntax.get_root(), E, local_defined_table);

                // cerr << "\033[1;34mend execute\033[0m" << endl;

                Syntax.print(result);
                cout << endl; // Print the result of the evaluation

                Lexical.reset(); // reset lexical vector
            }

        } catch (Error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    // cerr << "\033[1;31m" << "UNEXPECTED_TOKEN" << "\033[0m" << endl;
                    cout << e.message << endl ;
                    Lexical.reset();
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    // cerr << "\033[1;31m" << "UNEXPECTED_CLOSE_PAREN" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_END_PAREN:
                    // cerr << "\033[1;31m" << "UNEXPECTED_END_PAREN" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_STRING:
                    // cerr << "\033[1;31m" << "UNEXPECTED_STRING" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EOF:
                    // cerr << "\033[1;31m" << "UNEXPECTED_EOF" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EXIT:
                    // cerr << "\033[1;31m" << "UNEXPECTED_EXIT" << "\033[0m" << endl;
                    cout << endl;
                    Lexical.reset();
                    is_exit = true;
                    break;
                
                case incorrect_number_of_arguments:
                    // cerr << "\033[1;31m" << "incorrect_number_of_arguments" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case incorrect_argument_type:
                    // cerr << "\033[1;31m" << "incorrect_argument_type" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case undefined_function:
                    // cerr << "\033[1;31m" << "undefined_function" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case unbound_symbol:
                    // cerr << "\033[1;31m" << "unbound_symbol" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case unbound_parameter:
                    // cerr << "\033[1;31m" << "unbound_parameter" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case no_return_value:
                    // cerr << "\033[1;31m" << "no_return_value" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case no_return_value_inernal:
                    // cerr << "\033[1;31m" << "no_return_value_inernal" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case unbound_test_condition:
                    // cerr << "\033[1;31m" << "unbound_test_condition" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case unbound_condition:
                    // cerr << "\033[1;31m" << "unbound_condition" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case division_by_zero:
                    // cerr << "\033[1;31m" << "division_by_zero" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case non_list:
                    // cerr << "\033[1;31m" << "non_list" << "\033[0m" << endl;
                    cout << e.message;
                    
                    // Syntax.print(e.get_sub_error_tree());
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                case defined:
                    // cerr << "\033[1;31m" << "defined" << "\033[0m" << endl;
                    if (verbose_mode)
                        cout << e.message << endl;
                    Lexical.reset();
                    break;
                case error_level_define:
                    // cerr << "\033[1;31m" << "error_level_define" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case error_level_cleaned:
                    // cerr << "\033[1;31m" << "error_level_cleaned" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case error_level_exit:
                    // cerr << "\033[1;31m" << "error_level_exit" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case error_define_format:
                    // cerr << "\033[1;31m" << "error_define_format" << "\033[0m" << endl;
                    cout << e.message;
                    Syntax.print(e.get_sub_error_tree());
                    cout << "\n";
                    Lexical.reset();
                    break;
                
                case cleaned:
                    // cerr << "\033[1;31m" << "cleaned" << "\033[0m" << endl;
                    if (verbose_mode)
                        cout << e.message << endl;
                    push_bulid_in_func_in_defined_table();
                    Lexical.reset();
                    break;
                
                default:
                    break;
            }
        }
        
    }
        
    cout << "Thanks for using OurScheme!\n";
    fflush(stdout);
    clear_pointer_gather();
    return 0;
}
