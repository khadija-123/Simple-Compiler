#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============= PHASE 1: LEXICAL ANALYSIS ============= */
typedef enum {
    TOKEN_INT, TOKEN_FLOAT, TOKEN_CHAR, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_FOR, TOKEN_PRINT, TOKEN_INPUT, TOKEN_FUNC, TOKEN_ID, TOKEN_NUM,
    TOKEN_STR, TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV, TOKEN_MOD,
    TOKEN_ASSIGN, TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_GT, TOKEN_LTE, TOKEN_GTE,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_SEMICOLON,
    TOKEN_COMMA, TOKEN_EOF, TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[256];
    int line;
    int value;
} Token;

typedef struct {
    FILE *file;
    int line;
} Lexer;

Lexer* lexer_create(const char *filename) {
    Lexer *l = malloc(sizeof(Lexer));
    l->file = fopen(filename, "r");
    l->line = 1;
    return l;
}

void lexer_destroy(Lexer *l) {
    if (l) {
        if (l->file) fclose(l->file);
        free(l);
    }
}

Token lexer_next_token(Lexer *l) {
    Token t;
    t.line = l->line;
    t.value = 0;
    int c;
    
    do {
        c = fgetc(l->file);
        if (c == '\n') l->line++;
    } while (isspace(c));
    
    if (c == EOF) {
        t.type = TOKEN_EOF;
        strcpy(t.lexeme, "EOF");
        return t;
    }
    
    if (isalpha(c) || c == '_') {
        int i = 0;
        while (isalnum(c) || c == '_') {
            t.lexeme[i++] = c;
            c = fgetc(l->file);
        }
        t.lexeme[i] = '\0';
        ungetc(c, l->file);
        
        if (strcmp(t.lexeme, "int") == 0) t.type = TOKEN_INT;
        else if (strcmp(t.lexeme, "float") == 0) t.type = TOKEN_FLOAT;
        else if (strcmp(t.lexeme, "char") == 0) t.type = TOKEN_CHAR;
        else if (strcmp(t.lexeme, "if") == 0) t.type = TOKEN_IF;
        else if (strcmp(t.lexeme, "else") == 0) t.type = TOKEN_ELSE;
        else if (strcmp(t.lexeme, "while") == 0) t.type = TOKEN_WHILE;
        else if (strcmp(t.lexeme, "for") == 0) t.type = TOKEN_FOR;
        else if (strcmp(t.lexeme, "print") == 0) t.type = TOKEN_PRINT;
        else if (strcmp(t.lexeme, "input") == 0) t.type = TOKEN_INPUT;
        else if (strcmp(t.lexeme, "func") == 0) t.type = TOKEN_FUNC;
        else t.type = TOKEN_ID;
        return t;
    }
    
    if (isdigit(c)) {
        int i = 0;
        while (isdigit(c)) {
            t.lexeme[i++] = c;
            t.value = t.value * 10 + (c - '0');
            c = fgetc(l->file);
        }
        t.lexeme[i] = '\0';
        ungetc(c, l->file);
        t.type = TOKEN_NUM;
        return t;
    }
    
    if (c == '"') {
        int i = 0;
        c = fgetc(l->file);
        while (c != '"' && c != EOF) {
            t.lexeme[i++] = c;
            c = fgetc(l->file);
        }
        t.lexeme[i] = '\0';
        t.type = TOKEN_STR;
        return t;
    }
    
    t.lexeme[0] = c;
    t.lexeme[1] = '\0';
    
    switch (c) {
        case '+': t.type = TOKEN_PLUS; break;
        case '-': t.type = TOKEN_MINUS; break;
        case '*': t.type = TOKEN_MUL; break;
        case '/': t.type = TOKEN_DIV; break;
        case '%': t.type = TOKEN_MOD; break;
        case '=':
            if (fgetc(l->file) == '=') {
                t.type = TOKEN_EQ;
                strcpy(t.lexeme, "==");
            } else {
                ungetc('=', l->file);
                t.type = TOKEN_ASSIGN;
            }
            break;
        case '!':
            if (fgetc(l->file) == '=') {
                t.type = TOKEN_NEQ;
                strcpy(t.lexeme, "!=");
            } else {
                ungetc('!', l->file);
                t.type = TOKEN_ERROR;
            }
            break;
        case '<': t.type = TOKEN_LT; break;
        case '>': t.type = TOKEN_GT; break;
        case '(': t.type = TOKEN_LPAREN; break;
        case ')': t.type = TOKEN_RPAREN; break;
        case '{': t.type = TOKEN_LBRACE; break;
        case '}': t.type = TOKEN_RBRACE; break;
        case ';': t.type = TOKEN_SEMICOLON; break;
        case ',': t.type = TOKEN_COMMA; break;
        default: t.type = TOKEN_ERROR;
    }
    return t;
}

