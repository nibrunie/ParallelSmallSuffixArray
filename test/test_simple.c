#include <stdlib.h>
#include <stdio.h>

#include "sort_struct.h"
#include "simple_sort.h"


int order_checker(sort_struct_t* ss, int (*func)(sort_struct_t* , unsigned, unsigned)) {
  unsigned i;
  for (i = 1; i < ss->length; ++i) {
    if (!(func(ss, i-1, i))) return 0;
  };
  return 1;
}

int uc_comp(sort_struct_t* ss, unsigned i0, unsigned i1) {
  return (ss->arr[ss->suffix_array[i0]] < ss->arr[ss->suffix_array[i1]]);
}

int main(void) {
  const int LENGTH = 10;

  // declaring and initializing empty sort structure
  sort_struct_t test_struct;
  test_struct.length = LENGTH;
  test_struct.arr = malloc(sizeof(unsigned char) * LENGTH);
  test_struct.suffix_array = malloc(sizeof(unsigned) * LENGTH);

  // filing and displaying initial state of sort structure
  unsigned i;
  for (i = 0; i < LENGTH; ++i) test_struct.arr[i] = (unsigned char) rand();
  printf("initial (unsorted) array\n");
  for (i = 0; i < LENGTH; ++i) printf("%02u ", test_struct.arr[i]);
  printf("\n");

  // sorting array
  printf("sorting array\n");
  simple_sort(&test_struct);

  printf("displaying sorted array \n");
  display_sorted_order(&test_struct);

  printf("checking order\n");
  if (!order_checker(&test_struct, uc_comp)) {
    printf("    FAILED\n");
  } else {
    printf("    SUCCESS\n");
  }


  return 0;
}
