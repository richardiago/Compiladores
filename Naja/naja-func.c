/*
 * Rotinas para o sugar
 */
#include "naja.h"

#ifdef DEBUG
#define debug(str)	printf(" ::: %s ::: \n", str);
#else
#define debug(str) ;
#endif
/* Macros */
short _M_VERBOSE = 0;
void callmacro(struct symbol *s){
	if(strcmp(s->name, "verbose") == 0){
		_M_VERBOSE = !_M_VERBOSE;
	}else{
		yyerror("Erro Semântico: Macro Desconhecida. (%s)", s->name);
	}
}

/* Tabela de símbolos */
// Função de hash para a tabela
static unsigned
symhash(char *sym){
	debug("symhash");
	assert(sym != NULL);
	unsigned int hash = 0;
	unsigned c;
	while(c = *sym++) hash = hash*9 ^ c;

	return hash;
}

struct symbol *
lookup(char* sym){
	debug("lookup");
	assert(sym != NULL);
	struct symbol *sp = &symtab[symhash(sym)%NHASH];
	int scount = NHASH; // Quantas vezes foi tomado
	assert(sp != NULL);

	while(--scount >= 0){
		if(sp->name && !strcmp(sp->name, sym)) { return sp; }
		if(!sp->name){/* É uma nova entrada */
			sp->name = strdup(sym);
			sp->initialized = 0;
			sp->func = NULL;
			sp->syms = NULL;
			return sp;
		}
		// Em ultimo caso, tenta a proxima entrada
		if(++sp >= symtab+NHASH) sp = symtab;
	}
	// TODO: Trocar tabela por lista encadeada
	// Devido ao tamanho limitado da tabela
	yyerror("Limite de tamanho da tabela de símbolos atingido.\n");
	abort(); /* Tentou todos, tabela está cheia */
}

