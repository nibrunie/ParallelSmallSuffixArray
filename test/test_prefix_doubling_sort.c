#include <stdlib.h>
#include <stdio.h>
#include "PSSA_utility.h"
#include "prefix_doubling_sort.h"
#include <inttypes.h>

#include "prefix_doubling_utility.h"



#ifndef NUM_THREADS
#define NUM_THREADS 1
#endif

#define OVERSHOOT 8

#ifndef TEST_LENGTH
#define TEST_LENGTH 100000
#endif

#ifndef MOD
#define MOD 256
#endif

#ifndef DISPLAY_LENGTH
#define DISPLAY_LENGTH 10
#endif

int main(void) {
  prefix_suffix_t ps;
  unsigned i;

#if 0
  const unsigned LENGTH = 9;
  //unsigned char table[] = {'B', 'A', 'N', 'A', 'N', 'A', '\0'}; 
  unsigned char table[] = {232, 232, 8, 232, 232, 7, 232, 232, 10};
  ps.A = table; 
#else
  const unsigned LENGTH = TEST_LENGTH;
  ps.A = malloc(sizeof(unsigned char) * LENGTH + OVERSHOOT);
  // filling array
  for (i = 0; i < LENGTH; ++i) ps.A[i] = rand() % MOD;
#endif

  ps.length = LENGTH;
  ps.ISA = malloc(sizeof(unsigned) * LENGTH + OVERSHOOT);
  ps.SA = malloc(sizeof(unsigned) * LENGTH + OVERSHOOT);
  ps.N[0] = malloc(sizeof(unsigned) * LENGTH + OVERSHOOT);
  ps.N[1] = malloc(sizeof(unsigned) * LENGTH + OVERSHOOT);


  printf("initial_array\n");
  display_ps_A(&ps, LENGTH > DISPLAY_LENGTH ? DISPLAY_LENGTH : LENGTH);
  
  unsigned long long timing = cycles();
  prefix_full_sort(&ps, 1);
  timing = cycles() - timing;

  printf("timing is %llu / %.3e cycles\n", timing, (double) timing);

  printf("sorted array\n");
  display_ps_ISA(&ps, LENGTH > DISPLAY_LENGTH ? DISPLAY_LENGTH : LENGTH, 10);

  int check_status = check_ps_sort(&ps);
  if (check_status != -1) {
    printf("sorted order is wrong at index %d\n", check_status);
    int index0 = ps.ISA[check_status];
    int index1 = ps.ISA[check_status+1];
    printf("ISA[%d] = %d \n", check_status, index0);
    for (i = 0; i < 5; ++i) printf("%02u ", ps.A[index0+i]);
    printf("\n\n");
    printf("ISA[%d] = %d \n", check_status+1, index1);
    for (i = 0; i < 5; ++i) printf("%02u ", ps.A[index1+i]);
    printf("\n\n");
    return 1;
  }
  else printf("sorted order is OK\n");


  return 0;
}
