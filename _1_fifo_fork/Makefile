all: srv.c clt.c
	gcc srv.c -o srv
	gcc clt.c -o clt

clean:
	rm *.fifo || echo ""

run: srv
	make clean
	make
	./srv
