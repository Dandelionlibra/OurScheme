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
    FLOAT, // '123.567', '123.', '.567', '+123.4', '-.123'
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
    Token token; // token info, value, type
    Node_Token *parent;
    Node_Token *left;
    Node_Token *right;
    Node_Token() : left(nullptr), right(nullptr) {}
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

    string expected;
    string current;    
    int line;
    int column;

    error() : type(Error_None), message(""), line(0), column(0), expected(""), current("") {}
    error(errorType t, string e_token, string c_token, int l, int c) {
        type = t;
        expected = e_token;
        current = c_token;
        line = l;
        column = c;

        // cout << "error type: " << t << " expected: " << e_token << " current: " << c_token << " line: " << l << " column: " << c << endl;

        if (t == UNEXPECTED_TOKEN)
            message = "UNEXPECTED_TOKEN ";
        else if (t == UNEXPECTED_CLOSE_PAREN)
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
class Parser_Tree {
    private:
        // SyntaxAnalyzer syntax;

        Token *root;

        void insert(Token *t) {
            if (root == nullptr) {
                root = t;
            } else {
                Token *temp = root;
    
            }
        }
    public:
        Parser_Tree() {
            root = nullptr;
        }
        ~Parser_Tree() {
            delete root;
        }

        void build_tree(vector<Token> &v) {
            for (int i = 0; i < v.size(); i++) {
                insert(&v.at(i));
            }
        }

        void print() {
            Token *temp = root;
    
        }
    
    };
        

class SyntaxAnalyzer {
private:
    Parser_Tree tree;

public:
    SyntaxAnalyzer() {

    }

    void build_tree(vector<Token> &v) {
        
        tree.build_tree(v);
    }


    ~SyntaxAnalyzer() {

    }


};


class LexicalAnalyzer {
private:
    vector<Token> tokenBuffer; // 暫存read data，去掉" "、"\n"、"\t"、""、    ?"\r"
    int g_line = 1, g_column = 1; // '(" ", "\t"， "\")保留
    int leftCount = 0, rightCount = 0; // count '(' and ')'
    bool is_EOF;

    int error_line, error_column;
    string ExpectedToken, CurrentToken;

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
        // 做特判，避免單一的 symbol 跟緊跟著的左右括號一起處理
        if (tokenBuffer.size() == 1 && leftCount == 0 && rightCount == 0) { // ex. 9(454(.3 2 * )3)
            if (tokenBuffer.at(0).value != "\'") { // 排除 '(454(.3 2 * )3)
                finish_input = true;
                return false;
            }
        }

        if (token == '(' || token == ')') {
            is_PAREN = true;
            if (token == '(') // count '('
                leftCount++;
            else // count ')'
                rightCount++;
        }

        return is_PAREN;
    }
    
    bool is_sep(char token ) {
        if ( token == ';' )
            return true;
        return false;
    }

    bool is_special_symbol(char token) {
        if (token == '(' || token == ')' || token == '\'' || token == ';' || token == ' ' || token == '\t' || token == '\n' || token == '\r' || token == EOF || token == '\"')
            return true;
        return false;
    }

    bool is_int(string &str) {
        if (str.empty())
            return false;

        string tmp = str;
        int i = 0;
        while (i < str.size()) {
            // + or - appear at the first character
            if (i == 0 && (str.at(0) == '+' || str.at(0) == '-')) {
                if (str.at(0) == '+')
                    tmp = str.substr(1); // Remove the '+' sign from the string
                cout << "int str tmp: " << tmp << endl;
                
                i++;
                continue;
            }
            if (!isdigit(str.at(i)))
                return false;
            i++;
        }

        str = tmp;
        cout << "END : int str: " << str << endl;
        return true;
    }

    bool is_float(string &str) {
        if (str.empty())
            return false;

        string tmp = str;
        string sign = "";
        int i = 0;
        int dot_count = 0;
        while (i < str.size()) {
            if (i == 0 && (str.at(0) == '+' || str.at(0) == '-')) {
                sign = str.at(0); // save the sign
                tmp = str.substr(1); // Remove the sign from the string
                i++;
                continue;
            }
            if (str[i] == '.') {
                dot_count++;
                if (dot_count > 1)
                    return false;
                if (i == 0 || !isdigit(str[i - 1])) {
                    tmp.insert(tmp.begin(), '0'); // Insert '0' before the dot if no digit before it
                }
                i++;
                continue;
            }
            if (!isdigit(str[i]) && str[i] != '.')
                return false;
            i++;
        }
        
        int dot_pos = tmp.find('.');
        // 小數點後大於三位數
        if (tmp.size() - dot_pos > 4)
            tmp = tmp.substr(0, dot_pos + 4);
        // add 0 to the end if after dot don't have at least 3 digits
        else { 
            if (tmp[tmp.size() - 1] == '.')
                tmp = tmp + "000";
            else if (tmp[tmp.size() - 2] == '.')
                tmp = tmp + "00";
            else if (tmp[tmp.size() - 3] == '.')
                tmp = tmp + "0";
        }

        if (sign == "-")
            tmp = sign + tmp;

        str = tmp;
        cout << "END : float str: " << str << endl;
        return true;
    }

    TokenType check_token_type(string &str) {
        TokenType type;
        if (str == "NIL" || str == "nil")
            type = NIL;
        else if (str == "T")
            type = T;
        else if (str == "'" || str == "QUOTE")
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

    Token token_info(string &str) {
        cout << "\033[1;33menter token_info\033[0m" << endl;
        Token tmptoken;
        tmptoken.type = check_token_type(str);
        tmptoken.value = str;

        tmptoken.line = g_line;
        tmptoken.column = g_column;
        tmptoken.level = leftCount - rightCount;
        if (tmptoken.value == ")")
            tmptoken.level++;

        cout << "\033[1;33m" << "tmptoken.value: " << tmptoken.value << " tmptoken.type: " << tmptoken.type << " tmptoken.line: " << tmptoken.line << " tmptoken.column: " << tmptoken.column << " level:" << tmptoken.level << "\033[0m" << endl;
        str = ""; // reset by ""

        return tmptoken;
    }
    
    void read_whole_string(char &c_peek, char end_char, errorType &error, bool &finish_input) {
        string tmpstr;
        // int begin_line = g_line;
        // int begin_column = g_column;

        if (end_char == '\"') {
            cout << "1. enter read_whole_string" << endl;
            c_peek = cin.peek();
            do {
                
                if (c_peek == '\n') {
                    error = UNEXPECTED_STRING;
                    int t_column = g_column;
                    g_column = g_column - tmpstr.size();
                    tokenBuffer.push_back(token_info(tmpstr)); // push_back string
                    g_column = t_column;
                    getchar(); // ignore '\n'
                    finish_input = true;

                    return;
                }
                else if (c_peek == EOF) {
                    error = UNEXPECTED_EOF;
                    getchar(); // ignore 'EOF'
                    set_EOF(true); 
                    finish_input = true;
                    return;
                }
                tmpstr.push_back(getchar());
                
                g_column++;
                c_peek = cin.peek();
            }
            while (c_peek != '\"');

            // finish_input = true;
        }
        else if (end_char == ' ') {
            cout << "2. enter read_whole_string" << endl;
            c_peek = cin.peek();
            do {
                // cout << "c_peek: " << c_peek << endl;
                if (c_peek == EOF) {
                    error = UNEXPECTED_EOF;
                    getchar(); // ignore 'EOF'
                    set_EOF(true); 
                    return;
                }

                tmpstr.push_back(getchar());
                
                g_column++;
                c_peek = cin.peek();
            }
            while (!is_special_symbol(c_peek));
                
        }

        if (leftCount == 0){
            if (is_enter(c_peek)) {
                getchar(); // ignore '\n'
                // cout << "\033[1;31m1.ignore enter\033[0m" << endl;
            }
            finish_input = true;
        }

        // int t_line = g_line;
        int t_column = g_column;
        // g_line = begin_line;
        g_column = g_column - tmpstr.size();
        tokenBuffer.push_back(token_info(tmpstr)); // push_back string
        
        // if (leftCount == 0 && tokenBuffer.at(tokenBuffer.size() - 2).value == "\'")
            
        // g_line = t_line;
        g_column = t_column;
    }

    void print_whitespace(int num) {
        for (int i = 0; i < num; i++) {
            cout << " ";
        }
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
    void print_vector(vector<Token> &v) {
        cout << "\033[1;34menter print_vector\033[0m" << endl;
        for (int i = 0; i < v.size(); i++)
            print_token(v.at(i));
    }


    void reset_Line_And_Column(int l, int c) {
        g_line = l;
        g_column = c;
        // cout << "reset line: " << g_line << " column: " << g_column << endl;
    }

    void reset_error_info() {
        g_line = 1;
        g_column = 1;
        leftCount = 0;
        rightCount = 0;
        error_line = 0;
        error_column = 0;
        ExpectedToken = "";
        CurrentToken = "";
        // is_EOF = false;
    }


    void set_EOF(bool parameter) {
        is_EOF = parameter;
    }

    void set_ErrorToken(errorType &error, Token &T) {
        // string expected = "";
        // set_CurrentToken(Token);
        CurrentToken = T.value;
        error_line = T.line;
        error_column = T.column;

        // debug error type
        switch (error) {
            case UNEXPECTED_TOKEN:
            cout << "\033[1;35merror: UNEXPECTED_TOKEN\033[0m" << endl;
            break;
            case UNEXPECTED_CLOSE_PAREN:
            cout << "\033[1;35merror: UNEXPECTED_CLOSE_PAREN\033[0m" << endl;
            break;
            case UNEXPECTED_STRING:
            cout << "\033[1;35merror: UNEXPECTED_STRING\033[0m" << endl;
            break;
            case UNEXPECTED_EOF:
            cout << "\033[1;35merror: UNEXPECTED_EOF\033[0m" << endl;
            break;
            case UNEXPECTED_EXIT:
            cout << "\033[1;35merror: UNEXPECTED_EXIT\033[0m" << endl;
            break;
            default:
            cout << "\033[1;35merror: UNKNOWN_ERROR\033[0m" << endl;
            break;
        }
        
        if (error == UNEXPECTED_EOF)
            return;
        else if (error == UNEXPECTED_TOKEN) {
            // if (CurrentToken == ")") {
                ExpectedToken = "unset!";
            // }
        }
        else if (error == UNEXPECTED_CLOSE_PAREN) {
            // cout << "\033[1;35merror == UNEXPECTED_CLOSE_PAREN\033[0m" << endl;
            ExpectedToken = "atom or '('";
            // cout << "\033[1;35mExpectedToken: " << ExpectedToken << "\033[0m" << endl;
            
        }
        else if (error == UNEXPECTED_STRING) {
            ExpectedToken = "unset!";
        }
        
    }

public:
    LexicalAnalyzer() : is_EOF(false), ExpectedToken(""), CurrentToken("") {}

    void GetStr(bool &finish_input, errorType &error) {
        Token tmptoken;
        string tmpstr = "";
        char c_peek = '\0';
        
        while(!finish_input) {
            c_peek = cin.peek();

            if (c_peek == EOF) { // read EOF
                error = UNEXPECTED_EOF;
                set_EOF(true);
                return;
            }
            if (error != Error_None){
                Token t = tokenBuffer.at(tokenBuffer.size() - 1);
                set_ErrorToken(error, t);
                reset_Line_And_Column(t.line, t.column);
                finish_input = true;
                
                return;
            }


            if (is_PAREN(c_peek, finish_input)) { //c_peek == '(' || c_peek == ')'
                // set_Line_And_Column(tmpstr);

                cout << "\033[1;33m" << "Read " << c_peek << "\033[0m" << endl;
                tmpstr.push_back(getchar());
                tokenBuffer.push_back(token_info(tmpstr));
                g_column++;

                if (leftCount < rightCount) 
                    error = UNEXPECTED_CLOSE_PAREN;

                else if (leftCount != 0 && (leftCount == rightCount)) {
                    leftCount = 0;
                    rightCount = 0;
                    
                    c_peek = cin.peek();
                    if (is_enter(c_peek)) {
                        getchar(); // ignore '\n'
                        // cout << "\033[1;31m3.ignore enter\033[0m" << endl;
                    }
                    finish_input = true;

                }
                
            }
            else if (c_peek == '\"') { // need read total line until meet another ", but not include '\n'
                                       // if meet '\n' before another ", rise -> ERROR (no closing quote) : END-OF-LINE encountered at Line 1 Column 7
                                       // ! (no closing quote)
                // cout << "\033[1;33m" << "Read " << c_peek << " add to vector." << "\033[0m" << endl;
                tmpstr.push_back(getchar());
                tokenBuffer.push_back(token_info(tmpstr));
                g_column++;
                
                // read total line
                read_whole_string(c_peek, '\"', error, finish_input); // in this func will set new c_peek
                // ! 預計離開 read_whole_string 後，若沒因 error 而 return，下個 c_peek 一定是 "
                if (error != Error_None){
                    // cout << "error != Error_None" << endl;
                    continue;
                }
                if (c_peek == '\"') {
                    tmpstr.push_back(getchar()); // push_back "
                    tokenBuffer.push_back(token_info(tmpstr));
                    tmpstr = "";
                    g_column++;
                }

                c_peek = cin.peek();
                if (is_enter(c_peek)) {
                    getchar(); // ignore '\n'
                    // cout << "\033[1;31m2.ignore enter\033[0m" << endl;
                }
                // finish_input = true;
            }

            else if ( c_peek == '\'') { // quote
                // cout << "\033[1;33m" << "Read " << c_peek << ", add to vector." << "\033[0m" << endl;
                tmpstr.push_back(getchar());
                tokenBuffer.push_back(token_info(tmpstr));
                g_column++;

                // c_peek = cin.peek(); //
            }
            else if (is_enter(c_peek)) { //c_peek == '\n'
                
                // cout << "\033[1;33m" << "Read enter!" << "\033[0m" << endl;
                cout << "\033[1;32mbefore read enter, line: " << g_line << " column: " << g_column << "\033[0m" << endl;
                getchar(); // ignore '\n'
                
                if (!tokenBuffer.empty() && leftCount == 0 && rightCount == 0)
                    finish_input = true; // build parser tree
                
                else {
                    g_line++;
                    g_column = 1;
                }

                cout << "\033[1;32mafter read enter, line: " << g_line << " column: " << g_column << "\033[0m" << endl;

            }
            else if (is_space(c_peek)) { //c_peek == ' ' || c_peek == '\t') { // 有 " 時，不進入 && count_str_quote % 2 == 0
                cout << "\033[1;33m" << "Read space or tab!" << "\033[0m" << endl;
                // cout << "peek: " << c_peek << endl;
                
                getchar(); // ignore ' ' or '\t'
                g_column++;
            }
            else if (is_sep(c_peek)) { // c_peek == ';'
                cout << "\033[1;33m" << "; Read sep!" << "\033[0m" << endl;
                // cout << "ignore peek: " << c_peek << endl;
                
                while (c_peek != '\n' && c_peek != EOF) {
                    getchar(); // ignore characters until '\n' or EOF
                    c_peek = cin.peek();
                }
                
            }
            else if (!finish_input) // 不結束
                read_whole_string(c_peek, ' ', error, finish_input);
            
            // cout << "\033[1;32mline: " << g_line << " column: " << g_column << "\033[0m" << endl;
            // cout << "\033[1;32mleftCount: " << leftCount << " rightCount: " << rightCount << "\033[0m" << endl;
        }

        if (finish_input)
            print_vector(tokenBuffer);
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

    string getExpectedToken() {
        // cout << "ExpectedToken: " << ExpectedToken << endl;
        return ExpectedToken;
    }

    string getCurrentToken() {
        return CurrentToken;
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

    cout << "Welcome to OurScheme!" << endl;
    
    while (!Lexical.Get_is_EOF()) { // while (true)
        bool finish_input = false;
        try {
            cout << "\n> ";
            errorType E = Error_None;
            Lexical.GetStr(finish_input, E);

            if (E != Error_None) {
                throw error {
                    E,
                    Lexical.getExpectedToken(), // return expected token, type string
                    Lexical.getCurrentToken(), // return current token, type string                        
                    Lexical.getCurrentLine(),
                    Lexical.getCurrentColumn()
                };
                break;
            }
            else if (finish_input) {
                cout << "\033[1;32mfinish_input\033[0m" << endl;
                // bulid parser tree
                // Syntax.;

                // reset lexical vector
                Lexical.reset();
            }

        } catch (error e) {
            switch (e.type) {
                case UNEXPECTED_TOKEN:
                    cout << "\033[1;31m" << "UNEXPECTED_TOKEN" << "\033[0m" << endl;
                    cout << "\033[1;31m" << "ERROR (unexpected token) : " << e.message << " when token at Line " << e.line << " Column " << e.column << " is >>" << e.expected << "<< " << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_CLOSE_PAREN:
                    cout << "\033[1;31m" << "UNEXPECTED_CLOSE_PAREN" << "\033[0m" << endl;
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_STRING:
                    cout << "\033[1;31m" << e.message << "\033[0m" << endl;
                    Lexical.reset();
                    break;
                case UNEXPECTED_EOF:
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