iploc.o: iploc.c
	gcc -Wall -g -O2 -c iploc.c

test-proc: iploc.o test.c
	gcc -Wall -g -O2 test.c iploc.o -o test-proc

test: test-proc
	./test-proc

leak-check: test-proc
	valgrind --leak-check=full --log-file=vg.out ./test-proc && \
		echo "Check vg.out for memory result."

clean:
	rm -f iploc.o test.o test-proc vg.out

.PHONY: clean test leak-check