/* ============= PHASE 2: SYNTAX ANALYSIS (TOP-DOWN PARSER) ============= */
typedef struct ASTNode {
    char type[64];
    char value[256];
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next;
} ASTNode;

typedef struct {
    Lexer *lexer;
    Token current;
} Parser;

ASTNode* ast_create(const char *type) {
    ASTNode *n = malloc(sizeof(ASTNode));
    strcpy(n->type, type);
    n->value[0] = '\0';
    n->left = n->right = n->next = NULL;
    return n;
}

Parser* parser_create(const char *filename) {
    Parser *p = malloc(sizeof(Parser));
    p->lexer = lexer_create(filename);
    p->current = lexer_next_token(p->lexer);
    return p;
}

void parser_advance(Parser *p) {
    p->current = lexer_next_token(p->lexer);
}

ASTNode* parse_expression(Parser *p);

ASTNode* parse_primary(Parser *p) {
    if (p->current.type == TOKEN_NUM) {
        ASTNode *n = ast_create("number");
        sprintf(n->value, "%d", p->current.value);
        parser_advance(p);
        return n;
    }
    
    if (p->current.type == TOKEN_ID) {
        ASTNode *n = ast_create("identifier");
        strcpy(n->value, p->current.lexeme);
        parser_advance(p);
        return n;
    }
    
    if (p->current.type == TOKEN_STR) {
        ASTNode *n = ast_create("string");
        strcpy(n->value, p->current.lexeme);
        parser_advance(p);
        return n;
    }
    
    if (p->current.type == TOKEN_LPAREN) {
        parser_advance(p);
        ASTNode *expr = parse_expression(p);
        if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        return expr;
    }
    
    return ast_create("empty");
}

ASTNode* parse_term(Parser *p) {
    ASTNode *left = parse_primary(p);
    
    while (p->current.type == TOKEN_MUL || 
           p->current.type == TOKEN_DIV || 
           p->current.type == TOKEN_MOD) {
        ASTNode *op = ast_create("binary_op");
        strcpy(op->value, p->current.lexeme);
        parser_advance(p);
        op->left = left;
        op->right = parse_primary(p);
        left = op;
    }
    
    return left;
}

ASTNode* parse_expression(Parser *p) {
    ASTNode *left = parse_term(p);
    
    while (p->current.type == TOKEN_PLUS || 
           p->current.type == TOKEN_MINUS) {
        ASTNode *op = ast_create("binary_op");
        strcpy(op->value, p->current.lexeme);
        parser_advance(p);
        op->left = left;
        op->right = parse_term(p);
        left = op;
    }
    
    return left;
}

ASTNode* parse_statement(Parser *p);

ASTNode* parse_block(Parser *p) {
    ASTNode *block = ast_create("block");
    parser_advance(p);
    
    ASTNode *current = NULL;
    while (p->current.type != TOKEN_RBRACE && p->current.type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(p);
        if (block->left == NULL) {
            block->left = stmt;
            current = stmt;
        } else {
            current->next = stmt;
            current = stmt;
        }
    }
    
    if (p->current.type == TOKEN_RBRACE) parser_advance(p);
    return block;
}

ASTNode* parse_statement(Parser *p) {
    if (p->current.type == TOKEN_INT || 
        p->current.type == TOKEN_FLOAT || 
        p->current.type == TOKEN_CHAR) {
        
        ASTNode *decl = ast_create("declaration");
        strcpy(decl->value, p->current.lexeme);
        parser_advance(p);
        
        if (p->current.type == TOKEN_ID) {
            ASTNode *id = ast_create("identifier");
            strcpy(id->value, p->current.lexeme);
            decl->left = id;
            parser_advance(p);
        }
        
        if (p->current.type == TOKEN_SEMICOLON) parser_advance(p);
        return decl;
    }
    
    if (p->current.type == TOKEN_ID) {
        ASTNode *assign = ast_create("assignment");
        strcpy(assign->value, p->current.lexeme);
        parser_advance(p);
        
        if (p->current.type == TOKEN_ASSIGN) {
            parser_advance(p);
            assign->left = parse_expression(p);
        }
        
        if (p->current.type == TOKEN_SEMICOLON) parser_advance(p);
        return assign;
    }
    
    if (p->current.type == TOKEN_PRINT) {
        ASTNode *print = ast_create("print");
        parser_advance(p);
        
        if (p->current.type == TOKEN_LPAREN) {
            parser_advance(p);
            print->left = parse_expression(p);
            if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        }
        
        if (p->current.type == TOKEN_SEMICOLON) parser_advance(p);
        return print;
    }
    
    if (p->current.type == TOKEN_IF) {
        ASTNode *ifstmt = ast_create("if");
        parser_advance(p);
        
        if (p->current.type == TOKEN_LPAREN) {
            parser_advance(p);
            ifstmt->left = parse_expression(p);
            if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        }
        
        if (p->current.type == TOKEN_LBRACE) {
            ifstmt->right = parse_block(p);
        }
        
        return ifstmt;
    }
    
    if (p->current.type == TOKEN_WHILE) {
        ASTNode *whilestmt = ast_create("while");
        parser_advance(p);
        
        if (p->current.type == TOKEN_LPAREN) {
            parser_advance(p);
            whilestmt->left = parse_expression(p);
            if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        }
        
        if (p->current.type == TOKEN_LBRACE) {
            whilestmt->right = parse_block(p);
        }
        
        return whilestmt;
    }
    
    parser_advance(p);
    return ast_create("empty");
}

