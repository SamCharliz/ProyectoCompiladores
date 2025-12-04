%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex();
void yyerror(const char *s);
extern FILE* yyin;
extern int yylineno;

ASTNode* root;
int labelCount = 0;
int tempCount = 0;

// --- Funciones de creación de nodos AST ---

ASTNode* newInt(int val) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_INT;
    node->intValue = val;
    return node;
}

ASTNode* newId(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ID;
    node->idName = strdup(name);
    return node;
}

ASTNode* newAssign(char* name, ASTNode* val) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    node->idName = strdup(name);
    node->left = val;
    return node;
}

ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_PIXEL;
    node->left = x;
    node->right = y;
    node->extra = c;
    return node;
}

ASTNode* newKey(int keyVal, char* dest) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_KEY;
    node->intValue = keyVal;
    node->idName = strdup(dest);
    return node;
}

ASTNode* newSeq(ASTNode* first, ASTNode* second) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_SEQ;
    node->left = first;
    node->right = second;
    return node;
}

ASTNode* newBinOp(NodeType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = op;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* newIf(ASTNode* cond, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->cond = cond;
    node->body = body;
    return node;
}

ASTNode* newIfElse(ASTNode* cond, ASTNode* body, ASTNode* elseBody) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_IF_ELSE;
    node->cond = cond;
    node->body = body;
    node->elseBody = elseBody;
    return node;
}

ASTNode* newWhile(ASTNode* cond, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->cond = cond;
    node->body = body;
    return node;
}

ASTNode* newBlock(ASTNode* stmts) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->body = stmts;
    return node;
}

// Generador de temporales
char* newTemp() {
    char* temp = malloc(10);
    sprintf(temp, "t%d", tempCount++);
    return temp;
}

// Generación de expresiones
char* generateExpr(ASTNode* node) {
    if (!node) return "0";
    
    switch(node->type) {
        case NODE_INT: {
            char* buf = malloc(20);
            sprintf(buf, "%d", node->intValue);
            return buf;
        }
        case NODE_ID:
            return node->idName;
        
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD:
        case NODE_LT:
        case NODE_GT:
        case NODE_LTE:
        case NODE_GTE:
        case NODE_EQ:
        case NODE_NEQ:
        case NODE_AND:
        case NODE_OR: {
            char* t = newTemp();
            char* l = generateExpr(node->left);
            char* r = generateExpr(node->right);
            char* op = "";
            switch(node->type) {
                case NODE_ADD: op = "ADD"; break;
                case NODE_SUB: op = "SUB"; break;
                case NODE_MUL: op = "MUL"; break;
                case NODE_DIV: op = "DIV"; break;
                case NODE_MOD: op = "MOD"; break;
                case NODE_LT:  op = "LT"; break;
                case NODE_GT:  op = "GT"; break;
                case NODE_LTE: op = "LTE"; break;
                case NODE_GTE: op = "GTE"; break;
                case NODE_EQ:  op = "EQ"; break;
                case NODE_NEQ: op = "NEQ"; break;
                case NODE_AND: op = "MUL"; break; // Lógica AND
                case NODE_OR:  op = "ADD"; break; // Lógica OR
                default: break;
            }
            printf("%s %s %s %s\n", op, l, r, t);
            return t;
        }
        default: return "0";
    }
}

// Generación de sentencias
void generateCode(ASTNode* node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_SEQ:
            generateCode(node->left);
            generateCode(node->right);
            break;
            
        case NODE_BLOCK:
            generateCode(node->body);
            break;
            
        case NODE_ASSIGN: {
            char* val = generateExpr(node->left);
            printf("ASSIGN %s %s\n", val, node->idName);
            break;
        }
        
        case NODE_PIXEL: {
            char* x = generateExpr(node->left);
            char* y = generateExpr(node->right);
            char* c = generateExpr(node->extra);
            printf("PIXEL %s %s %s\n", x, y, c);
            break;
        }

        case NODE_KEY: {
            printf("KEY %d %s\n", node->intValue, node->idName);
            break;
        }
        
        case NODE_IF: {
            char labelEnd[20];
            sprintf(labelEnd, "L%d", labelCount++);
            char* cond = generateExpr(node->cond);
            printf("IFFALSE %s GOTO %s\n", cond, labelEnd);
            generateCode(node->body);
            printf("LABEL %s\n", labelEnd);
            break;
        }

        case NODE_IF_ELSE: {
            char labelElse[20], labelEnd[20];
            sprintf(labelElse, "L%d", labelCount++);
            sprintf(labelEnd, "L%d", labelCount++);
            char* cond = generateExpr(node->cond);
            printf("IFFALSE %s GOTO %s\n", cond, labelElse);
            generateCode(node->body);
            printf("GOTO %s\n", labelEnd);
            printf("LABEL %s\n", labelElse);
            generateCode(node->elseBody);
            printf("LABEL %s\n", labelEnd);
            break;
        }
        
        case NODE_WHILE: {
            char labelStart[20], labelEnd[20];
            sprintf(labelStart, "L%d", labelCount++);
            sprintf(labelEnd, "L%d", labelCount++);
            printf("LABEL %s\n", labelStart);
            char* cond = generateExpr(node->cond);
            printf("IFFALSE %s GOTO %s\n", cond, labelEnd);
            generateCode(node->body);
            printf("GOTO %s\n", labelStart);
            printf("LABEL %s\n", labelEnd);
            break;
        }
    }
}
%}

