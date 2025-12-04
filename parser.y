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
int semanticErrors = 0;

// ==========================================
// TABLA DE SIMBOLOS (Validación Semántica)
// ==========================================
typedef struct Symbol {
    char* name;
    int type; 
    struct Symbol* next;
} Symbol;

Symbol* symbolTable = NULL;

void addSymbol(char* name, int type) {
    Symbol* s = malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->type = type;
    s->next = symbolTable;
    symbolTable = s;
}

Symbol* findSymbol(char* name) {
    Symbol* current = symbolTable;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) return current;
        current = current->next;
    }
    return NULL;
}

// ==========================================
// CREACIÓN DE NODOS
// ==========================================
ASTNode* newInt(int val) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_INT; n->intValue = val; return n;
}
ASTNode* newId(char* name) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_ID; n->idName = strdup(name); return n;
}
ASTNode* newAssign(char* name, ASTNode* val, int isDecl) {
    ASTNode* n = malloc(sizeof(ASTNode)); 
    n->type = NODE_ASSIGN; 
    n->idName = strdup(name); 
    n->left = val; 
    n->intValue = isDecl; // 1=Declaración, 0=Asignación normal
    return n;
}
ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_PIXEL; n->left = x; n->right = y; n->extra = c; return n;
}
ASTNode* newKey(int k, char* d) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_KEY; n->intValue = k; n->idName = strdup(d); return n;
}
ASTNode* newSeq(ASTNode* f, ASTNode* s) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_SEQ; n->left = f; n->right = s; return n;
}
ASTNode* newBinOp(NodeType op, ASTNode* l, ASTNode* r) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = op; n->left = l; n->right = r; return n;
}
ASTNode* newIf(ASTNode* c, ASTNode* b) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_IF; n->cond = c; n->body = b; return n;
}
ASTNode* newIfElse(ASTNode* c, ASTNode* b, ASTNode* e) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_IF_ELSE; n->cond = c; n->body = b; n->elseBody = e; return n;
}
ASTNode* newWhile(ASTNode* c, ASTNode* b) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_WHILE; n->cond = c; n->body = b; return n;
}
ASTNode* newBlock(ASTNode* b) {
    ASTNode* n = malloc(sizeof(ASTNode)); n->type = NODE_BLOCK; n->body = b; return n;
}

// ==========================================
// CHEQUEO SEMÁNTICO (RECORRIDO)
// ==========================================
void checkSemantics(ASTNode* node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_SEQ: 
            checkSemantics(node->left); 
            checkSemantics(node->right); 
            break;
        case NODE_BLOCK: 
            checkSemantics(node->body); 
            break;
        case NODE_ASSIGN:
            checkSemantics(node->left); // Revisar la expresión a asignar
            if (node->intValue == 1) { 
                // DECLARACIÓN (int x = 0)
                if (findSymbol(node->idName)) {
                    fprintf(stderr, "Error Semantico: Variable '%s' re-definida.\n", node->idName);
                    semanticErrors++;
                } else {
                    addSymbol(node->idName, 0); // Registrar variable
                }
            } else {
                // ASIGNACIÓN (x = 5)
                if (!findSymbol(node->idName)) {
                    fprintf(stderr, "Error Semantico: Variable '%s' no declarada.\n", node->idName);
                    semanticErrors++;
                }
            }
            break;
        case NODE_ID:
        case NODE_KEY: // KEY lee en una variable, debe existir
            if (!findSymbol(node->idName)) {
                fprintf(stderr, "Error Semantico: Variable '%s' usada sin declarar.\n", node->idName);
                semanticErrors++;
            }
            break;
        case NODE_IF:
        case NODE_WHILE:
            checkSemantics(node->cond);
            checkSemantics(node->body);
            break;
        case NODE_IF_ELSE:
            checkSemantics(node->cond);
            checkSemantics(node->body);
            checkSemantics(node->elseBody);
            break;
        case NODE_ADD: case NODE_SUB: case NODE_MUL: case NODE_DIV: case NODE_MOD:
        case NODE_LT: case NODE_GT: case NODE_LTE: case NODE_GTE: case NODE_EQ:
        case NODE_NEQ: case NODE_AND: case NODE_OR: case NODE_PIXEL:
            checkSemantics(node->left);
            checkSemantics(node->right);
            if(node->extra) checkSemantics(node->extra);
            break;
        default: break;
    }
}

// ==========================================
// GENERACIÓN DE CÓDIGO
// ==========================================
char* newTemp() {
    char* t = malloc(10); sprintf(t, "t%d", tempCount++); return t;
}

