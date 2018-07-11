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

digito  [0-9]
letra   [a-zA-Z]
id      {letra}({letra}|{digito})*
inteiro {digito}+


%%
[ \t\n]     ;    {/*espaços em branco*/}
\#.*\n      ;    {/*comentarios*/}
{id}        {if (palavra_reservada(yytext))
                return (S_PR);
            else
                return (S_ID);
            }

{inteiro}   printf("%s - %s\n", yytext, "NUM_INT");

"."         return (S_PF);
","         return (S_V);
":"         return (S_DP);
"["         return (S_ACO);
"]"         return (S_FCO);
"("         return (S_AP);
")"         return (S_FP);
"{"         return (S_ACH);
"}"         return (S_FCH);
"<"         return (S_MENOR);
">"         return (S_MAIOR);
"<="        return (S_MENOR_IGUAL);
">="        return (S_MAIOR_IGUAL);
"="         return (S_IGUAL);
"=="        return (S_COMP);
"!="        return (S_DIF);
"+"         return (S_SOMA);
"-"         return (S_SUB);
"*"         return (S_MULT);
"/"         return (S_DIV);
";"         return (S_PV);


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
			break;

		case S_V:
			return ",";
			break;

		case S_DP:
			return ":";
			break;

    case S_ACO:
      return "[";
      break;

    case S_FCO:
      return "]";
      break;

    case S_AP:
      return "(";
      break;

    case S_FP:
      return ")";
      break;

    case S_ACH:
      return "{";
      break;

    case S_MENOR:
      return "<";
      break;

    case S_MAIOR:
      return ">";
      break;

    case S_MENOR_IGUAL:
      return "<=";
      break;

    case S_MAIOR_IGUAL:
      return ">=";
      break;

    case S_IGUAL:
      return "=";
      break;

    case S_COMP:
      return "==";
      break;

    case S_DIF:
      return "!=";
      break;

    case S_SOMA:
      return "+";
      break;

    case S_SUB:
      return "-";
      break;

    case S_MULT:
      return "*";
      break;

    case S_DIV:
      return "/";
      break;

    case S_ID:
      return "ID";
      break;

		default:
			return "INVALIDO";

	}
}

int main()
{
  FILE *fp1, *fp2;
	char nomeArquivo[50];
	scanf("%s", nomeArquivo);
	fp1 = fopen(nomeArquivo, "r");
  fp2 = fopen("saida.txt", "w");

  if(fp1 == NULL || fp2 == NULL )
  {
     printf("Erro na abertura/criação do arquivo!");
     return 1;
  }

	yyin = fp1;
  int codigo;

	while (codigo = yylex()){
		printf("%s - %s\n", yytext, getString(codigo));
    fprintf(fp2, "%s - %s\n", yytext, getString(codigo));
	}

  fclose(fp1);
  fclose(fp2);

  return 0;
}