/* Nós da AST */
struct ast *
newast(int nodetype, struct ast *l, struct ast *r){
	debug("newast");
	assert(l != NULL); // r pode ser nulo para operadores unários.
	struct ast *a = malloc(sizeof(struct ast));

	if(!a){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = nodetype;
	a->l = l;
	a->r = r;
	return a;
}
/* Valores na AST */
struct ast *
newnumber(double val){
	debug("newnumber");
	data_value dv = {.d = val};
	return newvalue(number, dv);
}

struct ast *
newstring(str_def){
	debug("newstring");
	data_value dv = {.s = s};
	// Remove aspas
	dv.s++;
	dv.s[strlen(dv.s)-1] = '\0';
	return newvalue(string, dv);
}

struct ast *
newboolean(short b){
	debug("newboolean");
	data_value dv = {.b = b};
	return newvalue(boolean, dv);
}

struct ast *
newvalue(data_type dt, data_value dv){
	debug("newvalue");
	struct cteval *a = malloc(sizeof(struct cteval));
	if(!a){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = 'K';
	a->value.dt = dt;
	a->value.dv = dv;
	return (struct ast *)a;
}
/* Comparações na AST */
struct ast *
newcmp(int cmptype, struct ast *l, struct ast *r){
	debug("newcmp");
	assert(l != NULL);
	assert(r != NULL);
	struct ast *a = malloc(sizeof(struct ast));
	if(!a){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = '0' + cmptype;
	a->l = l;
	a->r = r;
	return a;
}
/* Funções na AST */
struct ast *
newfunc(int functype, struct ast *l){
	debug("newfunc");
	assert(l != NULL);
	struct fncall *a = malloc(sizeof(struct fncall));
	if(!a){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = 'F';
	a->l = l;
	a->functype = functype;
	return (struct ast *)a;
}
/* Chamada na AST */
struct ast *
newcall(struct symbol *s, struct ast *l){
	debug("newcall");
	assert(s != NULL);
	assert(l != NULL);
	struct ufncall *a = malloc(sizeof(struct ufncall));
	if(!a) {
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = 'C';
	a->l = l;
	a->s = s;
	return (struct ast *)a;
}
/* Referencia na AST */
struct ast *
newref(struct symbol *s){
	debug("newref");
	assert(s != NULL);
	struct symref *a = malloc(sizeof(struct symref));
	if(!a) {
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = 'N';
	a->s = s;
	return (struct ast *)a;
}
/* Atribuição na AST */
struct ast *
newasgn(struct symbol *s, struct ast *v){
	debug("newasgn");
	assert(s != NULL);
	assert(v != NULL);
	struct symasgn *a = malloc(sizeof(struct symasgn));
	if(!a){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = '=';
	a->s = s;
	a->v = v;
	return (struct ast *)a;
}
/* Estrutura de controle na AST */
struct ast *
newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el){
	debug("newflow");
	assert(cond != NULL);
	assert(tl != NULL); // el pode ser NULL em if sem else

	struct flow *a = malloc(sizeof(struct flow));
	if(!a){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	a->nodetype = nodetype;
	a->cond = cond;
	a->tl = tl;
	a->el = el;
	return (struct ast *)a;
}

/* Libera a memoria a partir de um nó */
void
treefree(struct ast *a){
	debug("treefree");
	assert(a != NULL);
	switch(a->nodetype) {
		/* Duas Sub-Arvores */
		case '+':
		case '-':
		case '*':
		case '/':
		case '1': case '2': case '3': 
		case '4': case '5': case '6': 
		case 'L': case 'P':
			treefree(a->r);
		/* Uma Sub-Arvore */
		case '|':
		case 'M': case 'C': case 'F':
			treefree(a->l);
		/* Sem Sub-Arvore */
		case 'K': case 'N':
			break;
		case '=':
			free( ((struct symasgn *)a)->v);
			break;
		/* Mais de Três Sub-Arvores */
		case 'I': case 'W':
			free( ((struct flow *)a)->cond );
			if( ((struct flow *)a)->tl ) treefree( ((struct flow*)a)->tl);
			if( ((struct flow *)a)->el ) treefree( ((struct flow*)a)->el);
			break;
		default:
			printf("Erro Interno: Erro na Operação Free %c\n", a->nodetype);
	}
	free(a);// Sempre limpa o proprio nó
}
/* Lista de Simbolos */
struct symlist *
newsymlist(struct symbol *sym, struct symlist *next){
	debug("newsymlist");
	assert(sym != NULL); // next pode ser NULL no fim da lista
	struct symlist *sl = malloc(sizeof(struct symlist));
	if(!sl){
		yyerror("Sem espaço de memória.");
		exit(0);
	}
	sl->sym = sym;
	sl->next = next;
	return sl;
}
/* Libera lista de simbolos */
void
symlistfree(struct symlist *sl){
	debug("symlistfree");
	assert(sl != NULL);
	struct symlist *nsl;

	while(sl){
		nsl = sl->next;
		free(sl);
		sl = nsl;
	}
}

/* Validação a partir de um nó AST */
static var* callbuiltin(struct fncall *);
static var* calluser(struct ufncall *);
static short to_boolean(var *v);

void init_validation(struct ast *n){
	if(_M_VERBOSE){
		printval("= %v\n> ", eval(n));
	}else{
		eval(n);
	}
}

short RETURNED = 0;

var *
eval(struct ast *a){
	debug("eval");
	var *v = (var *)malloc(sizeof(var));//TODO: Não alocar toda hora!
	if(!v){
		yyerror("Sem espaço de memória.");
		exit(0);
	}

	assert(a != NULL);

	// TODO: Assert nos cases
	switch(a->nodetype){
		  /* Constante */
		case 'K': v = &((struct cteval *)a)->value; break;
		  /* Referência de Nome */
		case 'N':
		 v = &((struct symref *)a)->s->value;
		 if(((struct symref *)a)->s->initialized == 0){
		 	yyerror("Erro Semântico: Variável (%s) não inicializada.", ((struct symref *)a)->s->name);
		 }
		 break;
		  /* Atribuição */
		case '=': 
			((struct symasgn *)a)->s->value = *eval(((struct symasgn *)a)->v); 
			v = &((struct symasgn *)a)->s->value;
			((struct symasgn *)a)->s->initialized = 1;
			break;
		  /* Expressões */
		case '+': v_add(v, eval(a->l), eval(a->r)); break;
		case '-': v_sub(v, eval(a->l), eval(a->r)); break;
		case '*': v_mul(v, eval(a->l), eval(a->r)); break;
		case '/': v_div(v, eval(a->l), eval(a->r)); break;
		case '%': v_mod(v, eval(a->l), eval(a->r)); break;
		case 'P': v_pow(v, eval(a->l), eval(a->r)); break;
		case 'M': v_neg(v, eval(a->l)); break;
		case '|': v = eval(a->l); v_abs(v); break;
		  /* Comparações */
		case '1': v_gt (v, eval(a->l), eval(a->r)); break;
		case '2': v_lt (v, eval(a->l), eval(a->r)); break;
		case '3': v_dif(v, eval(a->l), eval(a->r)); break;
		case '4': v_eql(v, eval(a->l), eval(a->r)); break;
		case '5': v_get(v, eval(a->l), eval(a->r)); break;
		case '6': v_let(v, eval(a->l), eval(a->r)); break;
		  /* Lógicos */
		case 'A': v_and(v, eval(a->l), eval(a->r)); break;
		case 'O': v_or (v, eval(a->l), eval(a->r)); break;
		  /* Estruturas de Controle */
		// if/then/else
		case 'I':
			if( to_boolean(eval( ((struct flow*)a)->cond )) ) {
				// True
				if( ((struct flow*)a)->tl ){
					v = eval( ((struct flow*)a)->tl );
				}else{
					//printf("TRUE\n");
					v_bool(v, 0); // Valor padrão
				}
			}else{
				// False
				if( ((struct flow*)a)->el ){
					v = eval( ((struct flow*)a)->el );
				}else{
					//printf("TRUE\n");
					v_bool(v, 0); // Valor padrão
				}
			}
			break;
		// while/do
		case 'W':
			v_bool(v, 0); // Valor padrão
			if( ((struct flow*)a)->tl ){
				// Valida a condição
				while( to_boolean( eval( ((struct flow*)a)->cond ) ) ){
					v = eval( ((struct flow*)a)->tl );
				}
			}
			break;
		  /* Lista de STMT */
		case 'L': 
			v = eval(a->l);
			if(!RETURNED){
				//printf("→ %4.4g\n> ", v);
				v = eval(a->r); 
			}else{
				RETURNED = 0;
			}
			break;
		case 'F': v = callbuiltin((struct fncall *)a); break;
		case 'C': v = calluser((struct ufncall *)a); break;
		case 'R': 
			RETURNED = 1;
			v = eval(a->l);
			break;

		default: printf("Erro interno: Nó desconhecido %c\n", a->nodetype);
	}
	assert(v != NULL);
	return v;
}

/* Validação de Funções Nativas */
static var*
callbuiltin(struct fncall *f){
	debug("callbuiltin");
	assert(f != NULL);
	enum bifs functype = f->functype;
	var *v = eval(f->l);
	assert(v != NULL);

	switch(functype){
		case B_sqrt:
			v_num(v, sqrt(v->dv.d));
			break;
		case B_exp:
			v_num(v, exp(v->dv.d));
			break;
		case B_log:
			v_num(v, log(v->dv.d));
			break;
		case B_pow:
			v_num(v, pow(eval(f->l->l)->dv.d, v->dv.d));
			break;
		case B_print:
			printval("%v\n", v);
			break;
		default:
			yyerror("Erro Semântico: Função Desconhecida %d", functype);
			v_bool(v, 0);// Valor Padrão
	}
	return v;
}

/* Definição de Funções do Usuário */
void
dodef(struct symbol *name, struct symlist *syms, struct ast *func){
	debug("dodef");
	assert(name != NULL);
	assert(syms != NULL);
	assert(func != NULL);
	if(name->syms) symlistfree(name->syms);
	if(name->func) treefree(name->func);
	name->syms = syms;
	name->func = func;
}

/* Validação de Funções do Usuário */
static var*
calluser(struct ufncall *f){
	debug("calluser");
	assert(f != NULL);
	struct symbol *fn = f->s; // Nome da função
	struct symlist *sl;	// Argumentos da declaração (dummy)
	struct ast *args = f->l; // Argumentos da chamada
	var *oldval, *newval; // Valores salvos dos args
	var *v;
	int nargs;
	int i;

	if(!fn->func){
		yyerror("Erro Semântico: Chamada a função indefinida (%s).", fn->name);
		v_bool(v, 0);// Valor Padrão
		return v;
	}

	/* Conta os argumentos */
	sl = fn->syms;
	for(nargs = 0; sl; sl = sl->next)
		nargs++;
	/* Prepara para salvar os valores */
	oldval = (var *)malloc(nargs * sizeof(var));
	newval = (var *)malloc(nargs * sizeof(var));
	if(!oldval || !newval ){
		yyerror("Sem espaço em %s", fn->name); 
		v_bool(v, 0);// Valor Padrão
		return v;
	}
	/* Valida os argumentos */
	for(i=0;i<nargs;i++){
		if(!args){
			yyerror("Erro Semântico: Poucos argumentos na chamda (%s).", fn->name);
			free(oldval);free(newval);
			v_bool(v, 0);// Valor Padrão
			return v;
		}

		if(args->nodetype == 'L'){// Se é nó lista
			v = eval(args->l);
			assert(v != NULL);
			newval[i] = *v;
			args = args->r;
		}else{// Fim de uma lista
			v = eval(args);
			assert(v != NULL);
			newval[i] = *v;
			args = NULL;
		}
	}
	/* Salva os valores antigos dos dummies, e atribui os novos */
	sl = fn->syms;
	for(i =0;i<nargs; i++){
		struct symbol *s = sl->sym;
		
		oldval[i] = s->value;
		s->value = newval[i];
		s->initialized = 1;
		sl = sl->next;
	}

	free(newval);

	/* Valida a função */
	v = eval(fn->func);
	assert(v != NULL);

	/* Devolve os valores dos dummies */
	sl = fn->syms;
	for(i=0; i<nargs; i++){
		struct symbol *s = sl->sym;

		s->value = oldval[i];
		s->initialized = 1;
		sl = sl->next;
	}

	free(oldval);
	return v;
}

char *str_replace(char *orig, char *rep, char *with) {
	debug("str_replace");
	assert(orig != NULL);
	assert(rep != NULL);
	assert(with != NULL);
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result){
    	yyerror("Sem espaço de memória.");
		exit(0);
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

static void print_type(var *v){
	assert(v != NULL);
	switch(v->dt){
		case number:
			puts("NUMBER");
			break;
		case string:
			puts("STRING");
			break;
		case boolean:
			puts("BOOLEAN");
			break;
		default:
			puts("UNKNOWN");
			break;
	}
}

void printval(char *msg, var *v){
	debug("printval");
	char *format;
	switch(v->dt){
		case number:
			format = str_replace(msg, "%v", "%.4g");
			printf(format, v->dv.d);
			break;
		case string:
			format = str_replace(msg, "%v", "%s");
			printf(format, v->dv.s);
			break;
		case boolean:
			if(v->dv.b != 0){
				format = str_replace(msg, "%v", "VERDADEIRO");
			}else{
				format = str_replace(msg, "%v", "FALSO");
			}
			printf("%s", format);
			break;
		default:
			yyerror("Erro Interno: Função Print indefinida para tipo.");
	}
	free(format);
}

static short to_boolean(var *v){
	assert(v != NULL);
	debug("to_boolean");
	switch(v->dt){
		case number:
			return v->dv.d != 0;
		case string:
			return 1;
		case boolean:
			return v->dv.b != 0;
	}
}

/* Operações */
var temp = {}; // Variável temporária usada nas funções
void v_num(var *v, double d){
	debug("v_num");
	assert(v != NULL);
	v->dt = number;
	v->dv.d = d;
}

void v_bool(var *v, short b){
	debug("v_bool");
	assert(v != NULL);
	v->dt = boolean;
	v->dv.b = b;
}

void v_add(var *v, var *a, var *b){
	debug("v_add");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = number;
		v->dv.d = a->dv.d + b->dv.d;
	}else if(a->dt == boolean && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = a->dv.b || b->dv.b;
	}else if(a->dt == boolean && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.b || (b->dv.d != 0);
	}else if(a->dt == number && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = (a->dv.d != 0) || b->dv.b;
	}else if(a->dt == number && b->dt == string){
		char buf[64];
		snprintf(buf, 64, "%f", a->dv.d);
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*( strlen(b->dv.s)+strlen(buf) ));
		snprintf(v->dv.s, ( strlen(b->dv.s)+strlen(buf) ), "%s%s", buf, b->dv.s);
	}else if(a->dt == string && b->dt == number){
		char buf[64];
		snprintf(buf, 64, "%f", b->dv.d);
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*( strlen(a->dv.s)+strlen(buf) ));
		snprintf(v->dv.s, ( strlen(a->dv.s)+strlen(buf) ), "%s%s", a->dv.s, buf);
	}else if(a->dt == string && b->dt == boolean){
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*(11+strlen(a->dv.s)) );
		snprintf(v->dv.s, (11+strlen(a->dv.s)), "%s%s", a->dv.s, (b->dv.b)?"VERDADEIROSSS":"FALSO" );
	}else if(a->dt == boolean && b->dt == string){
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*(11+strlen(b->dv.s)) );
		snprintf(v->dv.s, (11+strlen(b->dv.s)), "%s%s", (a->dv.b)?"VERDADEIROSSS":"FALSO", b->dv.s);
	}else if(a->dt == string && b->dt == string){
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*(strlen(a->dv.s)+strlen(b->dv.s)) );
		snprintf(v->dv.s, (strlen(a->dv.s)+strlen(b->dv.s)), "%s%s", a->dv.s, b->dv.s);
	}else{
		yyerror("Erro Interno: Soma de tipos não tratados.");
	}
}

void v_sub(var *v, var *a, var *b){
	debug("v_sub");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = number;
		v->dv.d = a->dv.d - b->dv.d;
	}else if(a->dt == string && b->dt == number){
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*strlen(a->dv.s));
		strcpy(v->dv.s, a->dv.s);
		v->dv.s[strlen(a->dv.s)-(int)b->dv.d] = '\0';
	}else if(a->dt == string && b->dt == string){
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*strlen(a->dv.s));
		strcpy(v->dv.s, a->dv.s);
		if(strlen(b->dv.s) > strlen(a->dv.s)){
			yyerror("Erro Semântico: Retirando palavra de maior comprimento.");
		}else{
			for(int i=1; i<=strlen(b->dv.s); ++i){
				if(a->dv.s[strlen(a->dv.s)-i] == b->dv.s[strlen(b->dv.s)-i]){
					v->dv.s[strlen(a->dv.s)-i] = '\0';
				}else{
					break;
				}
			}	
		}
	}else{
		yyerror("Erro Interno: Subtração de tipos não tratados.");
	}
}

