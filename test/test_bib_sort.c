#include <stdlib.h>
#include <stdio.h>

#include "PSSA_utility.h"

#include <test_data/bib_blocksort.h>

#ifndef NUM_THREADS
#define NUM_THREADS 1
#endif

#define OVERSHOOT 8


int main(void) {
  STRUCT_TYPE ss;
  unsigned i;

  const unsigned LENGTH;

  INIT(&ss, block, 99981, OVERSHOOT);

  printf("initial_array\n");
  DISPLAY_A(&ss, 10);
  
  unsigned long long timing = cycles();
  SUFFIX_SORT(&ss, NUM_THREADS);
  timing = cycles() - timing;

  printf("timing is %llu / %.3e cycles\n", timing, (double) timing);

  printf("sorted array\n");
  DISPLAY_ISA(&ss, 10, 10);


  if (CHECK_AGAINST(&ss, sorted_index_array)) return 1;


  return 0;
}
