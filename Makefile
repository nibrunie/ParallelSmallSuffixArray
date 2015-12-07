CFLAGS += -O0 -g -I./include 

test_sort: test/test_sort.c src/parallel_suffix_sort.c 
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -rf test_sort
