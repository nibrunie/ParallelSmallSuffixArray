
CFLAGS+=-I./include

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

test_simple: test/test_simple.c src/simple_sort.o
	$(CC) $(CFLAGS) -o test_simple $^

clean:
	rm -f test_simple *.o **.o
  
