#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum TokenType {
    INT,
    FLAOT,
    STRING,

    FUNCTION,
    PARAMETER,

    OPEN_PAREN, // (
    CLOSE_PAREN, // )
    QUOTE, // '
    DOT, // .
    NIL, // ()

    SYMBOL,

    BACKQUOTE,
    COMMA,
    COMMA_AT
};

// enum ExpressionType {
//     ATOM,
//     LIST,
//     NIL
// };

struct Token {
    TokenType type;
    string value;
    int level; // layer, 控制輸出前空格數量，紀錄括號深度
    Token *next;
    Token *prev;
};

struct Function {
    TokenType type;
    string value;

    /* data */
    Function() {
        type = FUNCTION;
    }
};

struct Parameter {
    TokenType type;
    string value;

    /* data */
    Parameter() {
        type = PARAMETER;
    }
};




enum errorType {
    UNEXPECTED_TOKEN // atom or '(' expected
    /*
    UNEXPECTED_EOF,
    UNEXPECTED_CLOSE_PAREN,
    UNEXPECTED_DOT,
    UNEXPECTED_QUOTE,
    UNEXPECTED_BACKQUOTE,
    UNEXPECTED_COMMA,
    UNEXPECTED_COMMA_AT,
    UNEXPECTED_SYMBOL,
    UNEXPECTED_INT,
    UNEXPECTED_FLOAT,
    UNEXPECTED_STRING,
    UNEXPECTED_OPEN_PAREN,
    UNEXPECTED_NIL,
    UNEXPECTED_LIST,
    UNEXPECTED_ATOM,
    UNEXPECTED_FUNCTION,
    UNEXPECTED_PARAMETER,
    UNEXPECTED_EXPRESSION_TYPE
    */
};

/* error message 
> ERROR (unexpected token) : atom or '(' expected when token at Line 1 Column 1 is >>)<<
*/
struct error {
    errorType type;
    string message;
    int line;
    int column;
    string expected;
};

bool isBalanced(const string& input) {
    int depth = 0;
    for (char c : input) {
        if (c == '(')
            ++depth;
        else if (c == ')')
            --depth;
    }
    return depth == 0;
}

vector<string> tokenize(const string& input) {
    vector<string> tokens;
    string token;
    for (char c : input) {
        if (isspace(c)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else if (c == '(' || c == ')' || c == '.') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            tokens.push_back(string(1, c));
        } else {
            token.push_back(c);
        }
    }
    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

/*
 in order to parse the input, build a parser tree

*/
class Parser_Tree {
private:
    Token *root;

public:
    Parser_Tree() {
        root = nullptr;
    }
    ~Parser_Tree() {
        delete root;
    }
    void insert(Token *t) {
        if (root == nullptr) {
            root = t;
        } else {
            Token *temp = root;
            while (temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = t;
            t->prev = temp;
        }
    }
    void print() {
        Token *temp = root;
        while (temp != nullptr) {
            cout << temp->value << endl;
            temp = temp->next;
        }
    }

};

/*
 recursive descent parser
 遞迴分解 parser
 
*/
void cut_token(Token *t, string input) {
    Token *temp = t;
    while (temp != nullptr) {
        cout << temp->value << endl;
        temp = temp->next;
    }
}

bool is_eof(string& input) {
    if (!getline(cin, input)) {
        cerr << "ERROR (no more input) : END-OF-FILE encountered" << endl;
        return true; // end of file
    }
    return false; 
}

int main() {
    cout << "Welcome to OurScheme!" << endl << endl;
    cout << "> ";
    
    string expr, input;
    int line, column = 0;
    Token *head = nullptr;
    Token *tail = nullptr;

    while (true) {
        /*
        if (!getline(cin, input)) {
            cerr << "ERROR (no more input) : END-OF-FILE encountered" << endl;
            break;
        }
        */
        if (is_eof(input))
           break;
        cout << "\033[1;33m" << "line:" << input << "\033[0m" << endl;
        
        
        for (char c : input) {
            cut
            if (c == '(') {
                Token *t = new Token;
                t->type = OPEN_PAREN;
                t->value = "(";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == ')') {
                Token *t = new Token;
                t->type = CLOSE_PAREN;
                t->value = ")";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == '.') {
                Token *t = new Token;
                t->type = DOT;
                t->value = ".";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == '\'') {
                Token *t = new Token;
                t->type = QUOTE;
                t->value = "'";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == '`') {
                Token *t = new Token;
                t->type = BACKQUOTE;
                t->value = "`";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == ',') {
                Token *t = new Token;
                t->type = COMMA;
                t->value = ",";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == '@') {
                Token *t = new Token;
                t->type = COMMA_AT;
                t->value = "@";
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            } else if (c == ' ') {
                continue;
            } else {
                Token *t = new Token;
                t->type = SYMBOL;
                t->value = c;
                t->level = 0;
                t->next = nullptr;
                t->prev = nullptr;
                if (head == nullptr) {
                    head = t;
                    tail = t;
                } else {
                    tail->next = t;
                    t->prev = tail;
                    tail = t;
                }
            }

        cut_token(head);

        /*expr += input + "\n";
        if (expr.find("(exit)") != string::npos)
            break;
        if (isBalanced(expr) && !expr.empty()) {
            auto tokens = tokenize(expr);
            for (auto& t : tokens) {
                cout << t << endl;
            }
            expr.clear();
            cout << "> ";
        }*/
    }
    
    cout << endl << "Thanks for using OurScheme!" << endl;
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