
# 🧠 Mini C Compiler in C

This is a Mini C Compiler written in pure C, capable of performing lexical analysis, parsing, symbol table management, and executing a subset of C-style statements. It's a great learning project to understand the basics of how compilers work.

---

## 🔥 Features

- ✅ Tokenizer (Lexical Analyzer)
- ✅ Recursive Descent Parser
- ✅ Symbol Table for Variables
- ✅ Abstract Syntax Tree (AST) Execution
- ✅ Supports simple:
  - `int` variable declarations
  - Arithmetic operations (+, -, *, /)
  - `if` statements
  - `while` loops
  - `print` statements

---

## 📝 Example Code It Can Interpret

```c
int x = 10;
int y = 5;
if (x > y) {
  print x + y;
}
```

---

## 🛠️ How to Compile and Run

### 1️⃣ Compile the Compiler:
```bash
gcc mini_compiler.c -o mini_compiler
```

### 2️⃣ Create a Source File:
Make a file `program.txt` with code using the mini C syntax.

### 3️⃣ Run the Compiler:
```bash
./mini_compiler program.txt
```

---

## 📂 Folder Structure

```
mini-c-compiler/
├── mini_compiler.c       # Main C compiler code
├── program.txt           # Sample input code
├── README.md             # This file
└── LICENSE               # Optional MIT License
```

---

## 🧪 Sample Output

```
Output: 15
```

---

## 📚 Topics You’ll Learn

- Compiler Design
- Abstract Syntax Trees
- Tokenization
- Parsing logic (Recursive Descent)
- Symbol Table implementation

---

## 🔗 Project Link

View full source code and contribute here:  
👉 [https://github.com/pranshu-rajan/mini-c-compiler](https://github.com/pranshu-rajan/mini-c-compiler)

---

## 👨‍💻 Author

**Pranshu Rajan**  
B.Tech, Electronics & Instrumentation Engineering  
Nirma University  
🔗 [LinkedIn Profile](https://www.linkedin.com/in/pranshu-rajan-898556325/)
