/* parser.y - Analizador Sintáctico para Compilador FIS-25 (SIN CONFLICTOS) */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex();
void yyerror(const char *s);
extern FILE* yyin;
extern int yylineno;

ASTNode* root = NULL;
%}

%union {
    int intValue;
    float floatValue;
    char* idName;
    struct ASTNode* node;
}

%token <intValue> NUMBER BOOL_VAL
%token <floatValue> FLOAT_NUM
%token <idName> ID STRING_LIT

%token TYPE_INT TYPE_FLOAT TYPE_BOOL TYPE_STRING
%token KW_PIXEL KW_KEY KW_INPUT KW_PRINT
%token KW_IF KW_ELSE KW_WHILE KW_FOR
%token KW_FUNCTION KW_RETURN

%token ASSIGN SEMI COMMA
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET DOT
%token PLUS MINUS MULT DIV MOD
%token EQ NEQ LT GT LTE GTE
%token AND OR NOT

/* Precedencia y asociatividad - ORDEN IMPORTA */
%right ASSIGN
%left OR
%left AND
%nonassoc EQ NEQ
%nonassoc LT GT LTE GTE
%left PLUS MINUS
%left MULT DIV MOD
%right NOT
%right UMINUS

/* Resolver el dangling else */
%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

%type <node> program stmt_list stmt block
%type <node> expr logical_or logical_and comparison additive multiplicative unary primary
%type <intValue> type

%%

program:
    stmt_list { root = $1; }
    | /* vacío */ { root = NULL; }
    ;

stmt_list:
    stmt { $$ = $1; }
    | stmt_list stmt { $$ = newSeq($1, $2); }
    ;

type:
    TYPE_INT { $$ = TYPE_INT_T; }
    | TYPE_FLOAT { $$ = TYPE_FLOAT_T; }
    | TYPE_BOOL { $$ = TYPE_BOOL_T; }
    | TYPE_STRING { $$ = TYPE_STRING_T; }
    ;

block:
    LBRACE stmt_list RBRACE { $$ = newBlock($2); }
    | LBRACE RBRACE { $$ = NULL; }
    ;

stmt:
    /* Declaración con asignación */
    type ID ASSIGN expr SEMI 
    { 
        $$ = newAssign($2, $4);
        $$->varType = $1;
    }
    
    /* Asignación simple a variable */
    | ID ASSIGN expr SEMI 
    { $$ = newAssign($1, $3); }
    
    /* Declaración de arreglo */
    | type ID LBRACKET NUMBER RBRACKET SEMI 
    { $$ = newArrayDecl($1, $2, newInt($4)); }
    
    /* Asignación a elemento de arreglo */
    | ID LBRACKET expr RBRACKET ASSIGN expr SEMI 
    { 
        $$ = newAssign($1, $6);
        $$->index = $3;
    }
    
    /* PIXEL */
    | KW_PIXEL expr expr expr SEMI 
    { $$ = newPixel($2, $3, $4); }
    
    /* KEY */
    | KW_KEY NUMBER ID SEMI 
    { $$ = newKey($2, $3); }
    
    /* INPUT */
    | KW_INPUT ID SEMI 
    { $$ = newInput($2); }
    
    /* PRINT */
    | KW_PRINT expr SEMI 
    { $$ = newPrint($2); }
    
    | KW_PRINT STRING_LIT SEMI 
    { $$ = newPrint(newString($2)); }
    
    /* IF sin ELSE */
    | KW_IF LPAREN expr RPAREN block %prec LOWER_THAN_ELSE
    { $$ = newIf($3, $5); }
    
    /* IF-ELSE */
    | KW_IF LPAREN expr RPAREN block KW_ELSE block 
    { $$ = newIfElse($3, $5, $7); }
    
    /* WHILE */
    | KW_WHILE LPAREN expr RPAREN block 
    { $$ = newWhile($3, $5); }
    
    /* FOR */
    | KW_FOR LPAREN stmt expr SEMI expr RPAREN block 
    { $$ = newFor($3, $4, $6, $8); }
    
    /* RETURN */
    | KW_RETURN expr SEMI 
    { $$ = newReturn($2); }
    
    | KW_RETURN SEMI 
    { $$ = newReturn(NULL); }
    ;

/* Jerarquía de expresiones sin ambigüedad */
expr:
    logical_or { $$ = $1; }
    ;

logical_or:
    logical_or OR logical_and { $$ = newBinOp(NODE_OR, $1, $3); }
    | logical_and { $$ = $1; }
    ;

logical_and:
    logical_and AND comparison { $$ = newBinOp(NODE_AND, $1, $3); }
    | comparison { $$ = $1; }
    ;

comparison:
    additive EQ additive { $$ = newBinOp(NODE_EQ, $1, $3); }
    | additive NEQ additive { $$ = newBinOp(NODE_NEQ, $1, $3); }
    | additive LT additive { $$ = newBinOp(NODE_LT, $1, $3); }
    | additive GT additive { $$ = newBinOp(NODE_GT, $1, $3); }
    | additive LTE additive { $$ = newBinOp(NODE_LTE, $1, $3); }
    | additive GTE additive { $$ = newBinOp(NODE_GTE, $1, $3); }
    | additive { $$ = $1; }
    ;

additive:
    additive PLUS multiplicative { $$ = newBinOp(NODE_ADD, $1, $3); }
    | additive MINUS multiplicative { $$ = newBinOp(NODE_SUB, $1, $3); }
    | multiplicative { $$ = $1; }
    ;

multiplicative:
    multiplicative MULT unary { $$ = newBinOp(NODE_MUL, $1, $3); }
    | multiplicative DIV unary { $$ = newBinOp(NODE_DIV, $1, $3); }
    | multiplicative MOD unary { $$ = newBinOp(NODE_MOD, $1, $3); }
    | unary { $$ = $1; }
    ;

unary:
    NOT unary { $$ = newUnaryOp(NODE_NOT, $2); }
    | MINUS unary %prec UMINUS { $$ = newBinOp(NODE_SUB, newInt(0), $2); }
    | primary { $$ = $1; }
    ;

primary:
    NUMBER { $$ = newInt($1); }
    | FLOAT_NUM { $$ = newFloat($1); }
    | BOOL_VAL { $$ = newBool($1); }
    | ID { $$ = newId($1); }
    | ID LBRACKET expr RBRACKET { $$ = newArrayAccess($1, $3); }
    | ID DOT ID { 
        if (strcmp($3, "length") == 0) {
            $$ = newArrayLength($1);
            free($3);
        } else {
            yyerror("Propiedad no reconocida");
            $$ = newInt(0);
        }
    }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis en línea %d: %s\n", yylineno, s);
}