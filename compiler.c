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

const char* token_type_name(TokenType t) {
    switch(t) {
        case TOKEN_INT: return "INT";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_INPUT: return "INPUT";
        case TOKEN_FUNC: return "FUNC";
        case TOKEN_ID: return "IDENTIFIER";
        case TOKEN_NUM: return "NUMBER";
        case TOKEN_STR: return "STRING";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MUL: return "MULTIPLY";
        case TOKEN_DIV: return "DIVIDE";
        case TOKEN_MOD: return "MODULO";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQ: return "EQUAL";
        case TOKEN_NEQ: return "NOT_EQUAL";
        case TOKEN_LT: return "LESS";
        case TOKEN_GT: return "GREATER";
        case TOKEN_LTE: return "LESS_EQUAL";
        case TOKEN_GTE: return "GREATER_EQUAL";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_EOF: return "EOF";
        default: return "ERROR";
    }
}

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
    int parse_depth;
    FILE *debug_out;
} Parser;

ASTNode* ast_create(const char *type) {
    ASTNode *n = malloc(sizeof(ASTNode));
    strcpy(n->type, type);
    n->value[0] = '\0';
    n->left = n->right = n->next = NULL;
    return n;
}

Parser* parser_create(const char *filename, FILE *debug_file) {
    Parser *p = malloc(sizeof(Parser));
    p->lexer = lexer_create(filename);
    p->current = lexer_next_token(p->lexer);
    p->parse_depth = 0;
    p->debug_out = debug_file;
    return p;
}

void parser_advance(Parser *p) {
    p->current = lexer_next_token(p->lexer);
}

void print_indent(int depth, FILE *out) {
    for (int i = 0; i < depth * 2; i++) fprintf(out, " ");
}

ASTNode* parse_expression(Parser *p);

ASTNode* parse_primary(Parser *p) {
    print_indent(p->parse_depth, p->debug_out);
    fprintf(p->debug_out, ">> parse_primary() - Current token: %s (%s)\n", 
            token_type_name(p->current.type), p->current.lexeme);
    p->parse_depth++;
    
    if (p->current.type == TOKEN_NUM) {
        ASTNode *n = ast_create("number");
        sprintf(n->value, "%d", p->current.value);
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "<< Parsed NUMBER: %s\n", n->value);
        parser_advance(p);
        p->parse_depth--;
        return n;
    }
    
    if (p->current.type == TOKEN_ID) {
        ASTNode *n = ast_create("identifier");
        strcpy(n->value, p->current.lexeme);
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "<< Parsed IDENTIFIER: %s\n", n->value);
        parser_advance(p);
        p->parse_depth--;
        return n;
    }
    
    if (p->current.type == TOKEN_STR) {
        ASTNode *n = ast_create("string");
        strcpy(n->value, p->current.lexeme);
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "<< Parsed STRING: %s\n", n->value);
        parser_advance(p);
        p->parse_depth--;
        return n;
    }
    
    if (p->current.type == TOKEN_LPAREN) {
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[EXPR in PARENS]\n");
        parser_advance(p);
        ASTNode *expr = parse_expression(p);
        if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        p->parse_depth--;
        return expr;
    }
    
    p->parse_depth--;
    return ast_create("empty");
}

ASTNode* parse_term(Parser *p) {
    print_indent(p->parse_depth, p->debug_out);
    fprintf(p->debug_out, ">> parse_term()\n");
    p->parse_depth++;
    
    ASTNode *left = parse_primary(p);
    
    while (p->current.type == TOKEN_MUL || 
           p->current.type == TOKEN_DIV || 
           p->current.type == TOKEN_MOD) {
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[OPERATOR: %s]\n", p->current.lexeme);
        ASTNode *op = ast_create("binary_op");
        strcpy(op->value, p->current.lexeme);
        parser_advance(p);
        op->left = left;
        op->right = parse_primary(p);
        left = op;
    }
    
    p->parse_depth--;
    return left;
}

ASTNode* parse_expression(Parser *p) {
    print_indent(p->parse_depth, p->debug_out);
    fprintf(p->debug_out, ">> parse_expression()\n");
    p->parse_depth++;
    
    ASTNode *left = parse_term(p);
    
    while (p->current.type == TOKEN_PLUS || 
           p->current.type == TOKEN_MINUS) {
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[OPERATOR: %s]\n", p->current.lexeme);
        ASTNode *op = ast_create("binary_op");
        strcpy(op->value, p->current.lexeme);
        parser_advance(p);
        op->left = left;
        op->right = parse_term(p);
        left = op;
    }
    
    p->parse_depth--;
    return left;
}

ASTNode* parse_statement(Parser *p);

ASTNode* parse_block(Parser *p) {
    ASTNode *block = ast_create("block");
    print_indent(p->parse_depth, p->debug_out);
    fprintf(p->debug_out, "[ENTERING BLOCK]\n");
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
    
    if (p->current.type == TOKEN_RBRACE) {
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[EXITING BLOCK]\n");
        parser_advance(p);
    }
    return block;
}

