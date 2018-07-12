%{
  #include <stdio.h>
  #include <string.h>

  //Simbolos
  #define S_PF 1
  #define S_V 2
  #define S_DP 3
  #define S_ACO 4
  #define S_FCO 5
  #define S_AP 6
  #define S_FP 7
  #define S_ACH 8
  #define S_FCH 9
  #define S_MENOR 10
  #define S_MAIOR 11
  #define S_MAIOR_IGUAL 12
  #define S_MENOR_IGUAL 13
  #define S_IGUAL 14
  #define S_COMP 15
  #define S_DIF 16
  #define S_SOMA 17
  #define S_SUB 18
  #define S_MULT 19
  #define S_DIV 20
  #define S_PR 21
  #define S_ID 22
  #define S_PV 23

  //Palavras reservadas
  #define DEFINE 24
  #define IMPRIME 25
  #define CRIA 26
  #define SE 27
  #define SENAO 28
  #define RETORNA 29
  #define TAMANHO 30
  #define ENQUANTO 31
  #define INTERROMPA 32
  #define CONTINUE 33

%}

digito    [0-9]
alfa      [a-zA-Z]
alfa_exp  [_áéíóúàèìòùãẽĩõũäëïöüç]
letra     {alfa}|{alfa_exp}
id        {letra}({letra}|{digito})*
inteiro   {digito}+
real      {digito}*[.]{digito}+
espaco    [ \t]
palavra   \"({letra}|{digito}|{espaco})*\"


%%
{espaco}      ;    {/*espaços em branco*/}
\#.*\n        ;    {/*comentarios*/}

"define"      return (DEFINE);
"imprime"     return (IMPRIME);
"cria"        return (CRIA);
"se"          return (SE);
"senao"       return (SENAO);
"retorna"     return (RETORNA);
"tamanho"     return (TAMANHO);
"enquanto"    return (ENQUANTO);
"interrompa"  return (INTERROMPA);


{id}          return (S_ID);
{real}        printf("%s - %s\n", yytext, "NUM_REAL");
{inteiro}     printf("%s - %s\n", yytext, "NUM_INT");

"."           return (S_PF);
","           return (S_V);
":"           return (S_DP);
"["           return (S_ACO);
"]"           return (S_FCO);
"("           return (S_AP);
")"           return (S_FP);
"{"           return (S_ACH);
"}"           return (S_FCH);
"<"           return (S_MENOR);
">"           return (S_MAIOR);
("<="|"≤")    return (S_MENOR_IGUAL);
(">="|"≥")    return (S_MAIOR_IGUAL);
"="           return (S_IGUAL);
"=="          return (S_COMP);
("!="|"≠")    return (S_DIF);
"+"           return (S_SOMA);
"-"           return (S_SUB);
("*"|"·"|"×") return (S_MULT);
("/"|"÷")     return (S_DIV);
";"           return (S_PV);

""

. fprintf(stderr, "Sem Captura: %s\n", yytext);
%%

const char *getString (int codigo)
{
	switch(codigo) {

		case S_PF:
			return ".";

		case S_V:
			return ",";

		case S_DP:
			return ":";

    case S_ACO:
      return "[";

    case S_FCO:
      return "]";

    case S_AP:
      return "(";

    case S_FP:
      return ")";

    case S_ACH:
      return "{";

    case S_MENOR:
      return "<";

    case S_MAIOR:
      return ">";

    case S_MENOR_IGUAL:
      return "<=";

    case S_MAIOR_IGUAL:
      return ">=";

    case S_IGUAL:
      return "=";

    case S_COMP:
      return "==";

    case S_DIF:
      return "!=";

    case S_SOMA:
      return "+";

    case S_SUB:
      return "-";

    case S_MULT:
      return "*";

    case S_DIV:
      return "/";

    case S_ID:
      return "ID";

    case S_PR:
      return "PR";

    case DEFINE:
      return "define";

    case IMPRIME:
      return "imprime";

    case CRIA:
      return "cria";

    case SE:
      return "se";

    case SENAO:
      return "senao";

    case RETORNA:
      return "retorna";

    case TAMANHO:
      return "tamanho";

    case INTERROMPA:
      return "interrompa";

    case CONTINUE:
      return "continue";

		default:
      return "INVALIDO";

	}
}

int main(int argc, char *argv[])
{

  FILE *fp1, *fp2;
  int codigo;

  if(argc != 2){
    printf("usage: analisador file");
    return 1;
  }

	fp1 = fopen(argv[1], "r");
  fp2 = fopen("saida.txt", "w");

  if(fp1 == NULL || fp2 == NULL ){

     fprintf(stderr, "Erro na abertura/criação do arquivo!");
     return 1;
  }

	yyin = fp1;


	while ((codigo = yylex())){
		printf("%s - %s\n", yytext, getString(codigo));
    fprintf(fp2, "%s - %s\n", yytext, getString(codigo));
	}

  fclose(fp1);
  fclose(fp2);

  return 0;
}
