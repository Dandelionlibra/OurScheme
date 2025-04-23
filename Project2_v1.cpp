#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
using namespace std;

const int No_Limit = -1;
const int Bigger_ThanTwo = 999;
const int Bigger_ThanOne = 99;
const int Two_Or_Three = 23;

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
    undefined_function = 555,
    unbound_symbol = 666,
    division_by_zero = 0,
    non_list = 777,



    Error_None = 99999
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    int level;
    bool is_function = false;

    Token() : type(SYMBOL), value(""), line(0), column(0), level(0) {}
};

struct Node_Token {
    Token token; // token info, value, type

    Node_Token *parent;
    Node_Token *left;
    Node_Token *right;
    Node_Token() : parent(nullptr), left(nullptr), right(nullptr) {}
};

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
        // expected = e_token;
        // current = c_token;
        line = l;
        column = c;
        root = r;

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
        else if (t == undefined_function)
            message = "ERROR (attempt to apply non-function) : " + e_token;
        else if (t == unbound_symbol)
            message = "ERROR (unbound symbol) : " + e_token;
        else if (t == division_by_zero)
            message = "ERROR (division by zero) : " + e_token;
        else if (t == non_list)
            message = "ERROR (non-list) : ";
        
        

    }
    Node_Token * get_sub_error_tree() {
        return root;
    }
};


struct variable {
    string name;
    TokenType type;
};

set<string> bulid_in_func = { // only bulid-in function
    "define", "cons", "lambda", "list",
    "car", "cdr",
    "atom?", "pair?", "list?", "null?", "integer?", "real?", "number?", "string?", "boolean", "symbol?",
    "+", "-", "*", "/",
    "not", "and", "or",
    "eqv?", "equal?",
    "=", "<", ">", "<=", ">=",
    "string-append", "string>?", "string<?", "string=?",
    "begin", "if", "cond",
    "clean-environment"
};
set <string> func; // all function name, unclude self-defined function
unordered_map<string, Node_Token*> vars_correspond_table; // store the variable name and value

// void test(string str_index, string str, TokenType type) {
//     variable var = {str, type};
//     vars_correspond_table.insert({str_index, var});
// }

class FunctionExecutor {
    private:
    bool float_flag = false;

    int count_args(Node_Token *args) {
        int count = 0;
        while (args->left != nullptr) {
            args = args->right;
        }
        return count;
    }

    public:
    Node_Token* apply_function(string& func_name, vector<Node_Token*>& args, errorType& e){

    };

    Node_Token* define_func(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 2)
            throw Error(incorrect_number_of_arguments, "define", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement define function
        }
        return nullptr;
    }

    Node_Token* cons(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 2)
            throw Error(incorrect_number_of_arguments, "cons", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement cons function
        }
        return nullptr;
    }
    Node_Token* lambda(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 2)
            throw Error(incorrect_number_of_arguments, "lambda", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement lambda function
        }
        return nullptr;
    }
    Node_Token* list(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) < 1)
            throw Error(incorrect_number_of_arguments, "list", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement list function
        }
        return nullptr;
    }
    Node_Token* car(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "car", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement car function
        }
        return nullptr;
    }
    Node_Token* cdr(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "cdr", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement cdr function
        }
        return nullptr;
    }
    
    Node_Token* is_atom(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "atom?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement atom? function
        }
        return nullptr;
    }
    Node_Token* is_pair(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "pair?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement pair? function
        }
        return nullptr;
    }
    Node_Token* is_list(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "list?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement list? function
        }
        return nullptr;
    }
    Node_Token* is_null(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "null?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement null? function
        }
        return nullptr;
    }
    Node_Token* is_integer(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "integer?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement integer? function
        }
        return nullptr;
    }
    Node_Token* is_real(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "real?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement real? function
        }
        return nullptr;
    }
    Node_Token* is_number(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "number?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement number? function
        }
        return nullptr;
    }
    Node_Token* is_string(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "string?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement string? function
        }
        return nullptr;
    }
    Node_Token* is_boolean(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "boolean?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement boolean? function
        }
        return nullptr;
    }
    Node_Token* is_symbol(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 1)
            throw Error(incorrect_number_of_arguments, "symbol?", "incorrect_number_of_arguments", 0, 0);
        else {
            // implement symbol? function
        }
        return nullptr;
    }
    Node_Token* plus(Node_Token *cur, Node_Token *args, errorType &e) {
        Node_Token* node = new Node_Token();
        vector<Node_Token*> arg_list;

        if (count_args(args) < 2)
            throw Error(incorrect_number_of_arguments, "+", "incorrect_number_of_arguments", 0, 0);
        else { // args >= 2
            Node_Token *t = args;
            while (t != nullptr) {
                Node_Token *parameter = 
                
                t->left;
                
                // arg_list.push_back();

                t = t->right;
            }
            
        }

        
    }

    Node_Token* minus(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) < 2)
            throw Error(incorrect_number_of_arguments, "-", "incorrect_number_of_arguments", 0, 0);

    }

    Node_Token* multiply(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) < 2)
            throw Error(incorrect_number_of_arguments, "*", "incorrect_number_of_arguments", 0, 0);
    }

    Node_Token* division(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) < 2)
            throw Error(incorrect_number_of_arguments, "/", "incorrect_number_of_arguments", 0, 0);
    }




    Node_Token* clean_environment(Node_Token *cur, Node_Token *args, errorType &e) {
        if (count_args(args) != 0)
            throw Error(incorrect_number_of_arguments, "clean-environment", "incorrect_number_of_arguments", 0, 0);
        else {
            func.clear();
        }
        return nullptr;
    }

};

