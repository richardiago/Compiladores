%option noyywrap nodefault yylineno
%{
	#include "naja.h"
	#include "naja.tab.h"
%}

espaco [ \t]
letra [a-zA-Z_çáàãâäéèêẽëíìîĩïóòôõöúùûũü]
numb [0-9]
EXP ([Ee][-+]?{numb}+)
simb ({letra}|{numb})
id {letra}{simb}*
string \"(\\.|[^"\\])*\"
%%

 /* Mistos */
"+="		{ return ADDATR; }
"-="		{ return SUBATR; }
"*="		{ return MULATR; }
"/="		{ return DIVATR; }
"%="		{ return MODATR; }
"++"		{ return ADDADD; }
"--"		{ return SUBSUB; }
 /* Operadores */
"+"			{ return '+'; }
"-"			{ return '-'; }
"*"			{ return '*'; }
"/"			{ return '/'; }
"%"			{ return '%'; }
"="			{ return '='; }
"|"			{ return '|'; }
","			{ return ','; }
"("			{ return '('; }
")"			{ return ')'; }
"#"			{ return '#'; }
 /* Comparações */
">"			{ yylval.fn = 1; return CMP;}
"<"			{ yylval.fn = 2; return CMP;}
("<>"|"!=")	{ yylval.fn = 3; return CMP;}
"=="		{ yylval.fn = 4; return CMP;}
(">="|"≥")	{ yylval.fn = 5; return CMP;}
("<="|"≤")	{ yylval.fn = 6; return CMP;}
("ou"|"||") { return OR;}
("e"|"&&") { return AND;}

 /* Palavras Chaves (Keywords) */
"se"	{ return IF;}
("então"|"entao")	{ return THEN;}
("senão"|"senao")	{ return ELSE;}
("enquanto")	{ return WHILE;}
("faça"|"faca")	{ return DO;}
("seja"|"defina"|"define")	{ return LET;}
"fim"	{ return END;}
"retorne" { return RET;}
"VERDADEIRO" {return TRUE;}
"FALSO" {return FALSE;}

 /* Funções Nativas */
"thread"	{return THREAD;}
"join"		{return JOIN;}
"raiz"		{ yylval.fn = B_sqrt;  return FUNC; }
"exp"		{ yylval.fn = B_exp;   return FUNC; }
"log"		{ yylval.fn = B_log;   return FUNC; }
"imprima" 	{ yylval.fn = B_print; return FUNC; }
"pot"		{ yylval.fn = B_pow;   return FUNC; }

 /* Valores */
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval.d = atof(yytext); return NUMBER; }
{string}	{ yylval.str = strdup(yytext); return STRING;}

 /* Nomes */
{id} { yylval.s = lookup(yytext); return NAME; }

 /* Especiais */
"^"	{ return POW; }
"²" { yylval.d = 2; return POW_2; }
"³"	{ yylval.d = 3; return POW_3; }
"½"	{ yylval.d = 0.5; return NUMBER; }
"¾"	{ yylval.d = 0.75; return NUMBER; }
"¼"	{ yylval.d = 0.25; return NUMBER; }
"⅜"	{ yylval.d = 0.375; return NUMBER; }
"⅞" { yylval.d = 0.875; return NUMBER; }

 /* Controle */
"//".*
[\n;]		{ return EOL; }
[ \t] 		/* ignore whitespace */
\\\n 		{ printf("c> "); } /* Ignora */
.			{ yyerror("Erro lexico : %s\n",yytext); }

%%
