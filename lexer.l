%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
%}
%option noyywrap yylineno

%%
"//".* ; /* <--- ESTA LINEA ES LA CLAVE: Ignora todo despues de // */

"int"       { return TYPE_INT; }
"if"        { return KW_IF; }
"else"      { return KW_ELSE; }
"while"     { return KW_WHILE; }
"PIXEL"     { return KW_PIXEL; }
"KEY"       { return KW_KEY; }

"="         { return ASSIGN; }
"=="        { return EQ; }
"<"         { return LT; }
">"         { return GT; }
"&&"        { return AND; }
"||"        { return OR; }
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MULT; }
"/"         { return DIV; }
";"         { return SEMI; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }

[0-9]+      { yylval.intValue = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.idName = strdup(yytext); return ID; }
[ \t\n]+    ;
.           { printf("Error lexico: %s en linea %d\n", yytext, yylineno); }
%%