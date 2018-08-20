/*
 * Declarações para sugar
 */ 
#pragma once
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "naja.tab.h"

// TODO: Trabalhar em strings como listas encadeadas, assim
// não será preciso se preocupar com limites, porém, será 
// preciso implementar todas as operações básicas
#define str_def char *s

/* Interface com lexer */
extern int yylineno;
void yyerror (char const *msg, ...);
int yylex();

/* Tipagem Dinâmica */
typedef enum data_type {
	number, string, boolean
}data_type;

typedef union data_value {
	short b;
	double d;
	str_def;
} data_value;

typedef struct variable {
	data_type  dt;
	data_value dv;
} var;

/* symbol table */
struct symbol{/* Variáveis e Funções */
	char *name;
	short initialized;
	var value;
	struct ast *func; /* stmt para funções */
	struct symlist *syms;
};

/* Tabela de Símbolos (tamanho fixo) */
#define NHASH 9997
struct symbol symtab[NHASH];
struct symbol *lookup(char*);

/* Lista de Simbolos, para uma lista de argumentos */
struct symlist {
	struct symbol *sym;
	struct symlist *next;
};

/* Cria um novo simbolo (var, func) */
struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
/* Limpa o simbolo da memoria */
void symlistfree(struct symlist *sl);

/* node types
 * + - * / |
 * 0-7 opp e cop, 04 bit igual, 02 menor, 01 maior
 * M negação
 * L lista (exp ou stmt)
 * I if
 * W while
 * N ref (ponteiro)
 * = atribuição
 * S lista de simbolos
 * F chamada de função nativa
 * C chamada de função do usuário
 */
/* Funções Nativas */
enum bifs {
	B_sqrt = 1,
	B_exp,
	B_log,
	B_print,
	B_pow
};

/* Os nós na AST tem todos um nodetype inicial comúm */
/* Nós da Arvore sintática Abstrata */
struct ast{
	int nodetype;
	struct ast *l;
	struct ast *r;
};
/* Chamada de Função Nativa */
struct fncall{
	int nodetype; // F
	struct ast *l;
	enum bifs functype;
};
/* Chamada de Função do Usuário */
struct ufncall{
	int nodetype;// C
	struct ast *l;/* Lista de Argumentos */
	struct symbol *s;
};
/* Flow - Estruturas de controle*/
struct flow{
	int nodetype;// I ou W
	struct ast *cond; 
	struct ast *tl; // DO
	struct ast *el; // ELSE (opcional)
};
/* Valor constante */
struct cteval{
	int nodetype;// K
	var value;
};
/* Referencia */
struct symref {
	int nodetype; // N
	struct symbol *s;
};
/* Atribuição */
struct symasgn{
	int nodetype;// =
	struct symbol *s;
	struct ast *v; /* Valor */
};

/* Tratamento de Macros */
void callmacro(struct symbol *s);

/* Constroi uma AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

struct ast *newvalue(data_type dt, data_value dv);
struct ast *newnumber(double val);
struct ast *newstring(str_def);
struct ast *newboolean(short b);


/* Define uma função */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* Valida uma AST */
void init_validation(struct ast *n);
var *eval(struct ast *);
void printval(char *msg, var *v);

/* Destroi e limpa uma AST */
void treefree(struct ast *);

/* Operações */
void v_num(var *v, double d);
void v_bool(var *v, short b);

void v_add(var *v, var *a, var *b);
void v_sub(var *v, var *a, var *b);
void v_mul(var *v, var *a, var *b);
void v_div(var *v, var *a, var *b);
void v_mod(var *v, var *a, var *b);
void v_pow(var *v, var *a, var *b);
void v_gt (var *v, var *a, var *b);
void v_lt (var *v, var *a, var *b);
void v_dif(var *v, var *a, var *b);
void v_eql(var *v, var *a, var *b);
void v_get(var *v, var *a, var *b);
void v_let(var *v, var *a, var *b);
void v_and(var *v, var *a, var *b);
void v_or (var *v, var *a, var *b);
void v_neg(var *v, var *a);
void v_abs(var *v);