ASTNode* parse_statement(Parser *p) {
    print_indent(p->parse_depth, p->debug_out);
    fprintf(p->debug_out, ">> parse_statement() - Current: %s\n", token_type_name(p->current.type));
    p->parse_depth++;
    
    if (p->current.type == TOKEN_INT || 
        p->current.type == TOKEN_FLOAT || 
        p->current.type == TOKEN_CHAR) {
        
        ASTNode *decl = ast_create("declaration");
        strcpy(decl->value, p->current.lexeme);
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[DECLARATION] type=%s\n", p->current.lexeme);
        parser_advance(p);
        
        if (p->current.type == TOKEN_ID) {
            ASTNode *id = ast_create("identifier");
            strcpy(id->value, p->current.lexeme);
            print_indent(p->parse_depth, p->debug_out);
            fprintf(p->debug_out, "[VARIABLE] name=%s\n", p->current.lexeme);
            decl->left = id;
            parser_advance(p);
        }
        
        if (p->current.type == TOKEN_SEMICOLON) {
            print_indent(p->parse_depth, p->debug_out);
            fprintf(p->debug_out, "[SEMICOLON]\n");
            parser_advance(p);
        }
        
        p->parse_depth--;
        return decl;
    }
    
    if (p->current.type == TOKEN_ID) {
        ASTNode *assign = ast_create("assignment");
        strcpy(assign->value, p->current.lexeme);
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[ASSIGNMENT] var=%s\n", p->current.lexeme);
        parser_advance(p);
        
        if (p->current.type == TOKEN_ASSIGN) {
            print_indent(p->parse_depth, p->debug_out);
            fprintf(p->debug_out, "[ASSIGN OPERATOR]\n");
            parser_advance(p);
            assign->left = parse_expression(p);
        }
        
        if (p->current.type == TOKEN_SEMICOLON) {
            print_indent(p->parse_depth, p->debug_out);
            fprintf(p->debug_out, "[SEMICOLON]\n");
            parser_advance(p);
        }
        
        p->parse_depth--;
        return assign;
    }
    
    if (p->current.type == TOKEN_PRINT) {
        ASTNode *print = ast_create("print");
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[PRINT STATEMENT]\n");
        parser_advance(p);
        
        if (p->current.type == TOKEN_LPAREN) {
            print_indent(p->parse_depth, p->debug_out);
            fprintf(p->debug_out, "[LPAREN]\n");
            parser_advance(p);
            print->left = parse_expression(p);
            if (p->current.type == TOKEN_RPAREN) {
                print_indent(p->parse_depth, p->debug_out);
                fprintf(p->debug_out, "[RPAREN]\n");
                parser_advance(p);
            }
        }
        
        if (p->current.type == TOKEN_SEMICOLON) {
            print_indent(p->parse_depth, p->debug_out);
            fprintf(p->debug_out, "[SEMICOLON]\n");
            parser_advance(p);
        }
        
        p->parse_depth--;
        return print;
    }
    
    if (p->current.type == TOKEN_IF) {
        ASTNode *ifstmt = ast_create("if");
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[IF STATEMENT]\n");
        parser_advance(p);
        
        if (p->current.type == TOKEN_LPAREN) {
            parser_advance(p);
            ifstmt->left = parse_expression(p);
            if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        }
        
        if (p->current.type == TOKEN_LBRACE) {
            ifstmt->right = parse_block(p);
        }
        
        p->parse_depth--;
        return ifstmt;
    }
    
    if (p->current.type == TOKEN_WHILE) {
        ASTNode *whilestmt = ast_create("while");
        print_indent(p->parse_depth, p->debug_out);
        fprintf(p->debug_out, "[WHILE STATEMENT]\n");
        parser_advance(p);
        
        if (p->current.type == TOKEN_LPAREN) {
            parser_advance(p);
            whilestmt->left = parse_expression(p);
            if (p->current.type == TOKEN_RPAREN) parser_advance(p);
        }
        
        if (p->current.type == TOKEN_LBRACE) {
            whilestmt->right = parse_block(p);
        }
        
        p->parse_depth--;
        return whilestmt;
    }
    
    p->parse_depth--;
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

void symtab_add(SymbolTable *st, const char *name, const char *type, FILE *debug) {
    strcpy(st->symbols[st->count].name, name);
    strcpy(st->symbols[st->count].type, type);
    fprintf(debug, "  [SYMBOL] Adding: %s (type: %s)\n", name, type);
    st->count++;
}

int symtab_find(SymbolTable *st, const char *name) {
    for (int i = 0; i < st->count; i++) {
        if (strcmp(st->symbols[i].name, name) == 0) return i;
    }
    return -1;
}

void codegen(ASTNode *node, FILE *out, SymbolTable *st, FILE *debug) {
    if (!node) return;
    
    if (strcmp(node->type, "declaration") == 0) {
        if (node->left && node->left->value[0]) {
            fprintf(out, "DECLARE %s %s\n", node->left->value, node->value);
            fprintf(debug, "  [GEN] DECLARE %s %s\n", node->left->value, node->value);
        }
    }
    
    if (strcmp(node->type, "assignment") == 0) {
        if (symtab_find(st, node->value) == -1) {
            fprintf(debug, "  [ERROR] Undefined variable: %s\n", node->value);
        }
        codegen(node->left, out, st, debug);
        fprintf(out, "STORE %s\n", node->value);
        fprintf(debug, "  [GEN] STORE %s\n", node->value);
    }
    
    if (strcmp(node->type, "print") == 0) {
        codegen(node->left, out, st, debug);
        fprintf(out, "PRINT\n");
        fprintf(debug, "  [GEN] PRINT\n");
    }
    
    if (strcmp(node->type, "number") == 0) {
        fprintf(out, "LOAD %s\n", node->value);
        fprintf(debug, "  [GEN] LOAD %s\n", node->value);
    }
    
    if (strcmp(node->type, "identifier") == 0) {
        fprintf(out, "LOAD %s\n", node->value);
        fprintf(debug, "  [GEN] LOAD %s\n", node->value);
    }
    
    if (strcmp(node->type, "binary_op") == 0) {
        codegen(node->left, out, st, debug);
        codegen(node->right, out, st, debug);
        fprintf(out, "%s\n", node->value);
        fprintf(debug, "  [GEN] %s\n", node->value);
    }
    
    codegen(node->left, out, st, debug);
    codegen(node->right, out, st, debug);
    codegen(node->next, out, st, debug);
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
    
    printf("\n===== SIMPLE COMPILER (Top-Down Recursive Descent Parser) =====\n\n");
    
    /* ===== PHASE 1: LEXICAL ANALYSIS ===== */
    printf("===== PHASE 1: LEXICAL ANALYSIS =====\n");
    fprintf(output, "; ===== PHASE 1: LEXICAL ANALYSIS =====\n");
    fprintf(output, "; Tokenizing source code\n\n");
    
    Lexer *lexer = lexer_create(argv[1]);
    Token t;
    int token_count = 0;
    
    printf("Tokenizing file: %s\n", argv[1]);
    do {
        t = lexer_next_token(lexer);
        if (t.type != TOKEN_EOF && t.type != TOKEN_ERROR) {
            printf("[TOKEN %d] Type: %-15s Value: %-20s Line: %d\n", 
                   token_count + 1, token_type_name(t.type), t.lexeme, t.line);
            fprintf(output, "; TOKEN %d: %s = %s (line %d)\n", 
                    token_count + 1, token_type_name(t.type), t.lexeme, t.line);
            token_count++;
        }
    } while (t.type != TOKEN_EOF);
    lexer_destroy(lexer);
    
    printf("\nTotal tokens found: %d\n\n", token_count);
    fprintf(output, "\n; Total tokens: %d\n\n", token_count);
    
    /* ===== PHASE 2: SYNTAX ANALYSIS ===== */
    printf("===== PHASE 2: SYNTAX ANALYSIS (TOP-DOWN PARSING) =====\n");
    fprintf(output, "; ===== PHASE 2: SYNTAX ANALYSIS (TOP-DOWN PARSING) =====\n");
    fprintf(output, "; Building Abstract Syntax Tree\n\n");
    
    Parser *parser = parser_create(argv[1], stdout);
    printf("Parsing...\n");
    ASTNode *ast = parse_program(parser);
    
    printf("\n[AST CONSTRUCTION COMPLETE]\n\n");
    fprintf(output, "; AST successfully created\n\n");
    
    /* ===== PHASE 3: SEMANTIC ANALYSIS ===== */
    printf("===== PHASE 3: SEMANTIC ANALYSIS =====\n");
    fprintf(output, "; ===== PHASE 3: SEMANTIC ANALYSIS =====\n");
    fprintf(output, "; Building symbol table and type checking\n\n");
    
    SymbolTable *symtab = symtab_create();
    printf("Building symbol table...\n");
    
    // Traverse AST and add symbols
    ASTNode *current = ast->left;
    while (current) {
        if (strcmp(current->type, "declaration") == 0) {
            if (current->left && current->left->value[0]) {
                symtab_add(symtab, current->left->value, current->value, stdout);
            }
        }
        current = current->next;
    }
    
    printf("\nSymbol table complete. Total symbols: %d\n\n", symtab->count);
    fprintf(output, "; Symbol table: %d variables\n", symtab->count);
    for (int i = 0; i < symtab->count; i++) {
        fprintf(output, ";   %s (%s)\n", symtab->symbols[i].name, symtab->symbols[i].type);
    }
    fprintf(output, "\n");
    
    /* ===== PHASE 4: CODE GENERATION ===== */
    printf("===== PHASE 4: CODE GENERATION =====\n");
    fprintf(output, "; ===== PHASE 4: CODE GENERATION =====\n");
    fprintf(output, "; Generating 3-address intermediate code\n\n");
    
    printf("Generating intermediate code...\n");
    codegen(ast, output, symtab, stdout);
    
    fprintf(output, "\n; ===== COMPILATION COMPLETE =====\n");
    
    fclose(output);
    
    printf("\n===== COMPILATION SUCCESSFUL! =====\n");
    printf("Output written to: %s\n\n", argv[2]);
    
    return 0;
}
