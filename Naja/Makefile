NAME = naja
PROG = Programas/teste_1_primo.nj

all: bison flex gcc run

bison:
	@echo "--- BISON --- "
	bison -d $(NAME).y

flex:
	@echo "--- FLEX --- "
	flex -o $(NAME).lex.c $(NAME).l

gcc:
	@echo "--- GCC --- "
	gcc -o exe $(NAME).tab.c $(NAME).lex.c $(NAME)-func.c -lfl -lm

run:
	@echo "--- RUN --- "
	./exe < $(PROG)

clear:
	@echo "--- CLEAR --- "
	rm $(NAME).tab.h $(NAME).tab.c exe $(NAME).lex.c