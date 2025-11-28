/* parser.y */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex();
void yyerror(const char *s);
ASTNode* root; // Raíz del árbol
%}

%union {
    int intValue;
    char* idName;
    struct ASTNode* node;
}

%token <intValue> NUMBER
%token <idName> ID
%token TYPE_INT KW_PIXEL KW_KEY KW_IF ASSIGN SEMI

%type <node> program stmt_list stmt expr

%%

program:
    stmt_list { root = $1; }
    ;

stmt_list:
    stmt { $$ = $1; }
    | stmt_list stmt { $$ = newSeq($1, $2); }
    ;

stmt:
    /* Asignación de variables */
    TYPE_INT ID ASSIGN expr SEMI { $$ = newAssign($2, $4); }
    
    /* Instrucción PIXEL x y c [cite: 25] */
    | KW_PIXEL expr expr expr SEMI { $$ = newPixel($2, $3, $4); }
    
    /* Instrucción KEY k dest [cite: 26] */
    | KW_KEY NUMBER ID SEMI { $$ = newKey($2, $3); }
    ;

expr:
    NUMBER { $$ = newInt($1); }
    | ID   { $$ = newId($1); }
    ;

%%

/* --- Implementación de Funciones C --- */

ASTNode* newInt(int val) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_INT; n->intValue = val; return n;
}

ASTNode* newId(char* name) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_ID; n->idName = name; return n;
}

ASTNode* newAssign(char* name, ASTNode* val) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_ASSIGN; n->idName = name; n->left = val; return n;
}

ASTNode* newPixel(ASTNode* x, ASTNode* y, ASTNode* c) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_PIXEL; n->left = x; n->right = y; n->extra = c; return n;
}

ASTNode* newKey(int keyParam, char* destVar) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_KEY; n->intValue = keyParam; n->idName = destVar; return n;
}

ASTNode* newSeq(ASTNode* first, ASTNode* second) {
    ASTNode* n = malloc(sizeof(ASTNode));
    n->type = NODE_SEQ; n->left = first; n->right = second; return n;
}

/* --- GENERADOR DE CÓDIGO (BACK-END) --- */
/* Recorre el árbol e imprime instrucciones FIS-25 */
void generateCode(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_SEQ:
            generateCode(node->left);
            generateCode(node->right);
            break;

        case NODE_ASSIGN:
            // Simplificación: Asumimos expr es un número directo
            // En un caso real, evaluas la expr y mueves a registro
            if(node->left->type == NODE_INT) {
                printf("MOV %s, %d\n", node->idName, node->left->intValue);
            }
            break;

        case NODE_PIXEL:
            // Genera: PIXEL x y c
            printf("PIXEL ");
            // Imprime valores directamente (simplificado)
            if(node->left->type == NODE_INT) printf("%d ", node->left->intValue);
            if(node->right->type == NODE_INT) printf("%d ", node->right->intValue);
            if(node->extra->type == NODE_INT) printf("%d\n", node->extra->intValue);
            break;

        case NODE_KEY:
            // Genera: KEY k dest
            printf("KEY %d %s\n", node->intValue, node->idName);
            break;
    }
}

int main() {
    printf("Compilando...\n");
    yyparse();
    printf("--- CODIGO FIS-25 GENERADO ---\n");
    generateCode(root);
    return 0;
}

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}