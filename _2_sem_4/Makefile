all: srv.c clt.c
	gcc -o srv srv.c
	gcc -o clt clt.c
	gcc -o init init.c

run:
	make
	./semrm.sh || echo "" 2> /dev/null
	./init
	./srv

clean:
	./semrm.sh || echo "" 2> /dev/null
