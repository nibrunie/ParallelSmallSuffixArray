#include "parallel_suffix_sort.h"

void display_ss_A(suffix_struct_t* ss, int num);

void display_ss_ISA(suffix_struct_t* ss, int num, int depth);

int ss_compare_suffix_gtu(suffix_struct_t* ss, int index0, int index1); 

int ss_check_sort(suffix_struct_t* ss);

void ss_init(suffix_struct_t* ss, unsigned char* block, int length, int overshoot); 

int ss_check_against_index_array(suffix_struct_t* ss, int* sorted_index_array);

#define DISPLAY_A      display_ss_A
#define DISPLAY_ISA    display_ss_ISA
#define COMPARE_SUFFIX ss_compare_suffix_gtu
#define CHECK_SORT     ss_check_sort
#define INIT           ss_init
#define CHECK_AGAINST  ss_check_against_index_array
#define SUFFIX_SORT    ss_simple_sort
#define STRUCT_TYPE    suffix_struct_t
