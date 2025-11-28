/* lexer.l */
%{
#include "ast.h"
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap

%%

"int"       { return TYPE_INT; }
"PIXEL"     { return KW_PIXEL; } /* [cite: 25] */
"KEY"       { return KW_KEY; }   /* [cite: 26] */
"if"        { return KW_IF; }
"="         { return ASSIGN; }
";"         { return SEMI; }
[0-9]+      { yylval.intValue = atoi(yytext); return NUMBER; }
[a-zA-Z][a-zA-Z0-9]* { yylval.idName = strdup(yytext); return ID; }
[ \t\n]     ; /* Ignorar espacios */
.           { printf("Error lexico: %s\n", yytext); }

%%