/* parser.y - Generador completo FIS-25 */
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
Symbol symTable[100];
int symCount = 0;
int labelCount = 0;
int tempCount = 0;
%}

%union {
    int intValue;
    float floatValue;
    char* idName;
    struct ASTNode* node;
}

%token <intValue> NUMBER
%token <floatValue> FLOAT_NUM
%token <idName> ID
%token TYPE_INT TYPE_FLOAT KW_PIXEL KW_KEY KW_IF KW_ELSE KW_WHILE
%token ASSIGN SEMI COMMA LPAREN RPAREN LBRACE RBRACE
%token PLUS MINUS MULT DIV MOD
%token EQ NEQ LT GT LTE GTE

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
    /* Declaración y asignación */
    TYPE_INT ID ASSIGN expr SEMI { $$ = newAssign($2, $4); }
    | TYPE_FLOAT ID ASSIGN expr SEMI { $$ = newAssign($2, $4); }
    
    /* Asignación simple */
    | ID ASSIGN expr SEMI { $$ = newAssign($1, $3); }
    
    /* PIXEL x y c */
    | KW_PIXEL expr expr expr SEMI { $$ = newPixel($2, $3, $4); }
    
    /* KEY k dest */
    | KW_KEY NUMBER ID SEMI { $$ = newKey($2, $3); }
    
    /* IF con condición */
    | KW_IF LPAREN comparison RPAREN block { $$ = newIf($3, $5); }
    
    /* IF-ELSE con condición */
    | KW_IF LPAREN comparison RPAREN block KW_ELSE block { $$ = newIfElse($3, $5, $7); }
    
    /* WHILE con condición */
    | KW_WHILE LPAREN comparison RPAREN block { $$ = newWhile($3, $5); }
    ;

comparison:
    expr EQ expr { $$ = newBinOp(NODE_EQ, $1, $3); }
    | expr NEQ expr { $$ = newBinOp(NODE_NEQ, $1, $3); }
    | expr LT expr { $$ = newBinOp(NODE_LT, $1, $3); }
    | expr GT expr { $$ = newBinOp(NODE_GT, $1, $3); }
    | expr LTE expr { $$ = newBinOp(NODE_LTE, $1, $3); }
    | expr GTE expr { $$ = newBinOp(NODE_GTE, $1, $3); }
    | expr { $$ = $1; }
    ;

expr:
    NUMBER { $$ = newInt($1); }
    | FLOAT_NUM { $$ = newFloat($1); }
    | ID { $$ = newId($1); }
    | expr PLUS expr { $$ = newBinOp(NODE_ADD, $1, $3); }
    | expr MINUS expr { $$ = newBinOp(NODE_SUB, $1, $3); }
    | expr MULT expr { $$ = newBinOp(NODE_MUL, $1, $3); }
    | expr DIV expr { $$ = newBinOp(NODE_DIV, $1, $3); }
    | expr MOD expr { $$ = newBinOp(NODE_MOD, $1, $3); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

%%

/* === IMPLEMENTACIÓN === */

ASTNode* newInt(int val) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_INT;
    n->intValue = val;
    n->floatValue = 0.0;
    n->left = n->right = n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newFloat(float val) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_FLOAT;
    n->floatValue = val;
    n->intValue = 0;
    n->left = n->right = n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newId(char* name) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_ID;
    n->idName = name;
    n->left = n->right = n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newAssign(char* name, ASTNode* val) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_ASSIGN;
    n->idName = name;
    n->left = val;
    n->right = n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_PIXEL;
    n->left = x;
    n->right = y;
    n->extra = c;
    n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newKey(int keyParam, char* destVar) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_KEY;
    n->intValue = keyParam;
    n->idName = destVar;
    n->left = n->right = n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newSeq(ASTNode* first, ASTNode* second) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_SEQ;
    n->left = first;
    n->right = second;
    n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newBinOp(NodeType op, ASTNode* left, ASTNode* right) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = op;
    n->left = left;
    n->right = right;
    n->extra = n->cond = n->body = n->elseBody = NULL;
    return n;
}

ASTNode* newIf(ASTNode* cond, ASTNode* body) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_IF;
    n->cond = cond;
    n->body = body;
    n->left = n->right = n->extra = n->elseBody = NULL;
    return n;
}

ASTNode* newIfElse(ASTNode* cond, ASTNode* body, ASTNode* elseBody) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_IF_ELSE;
    n->cond = cond;
    n->body = body;
    n->elseBody = elseBody;
    n->left = n->right = n->extra = NULL;
    return n;
}