FunctionExecutor global_func_executor;

vector<Node_Token*> define_trees;
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
                Node_Token *left_quote = new Node_Token;
                // current_token.value = "quote";
                left_quote = set_node(current_token, nullptr, nullptr, node);
                node->left = left_quote;

                Node_Token *right_quote = new Node_Token;
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
        tokens = tokenBuffer;
        // cerr << "\033[1;34menter build_AST\033[0m" << endl;
        int index = 0;

        if (!tokens.empty())
            root = insert(root, tokens, index);
        
        define_trees.push_back(root);
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
        clear_tree(root);
        tokens.clear();
    }
};

/**/
class Function {
    private:
    // template <typename T>
    Token implement_arithmetic(string name, vector<Node_Token*> &args) {
        if (args.size() < 2)
            throw Error(incorrect_number_of_arguments, name, "incorrect_number_of_arguments", 0, 0);


        Token result;
        int cal_val;
        double cal_val_f;
        if (args.at(0)->token.type == INT)
            result.type = INT;
        else if (args.at(0)->token.type == FLOAT)
            result.type = FLOAT;
        else {
            // ! > ERROR (unbound symbol) : drg
        }

        if (name == "add") {
            // implement add function
            for (int i = 0 ; i < args.size(); i++) {
                if (args.at(i)->token.type == INT)
                    cal_val += stoi(args.at(i)->token.value);
                else if (args.at(i)->token.type == FLOAT)
                    cal_val_f += stod(args.at(i)->token.value);
            }
            
        }
        else if (name == "sub") {
            // implement sub function
            if (args.at(0)->token.type == INT)
                cal_val = stoi(args.at(0)->token.value);
            else if (args.at(0)->token.type == FLOAT)
                cal_val_f = stod(args.at(0)->token.value);
            
            for (int i = 1 ; i < args.size(); i++) {
                if (args.at(i)->token.type == INT)
                    cal_val -= stoi(args.at(i)->token.value);
                else if (args.at(i)->token.type == FLOAT)
                    cal_val_f -= stod(args.at(i)->token.value);
            }
        }
        else if (name == "mul") {
            // implement mul function
            if (args.at(0)->token.type == INT)
                cal_val = stoi(args.at(0)->token.value);
            else if (args.at(0)->token.type == FLOAT)
                cal_val = stod(args.at(0)->token.value);
            
            for (int i = 1 ; i < args.size(); i++) {
                if (args.at(i)->token.type == INT)
                    cal_val *= stoi(args.at(i)->token.value);
                else if (args.at(i)->token.type == FLOAT)
                    cal_val_f *= stod(args.at(i)->token.value);
            }
        }
        else if (name == "div") {
            // implement div function
            if (args.at(0)->token.type == INT)
                cal_val = stoi(args.at(0)->token.value);
            else if (args.at(0)->token.type == FLOAT)
                cal_val_f = stod(args.at(0)->token.value);

            for (int i = 1 ; i < args.size(); i++) {
                if (args.at(i)->token.value == "0" || args.at(i)->token.value == "0.0") {
                    cerr << "\033[1;31m" << args.at(i)->token.value << "\033[0m" << endl;
                    throw Error(division_by_zero, name, "division_by_zero", 0, 0);
                }

                if (args.at(i)->token.type == INT)
                    cal_val /= stoi(args.at(i)->token.value);
                else if (args.at(i)->token.type == FLOAT)
                    cal_val_f /= stod(args.at(i)->token.value);
            }
        }
        if (result.type == INT)
            result.value = to_string(cal_val);
        else if (result.type == FLOAT)
            result.value = to_string(cal_val_f);
        else
            cerr << "\033[1;31m" << "error in arithmetic function" << "\033[0m" << endl;
        return result;
    }
    /*
    // template <typename T>
    Token implement_logic(string name, vector<Node_Token*> &args) {
        if (args.size() < 2) {
            throw Error(incorrect_number_of_arguments, name, "incorrect_number_of_arguments", 0, 0);
        }

        Token result;
        result.type = TokenType::T;
        result.value = "#t";
        if (name == "not") {
            // implement not function
            if (args.size() != 1)
                throw Error(incorrect_number_of_arguments, name, "incorrect_number_of_arguments", 0, 0);
            if (args.at(0).type != TokenType::T){
                result.type = T;
                result.value = "#t";
            }
            else {
                result.type = NIL;
                result.value = "#f";
            }
        }
        // 'and'  第一個被計算為 nil 則回傳 nil，如果不是 nil 則回傳最後一個計算出的值
        else if (name == "and") {
            // implement and function
            for (int i = 0 ; i < args.size(); i++) {
                if (args.at(i).type == NIL) {
                    result = args.at(i); // NIL
                    break;
                }
                else if (i == args.size()-1) 
                    result = args.at(i);
            }
        }
        // 'or' 第一個被計算為非 nil 則直接回傳該值，如果都是 nil 則回傳最後一個計算出的值
        else if (name == "or") {
            // implement or function
            for (int i = 0 ; i < args.size(); i++) {
                if (args.at(i).type != NIL) {
                    result = args.at(i);
                    break;
                }
                else if (i == args.size()-1) 
                    result = args.at(i);
            }
        }
        // 全部相等回傳 T，否則 nil
        else if (name == "equal") {
            // Token nil_or_T_token.type = TokenType::T;
            for (int i = 1; i < args.size(); i++) {
                if (args.at(i).type != args.at(0).type && args.at(i).value != args.at(0).value) {
                    result.type = NIL;
                    result.value = "#f";
                    break;
                }
            }
        }
        else if (name == "less") {
            for (int i = 0; i < args.size()-1; i++){
                if (args.at(0).type == INT) {
                    int a = stoi(args.at(i).value);
                    int b = stoi(args.at(1+1).value);
                    if (a >= b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else if (args.at(0).type == FLOAT) {
                    double a = stod(args.at(i).value);
                    double b = stod(args.at(1+1).value);
                    if (a >= b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else cerr << "\033[1;31m" << "error in logic function" << "\033[0m" << endl;
            }
        }
        else if (name == "greater") {
            for (int i = 0; i < args.size()-1; i++){
                if (args.at(0).type == INT) {
                    int a = stoi(args.at(i).value);
                    int b = stoi(args.at(1+1).value);
                    if (a <= b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else if (args.at(0).type == FLOAT) {
                    double a = stod(args.at(i).value);
                    double b = stod(args.at(1+1).value);
                    if (a <= b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else cerr << "\033[1;31m" << "error in logic function" << "\033[0m" << endl;
            }
        }
        else if (name == "less_equal") {
            for (int i = 0; i < args.size()-1; i++){
                if (args.at(0).type == INT) {
                    int a = stoi(args.at(i).value);
                    int b = stoi(args.at(1+1).value);
                    if (a > b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else if (args.at(0).type == FLOAT) {
                    double a = stod(args.at(i).value);
                    double b = stod(args.at(1+1).value);
                    if (a > b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else cerr << "\033[1;31m" << "error in logic function" << "\033[0m" << endl;
            }
        }
        else if (name == "greater_equal") {
            for (int i = 0; i < args.size()-1; i++){
                if (args.at(0).type == INT) {
                    int a = stoi(args.at(i).value);
                    int b = stoi(args.at(1+1).value);
                    if (a < b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else if (args.at(0).type == FLOAT) {
                    double a = stod(args.at(i).value);
                    double b = stod(args.at(1+1).value);
                    if (a < b) {
                        result.type = NIL;
                        result.value = "#f";
                        break;
                    }
                }
                else cerr << "\033[1;31m" << "error in logic function" << "\033[0m" << endl;
            }
        }

        return result;
    }

    Token implement_same(string name, vector<Node_Token*> &args) {
        if (args.size() != 2)
            throw Error(incorrect_number_of_arguments, name, "incorrect_number_of_arguments", 0, 0);

        Token result;
        result.type = NIL;
        result.value = "#f";
        if (name == "eqv?") {
            // implement eqv function
            if (args.at(0).type != args.at(1).type) {
                result.type = NIL;
                result.value = "#f";
            }
        }
        else if (name == "equal?") {
            // implement equal function
            if (args.at(0).value != args.at(1).value) {
                result.type = NIL;
                result.value = "#f";
            }
        }
    }
    */
    
