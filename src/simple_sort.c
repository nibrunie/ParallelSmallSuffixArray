#include <stdio.h>
#include "simple_sort.h"

void simple_sort(const sort_struct_t* ss) {
  // initial radix sort on first byte
  unsigned ftab[257] = {0};

  unsigned i;
  // coutning each byte frequency
  for (i = 0; i < ss->length; ++i) {
    ftab[ss->arr[i]]++;
  };

  // building increasing frequency histogramm
  for (i = 1; i < 257; ++i) {
    ftab[i] += ftab[i-1];
  }

  // building first buckets repartition
  for (i = 0; i < ss->length; ++i) {
    ss->suffix_array[ftab[ss->arr[i]]-1] = i;
    ftab[ss->arr[i]]--;
  }

}

void display_sorted_order(const sort_struct_t *ss) {
  unsigned i;
  for (i = 0; i < ss->length; ++i) {
    unsigned index = ss->suffix_array[i];
    printf("%d[%d] ", ss->arr[index], index);
  }
  printf("\n");
}