ASTNode* newWhile(ASTNode* cond, ASTNode* body) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_WHILE;
    n->cond = cond;
    n->body = body;
    n->left = n->right = n->extra = n->elseBody = NULL;
    return n;
}

ASTNode* newBlock(ASTNode* stmts) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_BLOCK;
    n->body = stmts;
    n->left = n->right = n->extra = n->cond = n->elseBody = NULL;
    return n;
}

/* === TABLA DE SÍMBOLOS === */

void declareVar(char* name) {
    for (int i = 0; i < symCount; i++) {
        if (strcmp(symTable[i].name, name) == 0)
            return; // Ya existe
    }
    symTable[symCount].name = strdup(name);
    symCount++;
    printf("VAR %s\n", name);
}

/* === GENERADOR DE TEMPORALES === */

char* newTemp() {
    static char buf[20];
    sprintf(buf, "T%d", tempCount++);
    declareVar(buf);
    return strdup(buf);
}

/* === GENERADOR DE CÓDIGO FIS-25 === */

char* generateExpr(ASTNode* node) {
    if (!node) return NULL;
    
    switch (node->type) {
        case NODE_INT: {
            char* temp = newTemp();
            printf("ASSIGN %d %s\n", node->intValue, temp);
            return temp;
        }
        
        case NODE_FLOAT: {
            char* temp = newTemp();
            printf("ASSIGN %.6f %s\n", node->floatValue, temp);
            return temp;
        }
        
        case NODE_ID:
            return node->idName;
        
        case NODE_ADD: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("ADD %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_SUB: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("SUB %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_MUL: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("MUL %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_DIV: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("DIV %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_MOD: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("MOD %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_EQ: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("EQ %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_NEQ: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("NEQ %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_LT: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("LT %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_GT: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("GT %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_LTE: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("LTE %s %s %s\n", left, right, result);
            return result;
        }
        
        case NODE_GTE: {
            char* left = generateExpr(node->left);
            char* right = generateExpr(node->right);
            char* result = newTemp();
            printf("GTE %s %s %s\n", left, right, result);
            return result;
        }
        
        default:
            return NULL;
    }
}

void generateCode(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_SEQ:
            generateCode(node->left);
            generateCode(node->right);
            break;
            
        case NODE_BLOCK:
            generateCode(node->body);
            break;
            
        case NODE_ASSIGN:
            declareVar(node->idName);
            if (node->left->type == NODE_INT) {
                printf("ASSIGN %d %s\n", node->left->intValue, node->idName);
            } else if (node->left->type == NODE_FLOAT) {
                printf("ASSIGN %.6f %s\n", node->left->floatValue, node->idName);
            } else {
                char* expr = generateExpr(node->left);
                printf("ASSIGN %s %s\n", expr, node->idName);
            }
            break;
            
        case NODE_PIXEL: {
            char* x = generateExpr(node->left);
            char* y = generateExpr(node->right);
            char* c = generateExpr(node->extra);
            printf("PIXEL %s %s %s\n", x, y, c);
            break;
        }
            
        case NODE_KEY:
            declareVar(node->idName);
            printf("KEY %d %s\n", node->intValue, node->idName);
            break;
            
        case NODE_IF: {
            char* cond = generateExpr(node->cond);
            char labelEnd[20];
            sprintf(labelEnd, "L%d", labelCount++);
            
            printf("IFFALSE %s GOTO %s\n", cond, labelEnd);
            generateCode(node->body);
            printf("LABEL %s\n", labelEnd);
            break;
        }
        
        case NODE_IF_ELSE: {
            char* cond = generateExpr(node->cond);
            char labelElse[20], labelEnd[20];
            sprintf(labelElse, "L%d", labelCount++);
            sprintf(labelEnd, "L%d", labelCount++);
            
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

/* === MAIN === */

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Error: No se pudo abrir '%s'\n", argv[1]);
            return 1;
        }
    }
    
    printf("// Compilador FIS-25\n");
    printf("// Codigo intermedio generado\n\n");
    
    if (yyparse() != 0) {
        fprintf(stderr, "Compilación fallida\n");
        return 1;
    }
    
    printf("\n// Programa principal\n");
    generateCode(root);
    
    if (argc > 1) fclose(yyin);
    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Error en línea %d: %s\n", yylineno, s);
}