    public:
    string func_name;
    int arg_num;
    vector<Token> args;
    vector<Token> body;

    Function() : func_name(""), arg_num(0) {}

    Function(string n) {
        func_name = n;
        arg_num = No_Limit;
    }
    Function(string n, int num) {
        func_name = n;
        arg_num = num;
    }


    // template <class T>
    void implement(string name, vector<Node_Token*> &args) {
        AST_Tree tree;
        Token result;
        // (+ 4  5)、(+ 5(+ 4  5))
        if (name == "add" || name == "sub" || name == "mul" || name == "div") {
            result = implement_arithmetic(name, args);
        }
        else if (name == "not" || name == "and" || name == "or" || name == "equal" || name == "less" || name == "greater" || name == "less_equal" || name == "greater_equal" ) {
            // implement_logic(name, args);
        }
        else if (name == "string-append") {
            // string-append
        }
        else if (name == "string>?" || name == "string<?" || name == "string=?") {
            // string>?, string<? , string=?
        }
        else if (name == "eqv?" || name == "equal?") {
            // eqv? or equal?
            // implement_same(name, args);
        }
        else if (name == "begin") {
            // begin
        }

        else if (name == "if") {
            // if
        }
        else if (name == "cond") {
            // cond
        }
        else if (name == "define") {
            // define
        }
    }
};