%union {
    int intValue;
    float floatValue;
    char* idName;
    struct ASTNode* node;
}

%token <intValue> NUMBER BOOL_VAL
%token <floatValue> FLOAT_NUM
// Se agregó STRING_LIT aquí abajo
%token <idName> ID STRING_LIT
%token TYPE_INT TYPE_FLOAT TYPE_BOOL TYPE_STRING
%token KW_PIXEL KW_KEY KW_IF KW_ELSE KW_WHILE KW_FOR KW_FUNCTION KW_RETURN KW_INPUT KW_PRINT
%token ASSIGN SEMI COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET DOT
%token PLUS MINUS MULT DIV MOD
%token EQ NEQ LT GT LTE GTE AND OR NOT

/* Precedencia corregida */
%left OR
%left AND
%left EQ NEQ LT GT LTE GTE
%left PLUS MINUS
%left MULT DIV MOD

%type <node> program stmt_list stmt block expr comparison

%%

program:
    stmt_list { root = $1; }
    ;

stmt_list:
    stmt { $$ = $1; }
    | stmt_list stmt { $$ = newSeq($1, $2); }
    ;

block:
    LBRACE stmt_list RBRACE { $$ = newBlock($2); }
    | LBRACE RBRACE { $$ = NULL; }
    ;

stmt:
    TYPE_INT ID ASSIGN expr SEMI { $$ = newAssign($2, $4); }
    | ID ASSIGN expr SEMI { $$ = newAssign($1, $3); }
    | KW_PIXEL expr expr expr SEMI { $$ = newPixel($2, $3, $4); }
    | KW_KEY NUMBER ID SEMI { $$ = newKey($2, $3); }
    | KW_IF LPAREN comparison RPAREN block { $$ = newIf($3, $5); }
    | KW_IF LPAREN comparison RPAREN block KW_ELSE block { $$ = newIfElse($3, $5, $7); }
    | KW_WHILE LPAREN comparison RPAREN block { $$ = newWhile($3, $5); }
    // Reglas "dummy" para INPUT y PRINT para evitar errores de sintaxis si se usan
    | KW_INPUT ID SEMI { /* INPUT no implementado completamente en AST para Pong, pero permite compilar */ }
    | KW_PRINT expr SEMI { /* PRINT no implementado completamente */ }
    | block { $$ = $1; }
    ;

expr:
    expr PLUS expr { $$ = newBinOp(NODE_ADD, $1, $3); }
    | expr MINUS expr { $$ = newBinOp(NODE_SUB, $1, $3); }
    | expr MULT expr { $$ = newBinOp(NODE_MUL, $1, $3); }
    | expr DIV expr { $$ = newBinOp(NODE_DIV, $1, $3); }
    | NUMBER { $$ = newInt($1); }
    | ID { $$ = newId($1); }
    | BOOL_VAL { $$ = newInt($1); }
    // Regla para strings para que no falle el parser, aunque Pong no los use
    | STRING_LIT { $$ = newId($1); } 
    ;

comparison:
    comparison AND comparison { $$ = newBinOp(NODE_AND, $1, $3); }
    | comparison OR comparison { $$ = newBinOp(NODE_OR, $1, $3); }
    | expr EQ expr { $$ = newBinOp(NODE_EQ, $1, $3); }
    | expr NEQ expr { $$ = newBinOp(NODE_NEQ, $1, $3); }
    | expr LT expr { $$ = newBinOp(NODE_LT, $1, $3); }
    | expr GT expr { $$ = newBinOp(NODE_GT, $1, $3); }
    | expr LTE expr { $$ = newBinOp(NODE_LTE, $1, $3); }
    | expr GTE expr { $$ = newBinOp(NODE_GTE, $1, $3); }
    | expr { $$ = $1; }
    | LPAREN comparison RPAREN { $$ = $2; }
    ;

%%

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Error al abrir archivo\n");
            return 1;
        }
    }
    
    // Encabezado para el simulador
    printf("// Codigo FIS-25 generado\n");
    
    if (yyparse() == 0) {
        generateCode(root);
    }
    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Error linea %d: %s\n", yylineno, s);
}