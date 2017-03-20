CC=gcc -Wall -g -O2

iploc.o: iploc.c
	$(CC) -c iploc.c

dump: iploc.o dump.c
	$(CC) dump.c iploc.o -o db-dump

test-proc: iploc.o test.c
	$(CC) test.c iploc.o -o test-proc

test: test-proc
	./test-proc

leak-check: test-proc
	valgrind --leak-check=full --log-file=vg.out ./test-proc && \
		echo "Check vg.out for memory result."

clean:
	rm -f *.o test-proc db-dump vg.out

.PHONY: clean
