#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Token types
typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_PRINT,
    TOKEN_INT,
    TOKEN_EQUALS,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_EOF,
    TOKEN_NEWLINE
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char value[100];
    int line;
} Token;

// Symbol table entry
typedef struct Symbol {
    char name[100];
    int value;
    struct Symbol* next;
} Symbol;

// AST Node types
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK
} NodeType;

// AST Node structure
typedef struct ASTNode {
    NodeType type;
    char value[100];
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* condition;
    struct ASTNode* body;
    struct ASTNode* else_body;
    struct ASTNode** statements;
    int statement_count;
} ASTNode;

// Global variables
char* source_code;
int current_pos = 0;
int current_line = 1;
Token current_token;
Symbol* symbol_table = NULL;
FILE* output_file;

// Function prototypes
void lexer_init(char* code);
Token get_next_token();
void parser_init();
ASTNode* parse_program();
ASTNode* parse_statement();
ASTNode* parse_expression();
ASTNode* parse_term();
ASTNode* parse_factor();
void generate_code(ASTNode* node);
void execute_interpreter(ASTNode* node);
Symbol* lookup_symbol(char* name);
void add_symbol(char* name, int value);
void print_ast(ASTNode* node, int depth);
void error(char* message);

// Lexical Analyzer
void lexer_init(char* code) {
    source_code = code;
    current_pos = 0;
    current_line = 1;
}

Token get_next_token() {
    Token token;
    
    // Skip whitespace
    while (current_pos < strlen(source_code) && isspace(source_code[current_pos])) {
        if (source_code[current_pos] == '\n') {
            current_line++;
        }
        current_pos++;
    }
    
    // End of file
    if (current_pos >= strlen(source_code)) {
        token.type = TOKEN_EOF;
        strcpy(token.value, "EOF");
        token.line = current_line;
        return token;
    }
    
    token.line = current_line;
    
    // Numbers
    if (isdigit(source_code[current_pos])) {
        int i = 0;
        while (current_pos < strlen(source_code) && isdigit(source_code[current_pos])) {
            token.value[i++] = source_code[current_pos++];
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }
    
    // Identifiers and keywords
    if (isalpha(source_code[current_pos]) || source_code[current_pos] == '_') {
        int i = 0;
        while (current_pos < strlen(source_code) && 
               (isalnum(source_code[current_pos]) || source_code[current_pos] == '_')) {
            token.value[i++] = source_code[current_pos++];
        }
        token.value[i] = '\0';
        
        // Check for keywords
        if (strcmp(token.value, "if") == 0) token.type = TOKEN_IF;
        else if (strcmp(token.value, "else") == 0) token.type = TOKEN_ELSE;
        else if (strcmp(token.value, "while") == 0) token.type = TOKEN_WHILE;
        else if (strcmp(token.value, "print") == 0) token.type = TOKEN_PRINT;
        else if (strcmp(token.value, "int") == 0) token.type = TOKEN_INT;
        else token.type = TOKEN_IDENTIFIER;
        
        return token;
    }
    
    // Single character tokens
    char ch = source_code[current_pos++];
    token.value[0] = ch;
    token.value[1] = '\0';
    
    switch (ch) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_MULTIPLY; break;
        case '/': token.type = TOKEN_DIVIDE; break;
        case '=': 
            if (current_pos < strlen(source_code) && source_code[current_pos] == '=') {
                current_pos++;
                token.type = TOKEN_EQUALS;
                strcpy(token.value, "==");
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '<': token.type = TOKEN_LESS; break;
        case '>': token.type = TOKEN_GREATER; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        default:
            printf("Error: Unknown character '%c' at line %d\n", ch, current_line);
            exit(1);
    }
    
    return token;
}

// Parser
void parser_init() {
    current_token = get_next_token();
}

ASTNode* create_node(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    node->body = NULL;
    node->else_body = NULL;
    node->statements = NULL;
    node->statement_count = 0;
    strcpy(node->value, "");
    return node;
}

void expect_token(TokenType expected) {
    if (current_token.type != expected) {
        printf("Error: Expected token type %d, got %d at line %d\n", 
               expected, current_token.type, current_token.line);
        exit(1);
    }
    current_token = get_next_token();
}

ASTNode* parse_program() {
    ASTNode* program = create_node(NODE_BLOCK);
    program->statements = malloc(sizeof(ASTNode*) * 100);
    
    while (current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_NEWLINE) {
            current_token = get_next_token();
            continue;
        }
        program->statements[program->statement_count++] = parse_statement();
    }
    
    return program;
}