void v_mul(var *v, var *a, var *b){
	debug("v_mul");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = number;
		v->dv.d = a->dv.d * b->dv.d;
	}else if(a->dt == number && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = (a->dv.d != 0) && b->dv.b;
	}else if(a->dt == boolean && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.b && (b->dv.d != 0);
	}else if(a->dt == boolean && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = a->dv.b && b->dv.b;
	}else if(a->dt == string && b->dt == number){
		int i=b->dv.d;
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(i*sizeof(char)*strlen(a->dv.s));
		v->dv.s[0] = '\0';
		while(i--){
			strcat(v->dv.s, a->dv.s);
		}
	}else if(a->dt == number && b->dt == string){
		int i=a->dv.d;
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(i*sizeof(char)*strlen(b->dv.s));
		v->dv.s[0] = '\0';
		while(i--){
			strcat(v->dv.s, b->dv.s);
		}
	}else{
		yyerror("Erro Interno: Multiplicação de tipos não tratados.");
	}
}

void v_div(var *v, var *a, var *b){
	debug("v_div");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		if(b->dv.d == 0){
			yyerror("Erro Semântico: Divisão por zero");
		}else{
			v->dt = number;
			v->dv.d = a->dv.d / b->dv.d;
		}
	}else if(a->dt == string && b->dt == number){
		if(b->dv.d == 0){
			yyerror("Erro Semântico: Divisão por zero");
		}else{
			v->dt = string;
			if(v->dv.s != NULL){free(v->dv.s);}
			v->dv.s = (char*)malloc(sizeof(char)*strlen(a->dv.s));
			strcpy(v->dv.s, a->dv.s);
			v->dv.s[(int)(strlen(a->dv.s)/b->dv.d)] = '\0';
		}
	}else{
		yyerror("Erro Interno: Divisão de tipos não tratados.");
	}
}