char* genExpr(ASTNode* node) {
    if(!node) return "0";
    if(node->type == NODE_INT) { char* b=malloc(20); sprintf(b,"%d", node->intValue); return b; }
    if(node->type == NODE_ID) return node->idName;
    
    char* l = genExpr(node->left);
    char* r = genExpr(node->right);
    char* t = newTemp();
    char* op = NULL;
    
    switch(node->type) {
        case NODE_ADD: op="ADD"; break; case NODE_SUB: op="SUB"; break;
        case NODE_MUL: op="MUL"; break; case NODE_DIV: op="DIV"; break;
        case NODE_LT: op="LT"; break;   case NODE_GT: op="GT"; break;
        case NODE_EQ: op="EQ"; break;   case NODE_AND: op="MUL"; break; // Simplificado
        case NODE_OR: op="ADD"; break;  // Simplificado
        default: return "0";
    }
    printf("%s %s %s %s\n", op, l, r, t);
    return t;
}

void genCode(ASTNode* node) {
    if (!node) return;
    switch(node->type) {
        case NODE_SEQ: genCode(node->left); genCode(node->right); break;
        case NODE_BLOCK: genCode(node->body); break;
        case NODE_ASSIGN: 
            printf("ASSIGN %s %s\n", genExpr(node->left), node->idName); 
            break;
        case NODE_PIXEL:
            printf("PIXEL %s %s %s\n", genExpr(node->left), genExpr(node->right), genExpr(node->extra));
            break;
        case NODE_KEY:
            printf("KEY %d %s\n", node->intValue, node->idName);
            break;
        case NODE_IF: {
            char lEnd[10]; sprintf(lEnd,"L%d", labelCount++);
            printf("IFFALSE %s GOTO %s\n", genExpr(node->cond), lEnd);
            genCode(node->body);
            printf("LABEL %s\n", lEnd);
            break;
        }
        case NODE_WHILE: {
            char lStart[10], lEnd[10];
            sprintf(lStart,"L%d", labelCount++); sprintf(lEnd,"L%d", labelCount++);
            printf("LABEL %s\n", lStart);
            printf("IFFALSE %s GOTO %s\n", genExpr(node->cond), lEnd);
            genCode(node->body);
            printf("GOTO %s\n", lStart);
            printf("LABEL %s\n", lEnd);
            break;
        }
        // ... (IF_ELSE sigue lógica similar)
    }
}

%}

%union { int intValue; float floatValue; char* idName; struct ASTNode* node; }
%token <intValue> NUMBER BOOL_VAL
%token <idName> ID
%token TYPE_INT KW_PIXEL KW_KEY KW_IF KW_ELSE KW_WHILE
%token ASSIGN SEMI LPAREN RPAREN LBRACE RBRACE 
%token PLUS MINUS MULT DIV LT GT EQ AND OR

%left OR
%left AND
%left EQ LT GT
%left PLUS MINUS
%left MULT DIV

%type <node> program stmt_list stmt block expr comparison

%%

program: stmt_list { root = $1; };

stmt_list: stmt { $$ = $1; } | stmt_list stmt { $$ = newSeq($1, $2); };

block: LBRACE stmt_list RBRACE { $$ = newBlock($2); } | LBRACE RBRACE { $$ = NULL; };

stmt:
    TYPE_INT ID ASSIGN expr SEMI { $$ = newAssign($2, $4, 1); }
    | ID ASSIGN expr SEMI        { $$ = newAssign($1, $3, 0); }
    | KW_PIXEL expr expr expr SEMI { $$ = newPixel($2, $3, $4); }
    | KW_KEY NUMBER ID SEMI      { $$ = newKey($2, $3); }
    | KW_IF LPAREN comparison RPAREN block { $$ = newIf($3, $5); }
    | KW_WHILE LPAREN comparison RPAREN block { $$ = newWhile($3, $5); }
    | block { $$ = $1; }
    ;

expr:
    expr PLUS expr { $$ = newBinOp(NODE_ADD, $1, $3); }
    | expr MINUS expr { $$ = newBinOp(NODE_SUB, $1, $3); }
    | expr MULT expr { $$ = newBinOp(NODE_MUL, $1, $3); }
    | NUMBER { $$ = newInt($1); }
    | ID { $$ = newId($1); }
    ;

comparison:
    expr LT expr { $$ = newBinOp(NODE_LT, $1, $3); }
    | expr GT expr { $$ = newBinOp(NODE_GT, $1, $3); }
    | expr EQ expr { $$ = newBinOp(NODE_EQ, $1, $3); }
    | comparison AND comparison { $$ = newBinOp(NODE_AND, $1, $3); }
    | expr { $$ = $1; } 
    ;

%%

int main(int argc, char** argv) {
    if (argc > 1) yyin = fopen(argv[1], "r");
    if (!yyin) return 1;

    if (yyparse() == 0) {
        checkSemantics(root);
        if (semanticErrors > 0) {
            fprintf(stderr, "%d Errores semanticos. Abortando.\n", semanticErrors);
            return 1;
        }
        printf("// Codigo FIS-25 Generado\n");
        genCode(root);
    }
    return 0;
}
void yyerror(const char *s) { fprintf(stderr, "Error: %s\n", s); }