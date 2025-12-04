/* lexer.l - Analizador Lexico para Compilador FIS-25 */
%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap
%option yylineno

%%

"int"           { return TYPE_INT; }
"float"         { return TYPE_FLOAT; }
"bool"          { return TYPE_BOOL; }
"string"        { return TYPE_STRING; }

"true"          { yylval.intValue = 1; return BOOL_VAL; }
"false"         { yylval.intValue = 0; return BOOL_VAL; }

"if"            { return KW_IF; }
"else"          { return KW_ELSE; }
"while"         { return KW_WHILE; }
"for"           { return KW_FOR; }

"function"      { return KW_FUNCTION; }
"return"        { return KW_RETURN; }

"PIXEL"         { return KW_PIXEL; }
"KEY"           { return KW_KEY; }
"INPUT"         { return KW_INPUT; }
"PRINT"         { return KW_PRINT; }

"="             { return ASSIGN; }
"=="            { return EQ; }
"!="            { return NEQ; }
"<="            { return LTE; }
">="            { return GTE; }
"<"             { return LT; }
">"             { return GT; }

"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }
"%"             { return MOD; }

";"             { return SEMI; }
","             { return COMMA; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
"."             { return DOT; }

\"([^\\\"]|\\.)*\" { 
    yylval.idName = strndup(yytext+1, strlen(yytext)-2); 
    return STRING_LIT; 
}

[0-9]+\.[0-9]+  { yylval.floatValue = atof(yytext); return FLOAT_NUM; }

[0-9]+          { yylval.intValue = atoi(yytext); return NUMBER; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.idName = strdup(yytext); return ID; }

"//".*          { }

[ \t\r\n]+      { }

.               { fprintf(stderr, "Error lexico linea %d: '%s'\n", yylineno, yytext); }

%%