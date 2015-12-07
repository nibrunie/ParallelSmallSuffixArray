#include <stdlib.h>
#include <stdio.h>

#include "parallel_suffix_sort.h"


void display_A(suffix_struct_t* ss, int num) {
  unsigned i;
  for (i = 0; i < ss->length && i < num; ++i) {
    printf("%02u ", ss->A[i]);
  }
  printf("\n");
}

void display_ISA(suffix_struct_t* ss, int num, int depth) {
  unsigned i;
  for (i = 0; i < ss->length && i < num; ++i) {
    printf("%02u[%d] ", ss->A[ss->ISA[i]], ss->ISA[i]);
    unsigned j;
    for (j = 0; j < depth && ss->ISA[i]+j < ss->length; j++) printf("%02u ", ss->A[ss->ISA[i]+j]);
    printf("\n");
  }
  printf("\n");
}

int suffix_compare_gtu(suffix_struct_t* ss, int index0, int index1) {
  int i = ss->ISA[index0];
  int j = ss->ISA[index1];
  while (i < ss->length && j < ss->length && ss->A[i] == ss->A[j]) { i++;  j++;};
  if (i >= ss->length) return 0;
  if (j >= ss->length) return 1;
  return ss->A[i] > ss->A[j];

};

int check_sort(suffix_struct_t* ss) {
  unsigned i = 0; 
  for (i = 0; i < ss->length - 1; i++) if (!suffix_compare_gtu(ss, i+1, i)) return i;

  return -1;
}


int main(void) {
  suffix_struct_t ss;
  unsigned i;

#if 0
  const unsigned LENGTH = 6;
  //unsigned char table[] = {'B', 'A', 'N', 'A', 'N', 'A', '\0'}; 
  unsigned char table[] = {232, 232, 10, 232, 9, 232};
  ss.A = table; 
#else
  const unsigned LENGTH = 100000;
  ss.A = malloc(sizeof(unsigned char) * LENGTH);
  // filling array
  for (i = 0; i < LENGTH; ++i) ss.A[i] = rand() % 256;
#endif

  ss.length = LENGTH;
  ss.ISA = malloc(sizeof(unsigned) * LENGTH);


  printf("initial_array\n");
  display_A(&ss, 10);
  

  simple_sort(&ss, 1);
  printf("sorted array\n");
  display_ISA(&ss, 10, 10);

  int check_status = check_sort(&ss);
  if (check_status != -1) {
    printf("sorted order is wrong at index %d\n", check_status);
    int index0 = ss.ISA[check_status];
    int index1 = ss.ISA[check_status+1];
    printf("ISA[%d] = %d \n", check_status, index0);
    for (i = 0; i < 5; ++i) printf("%02u ", ss.A[index0+i]);
    printf("\n\n");
    printf("ISA[%d] = %d \n", check_status+1, index1);
    for (i = 0; i < 5; ++i) printf("%02u ", ss.A[index1+i]);
    printf("\n\n");

  }
  else printf("sorted order is OK\n");

  return 0;
}