ASTNode* parse_statement() {
    ASTNode* node = NULL;
    
    if (current_token.type == TOKEN_INT) {
        // Variable declaration: int x = 5;
        current_token = get_next_token();
        if (current_token.type != TOKEN_IDENTIFIER) {
            error("Expected identifier after 'int'");
        }
        
        char var_name[100];
        strcpy(var_name, current_token.value);
        current_token = get_next_token();
        
        expect_token(TOKEN_ASSIGN);
        
        node = create_node(NODE_ASSIGN);
        strcpy(node->value, var_name);
        node->right = parse_expression();
        
        expect_token(TOKEN_SEMICOLON);
    }
    else if (current_token.type == TOKEN_IDENTIFIER) {
        // Assignment: x = 5;
        char var_name[100];
        strcpy(var_name, current_token.value);
        current_token = get_next_token();
        
        expect_token(TOKEN_ASSIGN);
        
        node = create_node(NODE_ASSIGN);
        strcpy(node->value, var_name);
        node->right = parse_expression();
        
        expect_token(TOKEN_SEMICOLON);
    }
    else if (current_token.type == TOKEN_PRINT) {
        // Print statement: print(x);
        current_token = get_next_token();
        expect_token(TOKEN_LPAREN);
        
        node = create_node(NODE_PRINT);
        node->left = parse_expression();
        
        expect_token(TOKEN_RPAREN);
        expect_token(TOKEN_SEMICOLON);
    }
    else if (current_token.type == TOKEN_IF) {
        // If statement: if (condition) { statements }
        current_token = get_next_token();
        expect_token(TOKEN_LPAREN);
        
        node = create_node(NODE_IF);
        node->condition = parse_expression();
        
        expect_token(TOKEN_RPAREN);
        expect_token(TOKEN_LBRACE);
        
        node->body = create_node(NODE_BLOCK);
        node->body->statements = malloc(sizeof(ASTNode*) * 100);
        
        while (current_token.type != TOKEN_RBRACE) {
            node->body->statements[node->body->statement_count++] = parse_statement();
        }
        
        expect_token(TOKEN_RBRACE);
    }
    else if (current_token.type == TOKEN_WHILE) {
        // While loop: while (condition) { statements }
        current_token = get_next_token();
        expect_token(TOKEN_LPAREN);
        
        node = create_node(NODE_WHILE);
        node->condition = parse_expression();
        
        expect_token(TOKEN_RPAREN);
        expect_token(TOKEN_LBRACE);
        
        node->body = create_node(NODE_BLOCK);
        node->body->statements = malloc(sizeof(ASTNode*) * 100);
        
        while (current_token.type != TOKEN_RBRACE) {
            node->body->statements[node->body->statement_count++] = parse_statement();
        }
        
        expect_token(TOKEN_RBRACE);
    }
    
    return node;
}

ASTNode* parse_expression() {
    ASTNode* node = parse_term();
    
    while (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS ||
           current_token.type == TOKEN_EQUALS || current_token.type == TOKEN_LESS ||
           current_token.type == TOKEN_GREATER) {
        
        ASTNode* op_node = create_node(NODE_BINARY_OP);
        strcpy(op_node->value, current_token.value);
        op_node->left = node;
        
        current_token = get_next_token();
        op_node->right = parse_term();
        
        node = op_node;
    }
    
    return node;
}

ASTNode* parse_term() {
    ASTNode* node = parse_factor();
    
    while (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE) {
        ASTNode* op_node = create_node(NODE_BINARY_OP);
        strcpy(op_node->value, current_token.value);
        op_node->left = node;
        
        current_token = get_next_token();
        op_node->right = parse_factor();
        
        node = op_node;
    }
    
    return node;
}

ASTNode* parse_factor() {
    ASTNode* node = NULL;
    
    if (current_token.type == TOKEN_NUMBER) {
        node = create_node(NODE_NUMBER);
        strcpy(node->value, current_token.value);
        current_token = get_next_token();
    }
    else if (current_token.type == TOKEN_IDENTIFIER) {
        node = create_node(NODE_IDENTIFIER);
        strcpy(node->value, current_token.value);
        current_token = get_next_token();
    }
    else if (current_token.type == TOKEN_LPAREN) {
        current_token = get_next_token();
        node = parse_expression();
        expect_token(TOKEN_RPAREN);
    }
    else {
        error("Unexpected token in expression");
    }
    
    return node;
}

// Symbol Table Management
Symbol* lookup_symbol(char* name) {
    Symbol* current = symbol_table;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_symbol(char* name, int value) {
    Symbol* existing = lookup_symbol(name);
    if (existing) {
        existing->value = value;
        return;
    }
    
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    strcpy(new_symbol->name, name);
    new_symbol->value = value;
    new_symbol->next = symbol_table;
    symbol_table = new_symbol;
}

// Interpreter
int evaluate_expression(ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            return atoi(node->value);
            
        case NODE_IDENTIFIER: {
            Symbol* symbol = lookup_symbol(node->value);
            if (!symbol) {
                printf("Error: Undefined variable '%s'\n", node->value);
                exit(1);
            }
            return symbol->value;
        }
        
        case NODE_BINARY_OP: {
            int left = evaluate_expression(node->left);
            int right = evaluate_expression(node->right);
            
            if (strcmp(node->value, "+") == 0) return left + right;
            else if (strcmp(node->value, "-") == 0) return left - right;
            else if (strcmp(node->value, "*") == 0) return left * right;
            else if (strcmp(node->value, "/") == 0) return left / right;
            else if (strcmp(node->value, "==") == 0) return left == right;
            else if (strcmp(node->value, "<") == 0) return left < right;
            else if (strcmp(node->value, ">") == 0) return left > right;
        }
    }
    return 0;
}