void v_mod(var *v, var *a, var *b){
	debug("v_mod");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		if(b->dv.d == 0){
			yyerror("Erro Semântico: Divisão módulo de zero");
		}else{
			v->dt = number;
			v->dv.d = (int)a->dv.d % (int)b->dv.d;
		}
	}else{
		yyerror("Erro Interno: Módulo entre tipos não tratados.");
	}
}

void v_pow(var *v, var *a, var *b){
	debug("v_pow");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = number;
		v->dv.d = pow(a->dv.d, b->dv.d);
	}else{
		yyerror("Erro Interno: Potência entre tipos não tratados.");
	}
}

void v_neg(var *v, var *a){
	debug("v_neg");
	assert(v != NULL);
	assert(a != NULL);
	if(a->dt == number){
		v->dt = number;
		v->dv.d = -a->dv.d;
	}else if(a->dt == boolean){
		v->dt = boolean;
		v->dv.b = !a->dv.b;
	}else if(a->dt == string){
		v->dt = string;
		if(v->dv.s != NULL){free(v->dv.s);}
		v->dv.s = (char*)malloc(sizeof(char)*strlen(a->dv.s));
		for(int i=0; i<strlen(a->dv.s); ++i){
			v->dv.s[i] = a->dv.s[strlen(a->dv.s)-i-1];
		}
		v->dv.s[strlen(a->dv.s)] = '\0';
	}else{
		yyerror("Erro Interno: Negação de tipo não tratado.");
	}
}

