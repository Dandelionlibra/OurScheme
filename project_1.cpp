#include <iostream>
#include <string>
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
    INT, // accept +&-
    FLAOT, // .3f
    STRING, // need use "", cannot separate by line

    // FUNCTION,
    // PARAMETER,
    Left_PAREN, // ( begin of list
    Right_PAREN, // ) end of list

    DOT, // . dot and dot pair
    NIL, // ();false; null
    T, // true
    QUOTE, // '
    SYMBOL // 以 identifier 表示的資料，變數名稱, a type of atom
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
    int level; // layer, 控制輸出前空格數量，紀錄括號深度
};

struct Node_Token {
    Token *token;
    Node_Token *next;
    Node_Token *prev;
    Node_Token() : token(nullptr), next(nullptr), prev(nullptr) {}
};

// ! 未使用
struct Function {
    TokenType type;
    string value;
};
struct Parameter {
    TokenType type;
    string value;
};
// ! 未使用

enum errorType {
    // ERROR (unexpected token) : atom or '(' expected when token at Line X Column Y is >>...<<
    UNEXPECTED_TOKEN, // atom or '(' expected

    // ERROR (unexpected token) : ')' expected when token at Line X Column Y is >>...<<
    UNEXPECTED_CLOSE_PAREN, // unexpected close parenthesis error, error (1 2 . 3 4) , correct is (1 2 . (3 4))

    // ERROR (no closing quote) : END-OF-LINE encountered at Line X Column Y
    UNEXPECTED_STRING, // unexpected string error ("hello)

    //ERROR (no more input) : END-OF-FILE encountered
    UNEXPECTED_EOF, // END-OF-FILE encountered
    
    // UNEXPECTED_DOT // unexpected dot error (1 2 . 3 4) , correct is (1 2 . (3 4))
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

class LexicalAnalyzer {
private:

    Node_Token root;
    Node_Token tail;
    int g_line = 1, g_column = 0;
    int leftCount = 0, rightCount = 0;
    bool is_EOF;
    string Str;

    bool is_space(char token) {
        if (token == ' ' || token == '\t')
            return true;
        return false;
    }

    bool is_enter(char token) {
        if (token == '\n', token == '\r')
            return true;
        return false;
    }

    void judge_token(string tmp_string) {

    }

    void set_EOF(bool parameter) {
        is_EOF = parameter;
    }


public:
    LexicalAnalyzer() : is_EOF(false), Str("") {
        root.token = nullptr;
        tail.token = nullptr;
    }

    char GetChar() {
        char c;
        if (cin.get(c)) {
            g_column++;
            if (is_enter(c)) {
                g_line++;
                g_column = 0;
            }
            else if (is_space(c)) {
                cout << "\033[1;33m" << "meet space" << "\033[0m" << endl;
            }
            cout << "Read char: " << c << " (line " << g_line << ", col " << g_column << ")" << endl;
            return c;
        }
        else {
            set_EOF(true);
        }
        return '\0';
    }

    void GetStr() {
        string tokenBuffer;
        while (!Get_is_EOF() || leftCount != rightCount) {
            char c = GetChar();
            //! if (c == '\0') break;

            if (is_space(c) || is_enter(c)) {
                if (!tokenBuffer.empty()) {
                    if (!root.token) {
                        root.token = new Token();
                        root.token->value = tokenBuffer;
                        tail = root;
                    } else {
                        Node_Token* newNode = new Node_Token();
                        newNode->token = new Token();
                        newNode->token->value = tokenBuffer;
                        newNode->prev = &tail;
                        tail.next = newNode;
                        tail = *newNode;
                    }
                    Str.append(tokenBuffer);
                    tokenBuffer.clear();
                }
            }
            else
                tokenBuffer.push_back(c);
            
            /*
            if (c == '(') leftCount++;
            else if (c == ')') rightCount++;

            if (leftCount == rightCount && leftCount != 0) {
                if (!tokenBuffer.empty()) {
                    if (!root.token) {
                        root.token = new Token();
                        root.token->value = tokenBuffer;
                        tail = root;
                    } else {
                        Node_Token* newNode = new Node_Token();
                        newNode->token = new Token();
                        newNode->token->value = tokenBuffer;
                        newNode->prev = &tail;
                        tail.next = newNode;
                        tail = *newNode;
                    }
                    tokenBuffer.clear();
                }
                break;
            }*/
        }
    }

    void CutToken(string input) {
        string tmp_string = "";
        int size = input.size();
        for(int i = 0, start = 0 ; i < size ; i++) {
            Token tmp_token;
            if (is_space(input.at(i)) || is_enter(input.at(i))) { // " "、"\n"、"\t""
                cout << "enter space judge" << endl;
                tmp_string = tmp_string.substr(start, i);
                cout << "\033[1;31m" << "tmp_string: " << tmp_string << "\033[0m" << endl;

                if (tmp_string != "") judge_token(tmp_string);
                tmp_string = "" ;
                

            }
            else {
                tmp_string = tmp_string + input.at(i);

            } 
            cout << "input.at(i): " << input.at(i) << endl;

        }
    }

    void GetToken(Token *t, string input) {
        Token *temp = t;

    }

    bool Get_is_EOF(){
        return is_EOF;
    }


    ~LexicalAnalyzer() {

    }
};

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

        }
    }
    void print() {
        Token *temp = root;

    }

};



/*
 recursive descent parser
 遞迴分解 parser
 
*/


// bool is_eof(string& input) {
//     if (!getline(cin, input)) {
//         // cerr << "ERROR (no more input) : END-OF-FILE encountered" << endl;
//         return true; // end of file
//     }
//     return false; 
// }

int main() {
    LexicalAnalyzer Lexical; //詞法分析器
    bool is_Syntax_legal = true;
    // SyntaxAnalyzer Syntax; //語法分析器

    // bool is_EOF = false;
    string expr, input;
    int line, column = 0;
    Node_Token *head = nullptr;
    Node_Token *tail = nullptr;

    cout << "Welcome to OurScheme!" << endl << endl;
    while (!Lexical.Get_is_EOF()) { // while (true)
        try {
            cout << "> ";
            Lexical.GetStr();
            if (Lexical.Get_is_EOF()) { // ReadSExp
                // is_EOF = true;
                throw error{UNEXPECTED_EOF};
                // cout << "ERROR (no more input) : END-OF-FILE encountered" << endl;
                // continue;
                break;
            }
            cout << "\033[1;33m" << "input:" << input << "\033[0m" << endl;
            

            if (input == "(exit)") {
                break;
            }

            // Lexical.CutToken(input);



        } catch (error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    cout << "ERROR (unexpected token) : " << e.message << " when token at Line " << e.line << " Column " << e.column << " is >>" << e.expected << "<< " << endl;
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    cout << "ERROR (unexpected token) : " << e.message << " when token at Line " << e.line << " Column " << e.column << " is >>" << e.expected << "<< " << endl;
                    break;
                case UNEXPECTED_STRING:
                    cout << "ERROR (no closing quote) : " << e.message << " at Line " << e.line << " Column " << e.column << endl;
                    break;
                case UNEXPECTED_EOF:
                    cout << "ERROR (no more input) : END-OF-FILE encountered" << endl;
                    break;
                default:
                    break;
            }
        }
        
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