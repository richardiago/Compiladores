%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "naja.h"

//#define YYERROR_VERBOSE

#define YY_INPUT(buf,result,max_size)  {\
    result = GetNextChar(buf, max_size); \
    if (  result <= 0  ) \
      result = YY_NULL; \
    }

%}
%error-verbose

%union {
	struct ast *a;
	double d;
	char *str;
	struct symbol *s;// Simbolo
	struct symlist *sl;
	int fn;//Função
}

/* Tokens */
%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token <str> STRING
%token <fn> TRUE FALSE

%token IF THEN ELSE WHILE DO LET END RET

%left AND OR
%nonassoc <fn> CMP
%right '=' ADDATR SUBATR MULATR DIVATR MODATR ADDADD SUBSUB
%left '+' '-'
%left '*' '/' '%'
%right '#'
%right EOL
%left <d> POW POW_2 POW_3
%nonassoc '|' UMINUS

%type <a> exp stmt list explist
%type <sl> symlist

%start calclist
%%

stmt	 : stmt IF exp EOL {$$ = newflow('I', $3, $1, NULL); }
		 | IF exp THEN list END { $$ = newflow('I', $2, $4, NULL); }
		 | IF exp EOL list END { $$ = newflow('I', $2, $4, NULL); }
		 | IF exp THEN list ELSE THEN list END { $$ = newflow('I', $2, $4, $7); }
		 | IF exp EOL list ELSE THEN list END { $$ = newflow('I', $2, $4, $7); }
		 | IF exp EOL list ELSE EOL list END { $$ = newflow('I', $2, $4, $7); }
		 | WHILE exp DO list END { $$ = newflow('W', $2, $4, NULL); }
		 | WHILE exp EOL list END { $$ = newflow('W', $2, $4, NULL); }
		 | RET exp {$$ = newast('R', $2, NULL);} 
		 | exp 
		 ;
list 	 : /* Vazio */   { $$ = NULL; }
		 | EOL list {$$ = $2;}
		 | stmt EOL list { 
		 	if($3 == NULL)
		 		$$ = $1;
		 	else
		 		$$ = newast('L', $1, $3);
		 }
		 | stmt
		 ;
exp		 :exp CMP exp {$$ = newcmp($2, $1, $3); }
		 |exp AND exp {$$ = newast('A', $1,$3); }
		 |exp OR exp {$$ = newast('O', $1,$3); }
		 |exp '+' exp {$$ = newast('+', $1,$3); }
		 |exp '-' exp {$$ = newast('-', $1,$3); }
		 |exp '*' exp {$$ = newast('*', $1,$3); }
		 |exp '/' exp {$$ = newast('/', $1,$3); }
		 |exp '%' exp {$$ = newast('%', $1,$3); }
		 |exp POW exp {$$ = newast('P', $1,$3); }
		 |exp POW_2   {	$$ = newast('P', $1,newnumber(2)); }
		 |exp POW_3   {$$ = newast('P', $1,newnumber(3));}
		 |'|' exp 	  {$$ = newast('|', $2, NULL); }
		 |'(' exp ')' {$$ = $2; }
		 |'-' exp %prec UMINUS  {$$ = newast('M', $2, NULL); }
		 |NUMBER 				{$$ = newnumber($1); }
		 |STRING 				{$$ = newstring($1); }
		 |NAME ADDADD			{$$ = newasgn($1, newast('+', newref($1), newnumber(1))); }
		 |NAME SUBSUB			{$$ = newasgn($1, newast('-', newref($1), newnumber(1))); }
		 |NAME ADDATR exp 		{$$ = newasgn($1, newast('+', newref($1), $3)); }
		 |NAME SUBATR exp 		{$$ = newasgn($1, newast('-', newref($1), $3)); }
		 |NAME MULATR exp 		{$$ = newasgn($1, newast('*', newref($1), $3)); }
		 |NAME DIVATR exp 		{$$ = newasgn($1, newast('/', newref($1), $3)); }
		 |NAME MODATR exp 		{$$ = newasgn($1, newast('%', newref($1), $3)); }
		 |NAME '=' exp 			{$$ = newasgn($1, $3); }
		 |NAME 					{$$ = newref($1); }
		 |FUNC '(' explist ')'	{$$ = newfunc($1, $3); }
		 |NAME '(' explist ')'	{$$ = newcall($1, $3); }
		 |TRUE 					{$$ = newboolean(1);}
		 |FALSE  				{$$ = newboolean(0);}
		 ;
explist	 : exp
		 | exp ',' explist  { $$ = newast('L', $1, $3); }
		 ;
symlist  : NAME { $$ = newsymlist($1, NULL); }
		 | NAME ',' symlist { $$ = newsymlist($1, $3); }
		 ;
macro	 : '#' NAME {callmacro($2);}
		 ;
calclist : /* vazio */
		 | calclist macro EOL {}
		 | calclist stmt EOL {
		 	init_validation($2);
		 	treefree($2);
		 }
		 | calclist LET NAME '(' symlist ')' EOL list END {
		 	dodef($3, $5, $8);
		 	//printf("Função Definida: %s\n> ", $3->name);
		 }
		 | calclist EOL {}
		 | calclist error EOL { yyerrok; printf("> "); }/* Linha vazia ou comentário */
		 ;
%%

void
yyerror (char const *msg, ...){
	va_list ap;
	va_start(ap, msg);

	fprintf(stderr, "%d: ", yylineno);
	vfprintf(stderr, msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

int main(){
	return yyparse();
}