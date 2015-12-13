CFLAGS += -I./include 

LIB_SOURCES= src/prefix_doubling_sort.c
LIB_OBJECTS=$(LIB_SOURCES:.c=.o)

LIB = libsort.a

$(LIB): $(LIB_OBJECTS)
	$(AR) rvs $(LIB) $^

test_sort: test/test_sort.c src/parallel_suffix_sort.c 
	$(CC) $(CFLAGS) -o $@ $^ -pthread

test_prefix_doubling_sort: test/test_prefix_doubling_sort.c $(LIB)
	$(CC) $(CFLAGS) -I./ -Iinclude -o $@ $^ -pthread

test_bib_sort: test/test_bib_sort.c src/parallel_suffix_sort.c src/suffix_sort_utility.c 
	$(CC) $(CFLAGS) -include include/suffix_sort_utility.h -I./ -o $@ $^ -pthread

test_bib_prefix_sort: test/test_bib_prefix_sort.c 
	$(CC) $(CFLAGS) -I./ -o $@ $^ -pthread


clean: 
	rm -rf $(LIB) $(LIB_OBJECTS) test_sort test_bib_sort test_suffix_sort test_bib_prefix_sort
