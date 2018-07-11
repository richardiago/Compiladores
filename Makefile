CC = gcc
LEX = flex

COMPILER_FLAGS = -Wall -Wextra -lfl

PROG = ./Programas/programa_1_heap.br

all: inf_l lex inf_c comp inf_r run

inf_l:
	@echo "--- Executando flex ---"

inf_c:
	@echo "--- Executando gcc ---"

inf_r:
	@echo "--- Executando analisador ---"

lex: ./Analisador\ lexico/alexico.l
	@$(LEX) ./Analisador\ lexico/alexico.l

comp:
	@$(CC) -o analisador lex.yy.c $(COMPILER_FLAGS)

run:
	@./analisador $(PROG)

clear:
	rm lex.yy.c analisador saida.txt