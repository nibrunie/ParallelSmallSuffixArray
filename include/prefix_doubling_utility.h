#include "prefix_doubling_sort.h"

#ifndef __PREFIX_DOUBLING_UTILITY_H__
#define __PREFIX_DOUBLING_UTILITY_H__

void display_ps_A(prefix_suffix_t* ps, int num);

void display_ps_ISA(prefix_suffix_t* ps, int num, int depth);

int ps_compare_suffix_gtu(prefix_suffix_t* ps, int index0, int index1);

int check_ps_sort(prefix_suffix_t* ps);

void ps_init(prefix_suffix_t* ss, unsigned char* block, int length, int overshoot); 

int ps_check_against_index_array(prefix_suffix_t* ss, int* sorted_index_array); 

#define DISPLAY_A      display_ps_A
#define DISPLAY_ISA    display_ps_ISA
#define COMPARE_SUFFIX ps_compare_suffix_gtu
#define CHECK_SORT     check_ps_sort
#define INIT           ps_init
#define CHECK_AGAINST  ps_check_against_index_array
#define SUFFIX_SORT    prefix_full_sort
#define STRUCT_TYPE    prefix_suffix_t
#endif /* ifndef __PREFIX_DOUBLING_UTILITY_H__ */