map <string, Function> Reserved_function_map = {
    {"define", Function("define", 2)},
    {"cons", Function("cons", 2)},
    {"list", Function("list")},

    {"car", Function("car", 1)},
    {"cdr", Function("cdr", 1)},

    {"atom?", Function("atom?", 1)},
    {"pair?", Function("pair?", 1)},
    {"list?", Function("list?", 1)},
    {"null?", Function("null?", 1)},
    {"integer?", Function("integer?", 1)},
    {"real?", Function("real?", 1)},
    {"number?", Function("number?", 1)},
    {"string?", Function("string?", 1)},
    {"boolean?", Function("boolean?", 1)},
    {"symbol?", Function("symbol?", 1)},

    {"+", Function("add", Bigger_ThanTwo)}, // args >= 2
    {"-", Function("sub", Bigger_ThanTwo)}, // args >= 2
    {"*", Function("mul", Bigger_ThanTwo)}, // args >= 2
    {"/", Function("div", Bigger_ThanTwo)}, // args >= 2

    {"not", Function("not", 1)},
    {"and", Function("and", Bigger_ThanTwo)}, // args >= 2
    {"or", Function("or", Bigger_ThanTwo)}, // args >= 2
    {"=", Function("equal", Bigger_ThanTwo)}, // args >= 2
    {"<", Function("less", Bigger_ThanTwo)}, // args >= 2
    {">", Function("greater", Bigger_ThanTwo)}, // args >= 2
    {"<=", Function("less_equal", Bigger_ThanTwo)}, // args >= 2
    {">=", Function("greater_equal", Bigger_ThanTwo)}, // args >= 2

    {"string-append", Function("string-append", Bigger_ThanTwo)}, // args >= 2
    {"string>?", Function("string_greater", Bigger_ThanTwo)}, // args >= 2
    {"string<?", Function("string_less", Bigger_ThanTwo)}, // args >= 2
    {"string=?", Function("string_equal", Bigger_ThanTwo)}, // args >= 2
    
    {"eqv?", Function("eqv?", 2)}, // qv? 只有當兩個物件都是 atoms(除了 strings)，或兩個物件記憶體位址相同才回傳 true
    {"equal?", Function("equal?", 2)}, // ; equal? 為等於的一般概念

    {"begin", Function("begin", Bigger_ThanOne)}, // (>= 1)
    {"if", Function("if", Two_Or_Three)}, // (>= 2)
    {"cond", Function("cond", Bigger_ThanOne)},
    {"clean-environment", Function("clean-environment", 0)}

};


