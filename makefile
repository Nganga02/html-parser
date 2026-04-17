all:
	gcc -I include -c src/main.c
	gcc -I include -c src/lexer.c
	gcc main.o lexer.o -o lexer