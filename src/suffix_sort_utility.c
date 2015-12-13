#include <stdlib.h>
#include <stdio.h>

#include "parallel_suffix_sort.h"
#include "PSSA_utility.h"

void display_ss_A(suffix_struct_t* ss, int num) {
  unsigned i;
  for (i = 0; i < ss->length && i < num; ++i) {
    printf("%02u ", ss->A[i]);
  }
  printf("\n");
}

void display_ss_ISA(suffix_struct_t* ss, int num, int depth) {
  unsigned i;
  for (i = 0; i < ss->length && i < num; ++i) {
    printf("%02u[%d] ", ss->A[ss->ISA[i]], ss->ISA[i]);
    unsigned j;
    for (j = 0; j < depth && ss->ISA[i]+j < ss->length; j++) printf("%02u ", ss->A[ss->ISA[i]+j]);
    printf("\n");
  }
  printf("\n");
}

int ss_compare_suffix_gtu(suffix_struct_t* ss, int index0, int index1) {
  int i = ss->ISA[index0];
  int j = ss->ISA[index1];
  while (i < ss->length && j < ss->length && ss->A[i] == ss->A[j]) { i++;  j++;};
  if (i >= ss->length) return 0;
  if (j >= ss->length) return 1;
  return ss->A[i] > ss->A[j];

};

int ss_check_sort(suffix_struct_t* ss) {
  unsigned i = 0; 
  for (i = 0; i < ss->length - 1; i++) if (!ss_compare_suffix_gtu(ss, i+1, i)) return i;

  return -1;
}


void ss_init(suffix_struct_t* ss, unsigned char* block, int length, int overshoot) 
{
  ss->A = block;
  ss->length = length;
  ss->ISA = malloc(sizeof(unsigned) * length + overshoot);
}

int ss_check_against_index_array(suffix_struct_t* ss, int* sorted_index_array) {
  int i;
  for (i = 0; i < ss->length; ++i) {
    if (ss->ISA[i] != sorted_index_array[i]) {
      printf("sorted array mismatch at index %d (%d vs %d)\n", i, ss->ISA[i], sorted_index_array[i]);
      unsigned j;
      int k;
      for (k = i - 2; k <= i+2; ++k) {
        printf("result ");
        for (j = 0; j < 15; ++j) printf("%02d ", ss->A[ss->ISA[k]+j]);
        printf("\n");
        printf("reference ");
        for (j = 0; j < 15; ++j) printf("%02d ", ss->A[sorted_index_array[k]+j]);
        printf("\n");
        printf("\n");
      };

      return 1;
    }
  }

  return 0;
}
