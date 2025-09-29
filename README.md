# OurScheme Interpreter

本專案為課程 **Programming Languages (PL)** 的專案，為期一學期，實作一個簡化版的 Scheme 直譯器 —— **OurScheme**。  
專案分為四個階段，逐步實作語法解析、求值、錯誤處理與 I/O 支援。

---

## 專案目標
- 熟悉 **S-expression** 的語法與語意
- 建立 **遞迴下降解析器 (Recursive Descent Parser)**
- 實作 **AST 評估器 (Evaluator)**
- 支援 **Scheme 常見內建函數**
- 處理 **錯誤物件 (Error Object)** 與 I/O 輸入輸出
- 體會直譯器設計與程式語言核心概念

---

## 功能實作進度

### Project 1：基本直譯器架構
- 支援基本 S-expression 解析
- 實作基本資料型別：
  - INT, FLOAT, STRING, SYMBOL, NIL
- 支援 `define` 與簡單運算
- 輸出格式符合題目規範

---

### Project 2：語法與語意擴充
- 增加特殊形式 (special forms)：
  - `quote`, `cons`, `list`, `car`, `cdr`
- 增加布林運算：
  - `eqv?`, `equal?`, `not`
- 增加數值運算：
  - `+`, `-`, `*`, `/`, `>`, `<`, `>=`, `<=`
- 增加判斷式：
  - `if`, `cond`, `and`, `or`

---

### Project 3：函數與環境
- 支援匿名函數 `(lambda ...)`
- 支援遞迴函數呼叫
- 增加變數環境 (Environment) 與作用域管理
- 實作 `let`, `begin`, `set!`
- 錯誤處理：
  - `unbound symbol`
  - `argument number mismatch`

---

### Project 4：錯誤物件與 I/O
- 新增資料型別：**ERROR**
- 新增函數：
  - `create-error-object`, `error-object?`
  - `read`, `write`, `display-string`, `newline`
  - `symbol->string`, `number->string`
- 行為說明：
  - `read`：讀取下一個 S-expression，錯誤時回傳錯誤物件
  - `write`：印出 S-expression，字串會有引號
  - `display-string`：印出字串，不帶引號
  - `newline`：換行並回傳 `nil`
- 錯誤物件：
  - 僅能透過 `create-error-object` 或 `read` 產生
  - `error-object?` 可判斷是否為錯誤物件

---

## 使用範例

```scheme
> (define a (read)) (1 2 3)
a defined

> (error-object? a)
nil

> a
( 1
  2
  3
)

> (define a (read))(1 3 5 . 7 s)
a defined

> (error-object? a)
#t

> a
"ERROR (unexpected character) : line 1 column 13 character 's'"

> (number->string 23.0)
"23.000"