class SyntaxAnalyzer {
private:
    AST_Tree tree;
    Node_Token *root;
    
    // int currnt_level = 0;
    
    vector <pair<Token, bool>> dot_appear; // first: pointer to ( , second: (dot appear or not, following data appear or not)

    void process_float(Token &token) {
        string str = token.value;
        string sign = "";

        if (token.value.at(0) == '+' || token.value.at(0) == '-') {
            sign = token.value.at(0); // save the sign
            str = token.value.substr(1); // Remove the sign from the string
        }


        // 四捨五入到小數點後第三位
        float float_value = stod(str);
        cerr << "float_value: " << float_value << endl;
        float_value = round(float_value * 1000.0) / 1000.0;
        str = to_string(float_value);

        // 小數點後大於三位數, 只取到小數點後第三位
        int dot_pos = str.find('.');
        if (str.size() - dot_pos > 4)
            str = str.substr(0, dot_pos + 4);

        if (sign == "-")
            str = sign + str;

        token.value = str;
    }
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
                cout << "quote" << endl;//
            }
            else if (left_token.type == DOT) {
                // cerr << "\033[1;35m" << "** inner DOT **" << "\033[0m" << endl;
                
                pretty_print(node->left, countquote);
                
            }
            else if (left_token.type == NIL) { // (root == node && current_token.type == NIL) ||
                // cerr << "\033[1;35m" << "** NIL **" << "\033[0m" << endl;
                // print_space(countquote);
                cout << "nil" << endl;//

            }
            else { // ATOM
                // cerr << "\033[1;35m" << "** 1.ATOM **" << "\033[0m" << endl;
                // deal float
                if (left_token.type == FLOAT)
                    process_float(left_token);
                
                // print_space(countquote);
                cout << left_token.value << endl;
                
            }
            Node_Token *tmp = node->right;
            // cerr << "** begin while **" <<endl;
            while (tmp != nullptr) {
                if (tmp->token.type == DOT && tmp->left->token.type == DOT) { //  && tmp->right->token.type == DOT
                    // cerr << "\033[1;35m" << "** while DOT 1 **" << "\033[0m" << endl;
                    print_space(countquote);
                    pretty_print(tmp->left, countquote);
                }
                else if (tmp->token.type == DOT) { // left_token.type != DOT
                    // cerr << "\033[1;35m" << "** while DOT 2 **" << "\033[0m" << endl;
                    // if (tmp->left->token.type != NIL) {
                        // deal float
                        if (tmp->left->token.type == FLOAT)
                            process_float(tmp->left->token);
        
                        print_space(countquote);
                        if (tmp->left->token.type == NIL) {
                            cout << "nil" << endl;
                        }
                        else
                            cout << tmp->left->token.value << endl;
                        
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
            print_space(countquote);
            cout << ")" << endl;
            
        }
        else { //ATOM, leaf node
            // cerr << "\033[1;35m" << "** 2.ATOM **" << "\033[0m" << endl;
            if (current_token.type != NIL) {
                // deal float
                if (current_token.type == FLOAT)
                    process_float(current_token);

                print_space(countquote);
                // if (current_token.value == "#f")
                //     cout << "nil" << endl;
                // else
                    cout << current_token.value << endl;
            }


        }


    }