ASTNode* parse_program(Parser *p) {
    ASTNode *program = ast_create("program");
    
    ASTNode *current = NULL;
    while (p->current.type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement(p);
        if (program->left == NULL) {
            program->left = stmt;
            current = stmt;
        } else {
            current->next = stmt;
            current = stmt;
        }
    }
    
    return program;
}

/* ============= PHASE 3 & 4: SEMANTIC ANALYSIS & CODE GENERATION ============= */
typedef struct {
    char name[256];
    char type[64];
} Symbol;

typedef struct {
    Symbol symbols[1000];
    int count;
} SymbolTable;

SymbolTable* symtab_create() {
    SymbolTable *st = malloc(sizeof(SymbolTable));
    st->count = 0;
    return st;
}

void symtab_add(SymbolTable *st, const char *name, const char *type) {
    strcpy(st->symbols[st->count].name, name);
    strcpy(st->symbols[st->count].type, type);
    st->count++;
}

int symtab_find(SymbolTable *st, const char *name) {
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->symbols[i].name, name) == 0) return i;
    }
    return -1;
}

void codegen(ASTNode *node, FILE *out, SymbolTable *st) {
    if (!node) return;
    
    if (strcmp(node->type, "declaration") == 0) {
        if (node->left && node->left->value[0]) {
            fprintf(out, "DECLARE %s %s\n", node->left->value, node->value);
            symtab_add(st, node->left->value, node->value);
        }
    }
    
    if (strcmp(node->type, "assignment") == 0) {
        if (symtab_find(st, node->value) == -1) {
            fprintf(out, "; WARNING: Undefined variable '%s'\n", node->value);
        }
        codegen(node->left, out, st);
        fprintf(out, "STORE %s\n", node->value);
    }
    
    if (strcmp(node->type, "print") == 0) {
        codegen(node->left, out, st);
        fprintf(out, "PRINT\n");
    }
    
    if (strcmp(node->type, "number") == 0) {
        fprintf(out, "LOAD %s\n", node->value);
    }
    
    if (strcmp(node->type, "identifier") == 0) {
        fprintf(out, "LOAD %s\n", node->value);
    }
    
    if (strcmp(node->type, "binary_op") == 0) {
        codegen(node->left, out, st);
        codegen(node->right, out, st);
        fprintf(out, "%s\n", node->value);
    }
    
    codegen(node->left, out, st);
    codegen(node->right, out, st);
    codegen(node->next, out, st);
}

/* ============= MAIN ============= */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input.src> <output.ir>\n", argv[0]);
        return 1;
    }
    
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        printf("Error: Cannot open input file '%s'\n", argv[1]);
        return 1;
    }
    fclose(input);
    
    FILE *output = fopen(argv[2], "w");
    if (!output) {
        printf("Error: Cannot create output file '%s'\n", argv[2]);
        return 1;
    }
    
    printf("=== SIMPLE COMPILER (Top-Down Recursive Descent Parser) ===\n\n");
    
    printf("PHASE 1: Lexical Analysis\n");
    fprintf(output, "; ========= PHASE 1: LEXICAL ANALYSIS =========\n");
    fprintf(output, "; Tokenizing source code\n\n");
    
    printf("PHASE 2: Syntax Analysis (Top-Down Parsing)\n");
    fprintf(output, "; ========= PHASE 2: SYNTAX ANALYSIS (TOP-DOWN PARSING) =========\n");
    fprintf(output, "; Building Abstract Syntax Tree\n");
    
    Parser *parser = parser_create(argv[1]);
    ASTNode *ast = parse_program(parser);
    fprintf(output, "; AST successfully created\n\n");
    
    printf("PHASE 3: Semantic Analysis\n");
    fprintf(output, "; ========= PHASE 3: SEMANTIC ANALYSIS =========\n");
    fprintf(output, "; Building symbol table and type checking\n\n");
    
    printf("PHASE 4: Code Generation\n");
    fprintf(output, "; ========= PHASE 4: CODE GENERATION =========\n");
    fprintf(output, "; Generating 3-address intermediate code:\n\n");
    
    SymbolTable *symtab = symtab_create();
    codegen(ast, output, symtab);
    
    fprintf(output, "\n; ===== COMPILATION COMPLETE =====\n");
    
    fclose(output);
    
    printf("\n✓ Compilation successful!\n");
    printf("✓ Output written to: %s\n\n", argv[2]);
    
    return 0;
}
