%{
  #include <stdio.h>
  #include <string.h>

  char lpr[13][10] = {"define", "imprime", "cria", "se", "senao", "retorna", "tamanho", "enquanto", "interrompa"};
  enum pr {define, imprime, cria, se, senao, retorna, tamanho, enquanto, interrompa};
  int palavra_reservada (char pal[]);

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

{id}          {if (palavra_reservada(yytext))
                  return (S_PR);
              else
                  return (S_ID);
              }

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

. fprintf(stderr, "Sem Captura: %s\n", yytext);
%%
int palavra_reservada (char pal[])
{

  for (int i= 0; i < 13; i++) {
    if (!strcmp (pal, lpr[i])) {
      return (1);
    }
  }

  return (0);
}

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

		default:
      //printf("INVALIDO: %d\n", codigo);
			return "INVALIDO";

	}
}

int main(int argc, char *argv[])
{
  FILE *fp1, *fp2;
  if(argc != 2){
    printf("usage: analisador file");
    return 1;
  }

	fp1 = fopen(argv[1], "r");
  fp2 = fopen("saida.txt", "w");

  if(fp1 == NULL || fp2 == NULL )
  {
     fprintf(stderr, "Erro na abertura/criação do arquivo!");
     return 1;
  }

	yyin = fp1;
  int codigo;

	while ((codigo = yylex())){
		printf("%s - %s\n", yytext, getString(codigo));
    fprintf(fp2, "%s - %s\n", yytext, getString(codigo));
	}

  fclose(fp1);
  fclose(fp2);

  return 0;
}
