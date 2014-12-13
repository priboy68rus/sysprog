all: srv.c clt.c
	gcc -o srv srv.c
	gcc -o clt clt.c
	gcc -o init init.c

run:
	make
	./srv
