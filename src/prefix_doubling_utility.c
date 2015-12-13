#include <stdlib.h>
#include "prefix_doubling_utility.h"

void display_ps_A(prefix_suffix_t* ps, int num) {
  unsigned i;
  for (i = 0; i < ps->length && i < num; ++i) {
    printf("%02u ", ps->A[i]);
  }
  printf("\n");
}

void display_ps_ISA(prefix_suffix_t* ps, int num, int depth) {
  unsigned i;
  for (i = 0; i < ps->length && i < num; ++i) {
    printf("%02u[%d] ", ps->A[ps->ISA[i]], ps->ISA[i]);
    unsigned j;
    for (j = 0; j < depth && ps->ISA[i]+j < ps->length; j++) printf("%02u ", ps->A[ps->ISA[i]+j]);
    printf("\n");
  }
  printf("\n");
}

int ps_compare_suffix_gtu(prefix_suffix_t* ps, int index0, int index1) {
  int i = ps->ISA[index0];
  int j = ps->ISA[index1];
  while (i < ps->length && j < ps->length && ps->A[i] == ps->A[j]) { i++;  j++;};
  if (i >= ps->length) return 0;
  if (j >= ps->length) return 1;
  return ps->A[i] > ps->A[j];

};

int check_ps_sort(prefix_suffix_t* ps) {
  unsigned i = 0; 
  for (i = 0; i < ps->length - 1; i++) if (!ps_compare_suffix_gtu(ps, i+1, i)) return i;

  return -1;
}

void ps_init(prefix_suffix_t* ss, unsigned char* block, int length, int overshoot) 
{
  ss->A = block;
  ss->length = length;
  ss->ISA = malloc(sizeof(unsigned) * length + overshoot);
  ss->SA = malloc(sizeof(unsigned) * length + overshoot);
  ss->N[0] = malloc(sizeof(int) * length + overshoot);
  ss->N[1] = malloc(sizeof(int) * length + overshoot);
}


int ps_check_against_index_array(prefix_suffix_t* ss, int* sorted_index_array) 
{
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