public:
    SyntaxAnalyzer() {
        // currnt_level = 0;
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
                // if (check_dot_appear(tokenBuffer.at(index_curr).level) == true) 
                //     error = UNEXPECTED_END_PAREN;

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
                            // cerr << "\033[1;33m2. tokenBuffer.at(i).value: " << tokenBuffer.at(i).value << "\033[0m" << endl;
                            c++;
                        
                    }
                }
            }

            renew_dot_appear(tokenBuffer.at(index_curr).type, tokenBuffer.at(index_curr));
        }

        // cerr << "\033[1;34mend check_syntax\033[0m" << endl;
    }

    void build_tree(vector<Token> &v) {
        tree.build_AST(v);

        if (tree.check_exit())
            throw Error(UNEXPECTED_EXIT, "exit", "exit", 0, 0);
    }
    
    vector<Node_Token*> eval_arg_list(Node_Token* list, errorType &e, AST_Tree &sub_tree) {
        vector<Node_Token*> args;
        if (list == nullptr || list->token.value == "nil")
            return args; // Return an empty vector if the list is nil or null
    
        Node_Token* curr = list;
    
        while (curr != nullptr && curr->token.type == DOT) {
            Node_Token* orig_arg = curr->left;
            Node_Token* evaled_arg = eval(orig_arg, e);
            if (e != Error_None)
                return args; // Return the collected arguments so far if an error occurred
    
            if (orig_arg != evaled_arg) {
                sub_tree.clear_tree(orig_arg);
                curr->left = evaled_arg;
            }
    
            args.push_back(evaled_arg);
            curr = curr->right;
        }
    
        if (curr != nullptr && curr->token.value != "nil") {
            e = non_list;
            throw Error(non_list, "non-list", "non-list", curr->token.line, curr->token.column);
        }
    
        return args;
    }
    
    bool is_quote_nil(Node_Token *t) {
        if (t->left->token.type == QUOTE && t->right->token.type == DOT && t->right->left->token.type == NIL && t->right->right->token.type == NIL) {
            return true;
        }
        return false;
    }
    
    Node_Token* eval(Node_Token *cur, errorType &e) {
        if (cur == nullptr) return nullptr;

        FunctionExecutor func_executor;
        string func_name;
        vector<Node_Token*> args;
        Node_Token *node = nullptr;

        // Handle atoms
        if (is_ATOM(cur->token.type)) { //  != DOT &&  != QUOTE
            if (cur->token.type == SYMBOL) {
                // Check if the symbol is bound
                // if (t->token.is_function) {
                //     // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //     // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //     // t->token.value = "#<procedure "+ t->token.value +">";
                //     // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //     // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                // }
                if (vars_correspond_table.find(cur->token.value) == vars_correspond_table.end()){
                    e = unbound_symbol;
                    throw Error(unbound_symbol, cur->token.value, "unbound symbol", cur->token.line, cur->token.column);
                }
                else
                    return vars_correspond_table[cur->token.value]; // Return the bound symbol
                
            }
            return cur; // Return the atom itself
        }
        /*else if (e == Error_None && t->token.type == DOT && t->left->token.type == DOT) {
            // Handle dot notation
            if (e == Error_None && t->left->token.type == DOT) {
                e = unbound_symbol;
                throw Error(unbound_symbol, t->left->token.value, "unexpected token", t->left->token.line, t->left->token.column);
            }
            node = eval(t->left, e);
            return node; // Evaluate the left side of the dot
        }*/


        // judge pure list or not, the rightmost node must be NIL so that can continue
        // if (...) is not a (pure) list
        Node_Token *t = cur;
        while (t != nullptr && t->token.type != NIL) {
            t = t->right;
        }
        if (t == nullptr || t->token.type != NIL) {
            cerr << "\033[1;35m" << "--- pure list ---" << "\033[0m" << endl;
            e = non_list;
            throw Error(non_list, "non-list", "non-list", t->token.line, t->token.column);
        }

        // if first argument of (...) is an atom ☆, which is not a symbol
        // ERROR (attempt to apply non-function) : ☆
        Node_Token* func_Token = cur->left;
        if ( is_ATOM(func_Token->token.type) ){
            if (func_Token->token.type != SYMBOL) {
                e = undefined_function;
                throw Error(undefined_function, func_Token->token.value, func_Token->token.value ,func_Token->token.line, func_Token->token.column);
            }
            // else if (func_Token->token.value == "define")

            // else if (func_Token->token.value == "cons")

            // else if (func_Token->token.value == "lambda")
            
            // else if (func_Token->token.value == "list")

            // else if (func_Token->token.value == "car")

            // else if (func_Token->token.value == "cdr")

            else if (func_Token->token.value == "+")
                return func_executor.plus(cur, cur->right, e);
            else if (func_Token->token.value == "-")
                func_executor.minus(cur, cur->right, e);
            else if (func_Token->token.value == "*")
                func_executor.multiply(cur, cur->right, e);
            else if (func_Token->token.value == "/")
                func_executor.division(cur, cur->right, e);
            else if (func_Token->token.value == "not")
                func_executor.not_func(cur, cur->right, e);
            else if (func_Token->token.value == "and")

            else if (func_Token->token.value == "or")

            else if (func_Token->token.value == "=")

            else if (func_Token->token.value == "<")

            else if (func_Token->token.value == ">")

            else if (func_Token->token.value == "<=")

            else if (func_Token->token.value == ">=")

            else if (func_Token->token.value == "string-append")

            else if (func_Token->token.value == "string>?")

            else if (func_Token->token.value == "string<?")

            else if (func_Token->token.value == "string=?")

            else if (func_Token->token.value == "eqv?")
                func_executor.eqv(cur, cur->right, e);
            else if (func_Token->token.value == "equal?")
                func_executor.equal(cur, cur->right, e);
            else if (func_Token->token.value == "begin")
                func_executor.begin(cur, cur->right, e);
            else if (func_Token->token.value == "if")
                func_executor.if_func(cur, cur->right, e);
            else if (func_Token->token.value == "cond")
                func_executor.cond(cur, cur->right, e);

            else if (func_Token->token.value == "clean-environment")
                func_executor.clean_environment(cur, cur->right, e);
            else { // undefined function
                // cerr << "\033[1;33m" << "func_Token: " << func_Token->token.value << "\033[0m" << endl;
                func_name = func_Token->token.value;
                cerr << "\033[1;33m" << "func_name: " << func_name << "\033[0m" << endl;
                if (Reserved_function_map.find(func_name) == Reserved_function_map.end()) {
                    e = undefined_function;
                    throw Error(undefined_function, func_name, func_name ,func_Token->token.line, func_Token->token.column);
                }

            }

        }
        else if (cur->left->token.type == DOT) {
            
        }
        else if (cur->left->token.type == QUOTE) {

        }


        /*Node_Token* func_Token = eval(t->left, e); // Evaluate the function token
        func_name = func_Token->token.value;
        cerr << "\033[1;33mfunc_Token: " << func_Token->token.value << "\033[0m" << endl;

        if (e != Error_None)
            return t; // Return the original node if an error occurred
        
        Node_Token* arg_list = t->right;
        AST_Tree tree_util;
        args = eval_arg_list(arg_list, e, tree_util);// eval 參數，替換節點與釋放原本節點
        if (e != Error_None)
            return t;
        */
        
        return global_func_executor.apply_function(func_name, args, e);
    }

    void print() {
        // root = tree.get_root();
        if (root->token.type == NIL) {
            cout << "nil" << endl;
            return;
        }
        int countquote = 0;

        cerr << "\033[1;34menter pretty_print\033[0m" << endl;
        pretty_print(root, countquote);
        cerr << "\033[1;34mend pretty_print\033[0m" << endl;
    }
    void print(Node_Token * r) {
        // root = tree.get_root();
        if (r->token.type == NIL) {
            cout << "nil" << endl;
            return;
        }
        int countquote = 0;

        cerr << "\033[1;34menter error pretty_print\033[0m" << endl;
        pretty_print(r, countquote);
        cerr << "\033[1;34mend error pretty_print\033[0m" << endl;
    }

    ~SyntaxAnalyzer() {
        // tree.clear_tree(tree.get_root());
        root = nullptr;
        
        delete root;
        
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
                if ( func.find(tmpstr) != func.end() )
                    tmptoken.is_function = true;

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
                        // else {
                        //     break;
                        // }
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
                    start_column = 1;
        
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
    func = bulid_in_func;
    // test("a", "3", INT);
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
                Syntax.set_root();
                Syntax.print();

                cerr << "\033[1;34menter execute\033[0m" << endl;
                Syntax.eval(Syntax.get_root(), E);

                cerr << "\033[1;34mend execute\033[0m" << endl;


                Syntax.print();

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
                
                case incorrect_number_of_arguments:
                    cerr << "\033[1;31m" << "incorrect_number_of_arguments" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case undefined_function:
                    cerr << "\033[1;31m" << "undefined_function" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case unbound_symbol:
                    cerr << "\033[1;31m" << "unbound_symbol" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case division_by_zero:
                    cerr << "\033[1;31m" << "division_by_zero" << "\033[0m" << endl;
                    cout << e.message << endl;
                    Lexical.reset();
                    break;
                case non_list:
                    cerr << "\033[1;31m" << "non_list" << "\033[0m" << endl;
                    cout << e.message << endl;
                    
                    Syntax.print(e.get_sub_error_tree());
                    Lexical.reset();
                    break;
                
                

                default:
                    break;
            }
        }
        
    }
        
    cout << "Thanks for using OurScheme!";
    fflush(stdout);
    return 0;
}
