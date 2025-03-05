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
    INT, // accept +&-, e.g., '123', '+123', '-123'
    FLAOT, // '123.567', '123.', '.567', '+123.4', '-.123'
    STRING, // need use "", cannot separate by line

    Left_PAREN, // ( begin of list
    Right_PAREN, // ) end of list

    DOT, // . dot and dot pair
    NIL, // ();false; null
    T, // true
    QUOTE, // '
    SYMBOL // 以 identifier 表示的資料，變數名稱, a type of atom
           // a consecutive sequence of printable characters that are
           // not numbers, strings, #t or nil, and do not contain 
           // '(', ')', single-quote, double-quote, semi-colon and 
           // white-spaces ; 
           // Symbols are case-sensitive 
           // (i.e., uppercase and lowercase are different);
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
    Token *token;
    Node_Token *next;
    Node_Token *prev;
    Node_Token() : token(nullptr), next(nullptr), prev(nullptr) {}
};

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
    UNEXPECTED_EXIT, // (exit)
    Error_None
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


class LexicalAnalyzer {
private:


public:
    LexicalAnalyzer() {

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
    
    


class LexicalAnalyzer {
private:
    Node_Token root;
    Node_Token tail;
    vector<Token> tokenBuffer; // 暫存read data，去掉" "、"\n"、"\t"、""、    ?"\r"
    int g_line = 1, g_column = 1; // '(" ", "\t"， "\")保留
    int leftCount = 0, rightCount = 0;
    bool is_EOF;
    // bool is_call_function;
    // bool is_quote;
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

    void whitespace(int num) {
        for (int i = 0; i < num; i++) {
            cout << " ";
        }
    }



    void set_EOF(bool parameter) {
        is_EOF = parameter;
    }

public:
    LexicalAnalyzer() : is_EOF(false), Str("") {//, is_call_function(false)
        root.token = nullptr;
        tail.token = nullptr;
    }

    void GetStr(bool &finish_input, errorType &error) {
        Token tmpstr;
        // bool is_string = false;
        // int count_str_quote = 0; // count "

        char c_peek;
        while(!finish_input && error == Error_None) {
            c_peek = cin.peek();
            tmpstr.line = g_line; // init by 1
            tmpstr.column = g_column; // init by 1

            if (error != Error_None) return; // 可刪，保險

            if (c_peek == EOF) { // read EOF
                error = UNEXPECTED_EOF;
                set_EOF(true);
                return;
            }

            if (c_peek == '(' || c_peek == ')' || c_peek == '\'') {
                tmpstr.value.push_back(getchar());
                
                cout << "peek: " << c_peek << endl;
                cout << "Read char: " << tmpstr.value << " (line " << g_line << ", col " << g_column << ")" << endl;
                tokenBuffer.push_back(tmpstr);
                tmpstr.value="";
                g_column++;
            }
            else if (c_peek == '\n') {
                cout << "\033[1;33m" << "Read Symbol, add to vector." << "\033[0m" << endl;
                cout << "Read char: " << tmpstr.value << " (line " << g_line << ", col " << g_column << ")" << endl;
                tokenBuffer.push_back(tmpstr);
                tmpstr.value="";

                getchar(); // ignore '\n'
                g_line++;
                g_column = 1;
            }
            else if (c_peek == ' ' || c_peek == '\t') { // 有 " 時，不進入 && count_str_quote % 2 == 0
                getchar(); // ignore ' ' or '\t'
                cout << "\033[1;33m" << "Read space or tab" << "\033[0m" << endl;
                // cout << "peek: " << c_peek << endl;
                cout << "Read char: " << tmpstr.value << " (line " << g_line << ", col " << g_column << ")" << endl;
                g_column++;
            }
            else if (c_peek == '\'') {
                cout << "peek: " << c_peek << endl;
                cout << "Read char: " << tmpstr.value << " (line " << g_line << ", col " << g_column << ")" << endl;
                tmpstr.value.push_back(getchar());
                tokenBuffer.push_back(tmpstr);

                tmpstr.value="";
                g_column++;

                // read total line
            }
            // !!!!!!!!!!!!
            else if (c_peek == '\"') { // need read total line until meet another ", but not include '\n'
                                       // if meet '\n' before another ", rise -> ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 7
                                       // ! (no closing quote)
                // char c = getchar();
                tmpstr.value.push_back(getchar());
                tmpstr.type = STRING;
                // tmpstr.line = g_line;
                // tmpstr.column = g_column;
                cout << "peek: " << c_peek << endl;
                cout << "Read char: " << tmpstr.value << " (line " << g_line << ", col " << g_column << ")" << endl;
                tokenBuffer.push_back(tmpstr);
                tmpstr.value="";
                g_column++;

                tmpstr.line = g_line; // after the first "
                tmpstr.column = g_column; // after the first "
                do {
                    c_peek = cin.peek();
                    if (c_peek == '\n') {
                        error = UNEXPECTED_STRING;
                        getchar(); // ignore '\n'
                        return;
                    }
                    else if (c_peek == EOF) {
                        error = UNEXPECTED_EOF;
                        getchar(); // ignore 'EOF'
                        set_EOF(true); 
                        return;
                    }
                    tmpstr.value.push_back(getchar());
                    
                    g_column++;
                }
                while (c_peek != '\"');

                // 
                
                cout << "Read char: " << tmpstr.value << " (line " << g_line << ", col " << g_column << ")" << endl;
                tokenBuffer.push_back(tmpstr);
                tmpstr.value="";
                // ! 預計離開 while 後，下個 c_peek 一定是 "
                if (c_peek == '\"') {
                    tmpstr.value.push_back(getchar());
                    tokenBuffer.push_back(tmpstr);
                    tmpstr.value="";
                    g_column++;
                }
                
            }

            else { // Symbol, INT, FLOAT
                // read_whole_symbol()
                char c = getchar();
                cout << "Read char c: " << c << " (line " << g_line << ", col " << g_column << ")" << endl;
                tmpstr.value.push_back(c);
                g_column++;
            }
            
            
        }

        // is_call_function = false;
    }

    bool Get_is_EOF(){
        return is_EOF;
    }

    int getCurrentLine() {
        return g_line;
    }

    int getCurrentColumn() {
        return g_column;
    }
    ~LexicalAnalyzer() {

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
    SyntaxAnalyzer Syntax; //語法分析器

    // bool is_EOF = false;
    string expr, input;
    int line, column = 0;
    Node_Token *head = nullptr;
    Node_Token *tail = nullptr;

    cout << "Welcome to OurScheme!" << endl << endl;
    
    while (!Lexical.Get_is_EOF()) { // while (true)
        bool finish_input = false;
        try {
            cout << "> ";
            // while(!finish_input && !Lexical.Get_is_EOF()) {
                errorType E = Error_None;
                Lexical.GetStr(finish_input, E);

                if (E != Error_None) {
                    throw error {
                        E,
                        "",
                        Lexical.getCurrentLine(),
                        Lexical.getCurrentColumn(),
                        ""
                    };
                    break;
                }
            // }

            if (E == UNEXPECTED_EXIT) {
                break;
            }

            // Lexical.CutToken(input);

        } catch (error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    cout << "\033[1;31m" << "ERROR (unexpected token) : " << e.message << " when token at Line " << e.line << " Column " << e.column << " is >>" << e.expected << "<< " << "\033[0m" << endl;
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    cout << "\033[1;31m" << "ERROR (unexpected token) : " << e.message << " when token at Line " << e.line << " Column " << e.column << " is >>" << e.expected << "<< " << "\033[0m" << endl;
                    break;
                case UNEXPECTED_STRING:
                    cout << "\033[1;31m" << "ERROR (no closing quote) : " << "END-OF-LINE encountered" << " at Line " << e.line << " Column " << e.column << "\033[0m" << endl;
                    break;
                case UNEXPECTED_EOF:
                    cout << "\033[1;31m" << "ERROR (no more input) : END-OF-FILE encountered" << "\033[0m" << endl;
                    break;
                case UNEXPECTED_EXIT:
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