void execute_interpreter(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_BLOCK:
            for (int i = 0; i < node->statement_count; i++) {
                execute_interpreter(node->statements[i]);
            }
            break;
            
        case NODE_ASSIGN: {
            int value = evaluate_expression(node->right);
            add_symbol(node->value, value);
            break;
        }
        
        case NODE_PRINT: {
            int value = evaluate_expression(node->left);
            printf("%d\n", value);
            break;
        }
        
        case NODE_IF: {
            int condition = evaluate_expression(node->condition);
            if (condition) {
                execute_interpreter(node->body);
            }
            break;
        }
        
        case NODE_WHILE: {
            while (evaluate_expression(node->condition)) {
                execute_interpreter(node->body);
            }
            break;
        }
    }
}

// Code Generator (generates C code)
void generate_code(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_BLOCK:
            fprintf(output_file, "{\n");
            for (int i = 0; i < node->statement_count; i++) {
                generate_code(node->statements[i]);
            }
            fprintf(output_file, "}\n");
            break;
            
        case NODE_ASSIGN:
            fprintf(output_file, "    int %s = ", node->value);
            generate_code(node->right);
            fprintf(output_file, ";\n");
            break;
            
        case NODE_PRINT:
            fprintf(output_file, "    printf(\"%%d\\n\", ");
            generate_code(node->left);
            fprintf(output_file, ");\n");
            break;
            
        case NODE_NUMBER:
            fprintf(output_file, "%s", node->value);
            break;
            
        case NODE_IDENTIFIER:
            fprintf(output_file, "%s", node->value);
            break;
            
        case NODE_BINARY_OP:
            fprintf(output_file, "(");
            generate_code(node->left);
            fprintf(output_file, " %s ", node->value);
            generate_code(node->right);
            fprintf(output_file, ")");
            break;
            
        case NODE_IF:
            fprintf(output_file, "    if (");
            generate_code(node->condition);
            fprintf(output_file, ") ");
            generate_code(node->body);
            break;
            
        case NODE_WHILE:
            fprintf(output_file, "    while (");
            generate_code(node->condition);
            fprintf(output_file, ") ");
            generate_code(node->body);
            break;
    }
}

void error(char* message) {
    printf("Error: %s at line %d\n", message, current_line);
    exit(1);
}

// Main function
int main() {
    printf("🚀 MINI COMPILER v1.0 - Built in C\n");
    printf("=====================================\n\n");
    
    // Sample program
    char* program = 
        "int x = 10;\n"
        "int y = 20;\n"
        "int sum = x + y;\n"
        "print(sum);\n"
        "int i = 1;\n"
        "while (i < 5) {\n"
        "    print(i);\n"
        "    i = i + 1;\n"
        "}\n"
        "if (sum > 25) {\n"
        "    print(999);\n"
        "}\n";
    
    printf("Source Code:\n");
    printf("------------\n");
    printf("%s\n", program);
    
    // Lexical Analysis
    printf("🔍 LEXICAL ANALYSIS:\n");
    printf("--------------------\n");
    lexer_init(program);
    Token token;
    do {
        token = get_next_token();
        printf("Token: %d, Value: '%s', Line: %d\n", token.type, token.value, token.line);
    } while (token.type != TOKEN_EOF);
    
    // Parsing
    printf("\n🌳 PARSING & AST GENERATION:\n");
    printf("----------------------------\n");
    lexer_init(program);
    parser_init();
    ASTNode* ast = parse_program();
    printf("✅ AST successfully generated!\n");
    
    // Code Generation
    printf("\n⚙️ CODE GENERATION:\n");
    printf("-------------------\n");
    output_file = fopen("output.c", "w");
    fprintf(output_file, "#include <stdio.h>\n\nint main() {\n");
    generate_code(ast);
    fprintf(output_file, "    return 0;\n}\n");
    fclose(output_file);
    printf("✅ C code generated in 'output.c'\n");
    
    // Interpretation
    printf("\n🎯 INTERPRETER OUTPUT:\n");
    printf("----------------------\n");
    execute_interpreter(ast);
    
    printf("\n🎉 Compilation completed successfully!\n");
    printf("📁 Check 'output.c' for generated C code\n");
    
    return 0;
}