void v_abs(var *v){
	debug("v_abs");
	assert(v != NULL);
	if(v->dt == number){// Toma valor positivo
		if(v->dv.d < 0) v->dv.d *= -1;
	}else if(v->dt == boolean){// Mantém 0 ou 1
		if(v->dv.b != 0) v->dv.b = 1;
	}else{
		yyerror("Erro Interno: Absoluto de tipo não tratado.");
	}
}

void v_gt (var *v, var *a, var *b){
	debug("v_gt");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.d > b->dv.d;
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (>)");
	}
}

void v_lt (var *v, var *a, var *b){
	debug("v_lt");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.d < b->dv.d;
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (<)");
	}
}

void v_dif(var *v, var *a, var *b){
	debug("v_dif");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.d != b->dv.d;
	}else if(a->dt == boolean && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = a->dv.b != b->dv.b;
	}else if(a->dt == string && b->dt == string){
		v->dt = boolean;
		v->dv.b = (strcmp(a->dv.s,b->dv.s) != 0);
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (!=)");
	}
}

void v_eql(var *v, var *a, var *b){
	debug("v_eql");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.d == b->dv.d;
	}else if(a->dt == boolean && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = a->dv.b == b->dv.b;
	}else if(a->dt == string && b->dt == string){
		v->dt = boolean;
		v->dv.b = (strcmp(a->dv.s,b->dv.s) == 0);
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (==)");
	}
}

void v_get(var *v, var *a, var *b){
	debug("v_get");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.d >= b->dv.d;
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (>=)");
	}
}

void v_let(var *v, var *a, var *b){
	debug("v_let");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == number && b->dt == number){
		v->dt = boolean;
		v->dv.b = a->dv.d <= b->dv.d;
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (<=)");
	}
}

void v_and(var *v, var *a, var *b){
	debug("v_and");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == boolean && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = a->dv.b && b->dv.b;
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (&&)");
	}
}

void v_or (var *v, var *a, var *b){
	debug("v_or");
	assert(v != NULL);
	assert(a != NULL);
	assert(b != NULL);
	if(a->dt == boolean && b->dt == boolean){
		v->dt = boolean;
		v->dv.b = a->dv.b || b->dv.b;
	}else{
		yyerror("Erro Interno: Comparação entre tipos não tratados. (||)");
	}
}
