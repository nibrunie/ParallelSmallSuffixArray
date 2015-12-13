#include "PSSA_utility.h"
#include "parallel_suffix_sort.h"
#include "pthread.h"
#include <stdio.h>

// size of the (byte) frequency table
#define FTAB_SIZE 256
#define MAX_THREADS 16

#ifdef DEBUG
#define DEBUG_PRINTF printf
#else 
#define DEBUG_PRINTF 
#endif




typedef struct {
  suffix_struct_t* ss;
  int* ftab;
  int start_index, end_index, depth;
} thread_arg_t;

void* thread_sort(void* arg) {
  thread_arg_t *ta = (void*) arg;
  unsigned i;
  DEBUG_PRINTF("thread sort [%d:%d] \n", ta->start_index, ta->end_index);

  for (i = ta->start_index; i < ta->end_index; i++) {
    if (ta->ftab[i] - ta->ftab[i-1] > 0) sub_sort(ta->ss, ta->ftab[i-1], ta->ftab[i] - 1, ta->depth, PARAM_LIMIT_OK);
  }

  pthread_exit(0);
  return NULL;
}

/** 
 * @return greater than unsigned comparison results between suffixes
 * @param  n size of arr
 * @param  arr character array
 * @param  s1 start index of first suffix
 * @param  s2 start index of second suffix
 */
int compare_gtu_suffixes_full(unsigned n, unsigned char* arr, unsigned s1, unsigned s2);

/** 
 * @return greater (1), equal (0) or less (-1) than unsigned comparison results between suffixes
 *         considering only byte at offset depth after suffix start
 * @param  n size of arr
 * @param  arr character array
 * @param  s1 start index of first suffix
 * @param  s2 start index of second suffix
 */
static inline int compare_gtu_suffixes(unsigned n, unsigned char* arr, unsigned s1, unsigned s2, unsigned depth) {
  if (s2 + depth >= n) return 1;
  else if (s1 + depth >= n) return -1;
  else if (arr[s1+depth] == arr[s2+depth]) return 0;
  else return (arr[s1+depth] > arr[s2+depth]) ? 1 : -1;
}


#define COMP_EXT 8
#define BMM_SWAP_MATRIX 0x0102040810204080ull

static inline int compare_gtu_suffixes_extended(unsigned n, unsigned char* arr, unsigned s0, unsigned s1, unsigned depth) {
  int index0 = s0 + depth >= n ? (s0 + depth - n) : s0 + depth;  
  int index1 = s1 + depth >= n ? (s1 + depth - n) : s1 + depth;
  unsigned long long v0 = *((unsigned long long*) (arr + index0));
  unsigned long long v1 = *((unsigned long long*) (arr + index1));
  v0 = BYTE_SWAP(v0);
  v1 = BYTE_SWAP(v1);

  if (v0 > v1) return 1;
  if (v0 == v1) return 0;
  return -1;
}

static inline unsigned long long get_suffix_extended_key(unsigned n , unsigned char* arr, unsigned pre_index, unsigned depth) {
  int index = pre_index + depth >= n ? (pre_index + depth - n) : pre_index + depth;  
  unsigned long long value = *((unsigned long long*) (arr + index));
  value = BYTE_SWAP(value);

  return value;
}

static inline int compare_gtu_suffixes_extended_index_value(unsigned n, unsigned char* arr, unsigned s0, unsigned long long v1, unsigned depth) {
  int index0 = s0 + depth >= n ? (s0 + depth - n) : s0 + depth;  
  unsigned long long v0 = *((unsigned long long*) (arr + index0));
  v0 = BYTE_SWAP(v0);

  if (v0 > v1) return 1;
  if (v0 == v1) return 0;
  return -1;
}

static inline int compare_gtu_suffixes_extended_value_index(unsigned n, unsigned char* arr, unsigned long long v0, unsigned s1, unsigned depth) {
  int index1 = s1 + depth >= n ? (s1 + depth - n) : s1 + depth;
  unsigned long long v1 = *((unsigned long long*) (arr + index1));
  v1 = BYTE_SWAP(v1);

  if (v0 > v1) return 1;
  if (v0 == v1) return 0;
  return -1;
}


static inline int compare_gtu_suffixes_value_index(unsigned n, unsigned char* arr, unsigned char value, unsigned s2, unsigned depth) {
  if (s2 + depth >= n) return 1;
  else if (value == arr[s2+depth]) return 0;
  else return (value > arr[s2+depth]) ? 1 : -1;
}

static inline int compare_gtu_suffixes_index_value(unsigned n, unsigned char* arr, unsigned s1, unsigned char value, unsigned depth) {
  if (s1 + depth >= n) return -1;
  else if (arr[s1+depth] == value) return 0;
  else return (arr[s1+depth] > value) ? 1 : -1;
}


void ss_simple_sort(suffix_struct_t* ss, const int n_thread) {
  unsigned n = ss->length;
  unsigned char* arr = ss->A;
  unsigned *suffix_array = ss->ISA;
  // frequency tab
#ifdef TIMING
  unsigned long long timing = cycles();
#endif

  unsigned ftab[FTAB_SIZE+1] = {0};


  unsigned i;
  for (i = 0; i < n; ++i) {
    ftab[arr[i]] += 1; 
  }


  int thread_last_index[MAX_THREADS+1];
  thread_last_index[0] = 1;
  unsigned opt_thread_count = (n + n_thread) / n_thread;
  unsigned thread_count = 0;


  /** The following steps do bucket sorting and thread repartition 
   *  Each bucket contains the suffix starting with an identical byte
   *  Bucket <i> contains suffixes starting with the i-th byte
   */
  int local_thread_count = ftab[0];
  int current_thread = 0;
  // incremental ftab
  for (i = 1; i < FTAB_SIZE + 1; i++) {
    local_thread_count += ftab[i];
    ftab[i] += ftab[i-1]; 

    if (local_thread_count >= opt_thread_count) {
      thread_last_index[current_thread+1] = i; 
      local_thread_count = 0;
      current_thread++;
    }
    
  }

  thread_last_index[current_thread+1] = FTAB_SIZE + 1;
  DEBUG_PRINTF("opt_thread_count=%d thread_count=%d\n", opt_thread_count, thread_count);
  DEBUG_PRINTF("thread last index \n");
  DEBUG_PRINTF("n_thread=%d\n", n_thread);
  for (i = 0; i < MAX_THREADS + 1; ++i) DEBUG_PRINTF("%d ", thread_last_index[i]);
  DEBUG_PRINTF("\n");


  /** ftab[i] contains the (last index + 1) of the i-th  buckets */

  // bucket sorting
  for (i = 0; i < n; ++i) {
    unsigned byte = arr[i];
    suffix_array[ftab[byte] - 1] = i;
    ftab[byte]--;
  }
  // last byte marker
  suffix_array[ftab[FTAB_SIZE]] = n;

  DEBUG_PRINTF("suffix array\n");
  for (i = 0; i < n; ++i) DEBUG_PRINTF("%d ", suffix_array[i]);
  DEBUG_PRINTF("\n");
  for (i = 0; i < n; ++i) DEBUG_PRINTF("%d ", arr[suffix_array[i]]);
  DEBUG_PRINTF("\n");

#ifdef TIMING
  timing = cycles() - timing;
  printf("initial sort timing is %llu\n", timing);
#endif

#if 1
  pthread_t threads[MAX_THREADS];
  thread_arg_t threads_args[MAX_THREADS];

  for (i = 0; i < n_thread; ++i) {

    threads_args[i].ss = ss;
    threads_args[i].start_index = thread_last_index[i];
    threads_args[i].end_index   = thread_last_index[i+1];
    threads_args[i].ftab        = ftab;
    threads_args[i].depth       = 1;

    if (i != n_thread - 1) pthread_create(threads + i, NULL, thread_sort, threads_args + i);
    else {
      thread_arg_t* ta = threads_args + i;
      for (i = ta->start_index; i < ta->end_index; i++) {
        if (ta->ftab[i] - ta->ftab[i-1] > 0) sub_sort(ta->ss, ta->ftab[i-1], ta->ftab[i] - 1, ta->depth, PARAM_LIMIT_OK);
      };
    }
  }

  for (i = 0; i < n_thread - 1; ++i) {
    pthread_join(threads[i], NULL);
  };

#else
  /** now suffix starting with byte <i> are listed
   * between indexes ftab[i] and ftab[i+1] - 1 in suffix_array
   */
  for (i = 1; i < FTAB_SIZE + 1; i++) {
    if (ftab[i] - ftab[i-1] > 0) sub_sort(ss, ftab[i-1], ftab[i] - 1, 1);
  }

#endif

}


/** return the median (index) of 3 elements */
unsigned get_median3_index(suffix_struct_t* ss, unsigned index0, unsigned index1, unsigned index2, unsigned depth) {
  unsigned n = ss->length;
  unsigned* ISA = ss->ISA;
  unsigned char* arr = ss->A;

  unsigned ord_01 = compare_gtu_suffixes(n, arr, ISA[index0], ISA[index1], depth);
  unsigned ord_02 = compare_gtu_suffixes(n, arr, ISA[index0], ISA[index2], depth);
  if (ord_01 != ord_02) return index0;
  unsigned ord_12 = compare_gtu_suffixes(n, arr, ISA[index1], ISA[index2], depth);

  if (ord_01 != ord_12) return index1;
  return index2;
}



#ifdef DEBUG
#define DEBUG_MACRO(x) x
#else 
#define DEBUG_MACRO(x)
#endif

void print_suffix_elt(suffix_struct_t* ss, int index) {
  unsigned arr_index = ss->ISA[index];
  unsigned i;
  printf("ISA[%d] = %d -> ", index, arr_index);
  for (i = 0; i < 4 && arr_index + i < ss->length; ++i) printf("%02d ", ss->A[arr_index + i]);
  printf("\n");
}

/** suffix sort when | end - start | is limited ( < REC_SORT_LIMIT)
 *  @param ss suffix sort structure (containing input parameters and suffix arrays)
 *  @param start starting position in suffix_array of the list to be sorted
 *  @param end   ending position in suffix_array of the list to be sorted
 */
static inline int sub_sort_short_2(suffix_struct_t* ss, int start, int end, unsigned depth) {
  int index0 = ss->ISA[start];
  int elt0   = ss->A[index0+depth];

  int index1 = ss->ISA[start + 1];
  int elt1   = ss->A[index1+depth];
  // failure
  if (elt0 == elt1) return 1;
  // success
  if (elt0 > elt1) { 
    ss->ISA[start] = index1;
    ss->ISA[start+1] = index0;
  };
  return 0;
}

static inline int sub_sort_short_3(suffix_struct_t* ss, int start, int end, unsigned depth) {
  int index0 = ss->ISA[start];
  int elt0   = ss->A[index0+depth];

  int index1 = ss->ISA[start + 1];
  int elt1   = ss->A[index1+depth];
  // failure
  if (elt0 == elt1) return 1;
  // success
  if (elt0 > elt1) { 
    ss->ISA[start] = index1;
    ss->ISA[start+1] = index0;
  };
  return 0;
}

void sub_sort_short_4(suffix_struct_t* ss, int start, int end, unsigned depth) {
  int index0 = ss->ISA[start];
  int elt0   = ss->A[index0+depth];

  int index1 = ss->ISA[start + 1];
  int elt1   = ss->A[index1+depth];

  int index2 = ss->ISA[start + 2];
  int elt2   = ss->A[index2+depth];

  int index3 = ss->ISA[start + 3];
  int elt3   = ss->A[index3+depth];

}

#ifndef REC_SORT_LIMIT
#define REC_SORT_LIMIT 12
#endif


__thread unsigned long long value_array[REC_SORT_LIMIT+1];

int sub_sort_fast(suffix_struct_t* ss, int start, int end, unsigned depth) {
  unsigned i, j;
  // using 4 byte sort value
  register uint64_t bmm_swap_endianess = 0x0102040810204080ull;

  for (i = start; i <= end; ++i) {
    value_array[i - start] = BYTE_SWAP((*((unsigned long long*) (ss->A + ss->ISA[i]))));

  };
  for (i = start; i < end; ++i) {
    int min_index = i;

    unsigned long long min_value = value_array[i - start];
    for (j = i+1; j <= end; j++) {
      unsigned long long current_value = value_array[j - start];
      if (current_value < min_value) {
        min_index = j;
        min_value = current_value;
      } else if (current_value == min_value) {
        // failed (not deep enough) 
        return 0;
      }
    }
    int tmp = ss->ISA[i];
    value_array[min_index - start] = value_array[i - start];
    ss->ISA[i] = ss->ISA[min_index];
    ss->ISA[min_index] = tmp;
  }
  // sort succesful
  return 1;
}


#ifndef MEDIAN_LIMIT
#define MEDIAN_LIMIT 8
#endif


int max_depth = 0;
/** sort suffix array between start and end indexes (included), after initial radix sort 
 *  so first byte of every entry is identical */
void sub_sort(suffix_struct_t* ss, int start, int end, unsigned depth, int param) {
  DEBUG_PRINTF("call to sub_sort [%d:%d] (depth = %d)\n", start, end, depth);
  //if (depth > max_depth) max_depth = depth;
  unsigned* suffix_array = ss->ISA;
  unsigned char* arr = ss->A;
  unsigned n = ss->length;

  int  init_start = start, init_end = end;
  if (start >= end) {
    DEBUG_PRINTF("  start <= end: early exit\n");
    return;
   } else {
      if (end - start < REC_SORT_LIMIT && param == PARAM_LIMIT_OK) {
        if (sub_sort_fast(ss, start, end, depth)) return;
        else param = PARAM_LIMIT_NO;
      }
      // FXIME
      //
      // choose pivot
      //unsigned start_index = suffix_array[start];
      //unsigned end_index   = suffix_array[end];
      unsigned middle_index = start + (end - start) / 2;

      unsigned equal_start = start;
      unsigned equal_end   = end;

      /*
      // probabilistic median
      unsigned median_index = get_median3_index(ss, start, end, middle_index, depth);

      DEBUG_PRINTF("median choice among (%d, %d, %d)\n", start, end, middle_index);
      DEBUG_PRINTF("   median is A[%d] = ", suffix_array[median_index]);
      unsigned t, tc;
      for (t = suffix_array[median_index], tc = 0; t < n && tc < 4; ++t, ++tc) DEBUG_PRINTF("%02d ", arr[t]);;
      DEBUG_PRINTF("\n");

      // move median to front
      unsigned front_index = suffix_array[start]; 
      suffix_array[start] = suffix_array[median_index];
      suffix_array[median_index] = front_index;
      start++;


      //unsigned char median_value = arr[suffix_array[init_start]+depth];


      int corrected_index = suffix_array[init_start] + depth >= n ? (suffix_array[init_start] + depth - n) : suffix_array[init_start] + depth;
      unsigned long long median_value = *((unsigned long long*) (arr + corrected_index));
      median_value = __builtin_k1_sbmm8_d(median_value, BMM_SWAP_MATRIX);
      */
      unsigned long long median_value = 127;
      /*if (end - start > MEDIAN_LIMIT) {

        int corrected_start_index = suffix_array[init_start] + depth >= n ? (suffix_array[init_start] + depth - n) : suffix_array[init_start] + depth;
        unsigned long long start_value = *((unsigned long long*) (arr + corrected_start_index));
        start_value = __builtin_k1_sbmm8_d(start_value, BMM_SWAP_MATRIX);

        int corrected_end_index = suffix_array[init_end] + depth >= n ? (suffix_array[init_end] + depth - n) : suffix_array[init_end] + depth;
        unsigned long long end_value = *((unsigned long long*) (arr + corrected_end_index));
        end_value = __builtin_k1_sbmm8_d(end_value, BMM_SWAP_MATRIX);

        int corrected_middle_index = suffix_array[middle_index] + depth >= n ? (suffix_array[middle_index] + depth - n) : suffix_array[middle_index] + depth;
        unsigned long long middle_value = *((unsigned long long*) (arr + corrected_middle_index));
        middle_value = __builtin_k1_sbmm8_d(middle_value, BMM_SWAP_MATRIX);

        median_value = end_value;
        if ((start_value > end_value) != (start_value > middle_value)) median_value = start_value;
        if ((middle_value > end_value) != (middle_value > end_value)) median_value = middle_value;
      } else {
      */
        int corrected_middle_index = suffix_array[middle_index] + depth >= n ? (suffix_array[middle_index] + depth - n) : suffix_array[middle_index] + depth;
        unsigned long long middle_value = *((unsigned long long*) (arr + corrected_middle_index));
        middle_value = BYTE_SWAP(middle_value);
        median_value = middle_value;

      //}


#define swap(x, y) {\
  unsigned tmp_index = suffix_array[x];\
  suffix_array[x] = suffix_array[y];\
  suffix_array[y] = tmp_index;\
}
      

      // partionning according to the median
      while (start < end) {
        //switch (compare_gtu_suffixes_index_value(n, arr, suffix_array[start], median_value, depth)) {
        //unsigned long long get_suffix_extended_key(unsigned n , unsigned char* arr, unsigned index, unsigned depth) {
        unsigned long long start_value = get_suffix_extended_key(n, arr, suffix_array[start], depth); 
        //switch (compare_gtu_suffixes_extended_index_value(n, arr, suffix_array[start], median_value, depth)) {
        //case -1: // less than median
        if (start_value < median_value) {
          DEBUG_MACRO(printf("less  ")); 
          DEBUG_MACRO(print_suffix_elt(ss, start));
          start++; 
        //  break;
        //case  0: // equal to median
        } else if (start_value == median_value) {
          DEBUG_MACRO(printf("equal  ")); 
          DEBUG_MACRO(print_suffix_elt(ss, start));
          swap(equal_start, start);
          start++; equal_start++;
        //  break;
        //case  1: // greater than median
        } else {
          DEBUG_MACRO(printf("greater  ")); 
          DEBUG_MACRO(print_suffix_elt(ss, start));
          int comp = compare_gtu_suffixes_extended_value_index(n, arr, median_value, suffix_array[end], depth); 
          //unsigned long long end_value = get_suffix_extended_key(n, arr, suffix_array[end], depth); 
          while (comp != 1) {
            // SA[median] <= SA[end]
            
            if (comp == 0) {
              //  SA[median] == SA[end]
              DEBUG_MACRO(printf("end equal  ")); 
              DEBUG_MACRO(print_suffix_elt(ss, end));
              swap(equal_end, end);
              end--;
              equal_end--;
            } else {
              DEBUG_MACRO(printf("end greater  ")); 
              DEBUG_MACRO(print_suffix_elt(ss, end));
              end--;
            }

            if (start >= end) break;
            
            // updating comp
            //comp = compare_gtu_suffixes_value_index(n, arr, median_value, suffix_array[end], depth); 
            comp = compare_gtu_suffixes_extended_value_index(n, arr, median_value, suffix_array[end], depth); 
          }
          if (start < end) {
            DEBUG_PRINTF("swapping start/end ISA[%d] = %d <-> %d = ISA[%d]  \n", start, suffix_array[start], suffix_array[end], end);
            swap(start, end);
            start++;
            end--;
          }
          //}
        }
      }
      // FIXME last element
      //int comp = compare_gtu_suffixes_index_value(n, arr, suffix_array[start], median_value, depth);
      int comp = compare_gtu_suffixes_extended_index_value(n, arr, suffix_array[start], median_value, depth);
      if (comp == 0) {
        DEBUG_MACRO(printf("last equal  ")); 
        DEBUG_MACRO(print_suffix_elt(ss, start));
        swap(equal_start, start);
        equal_start++;
        start++;
      } else if (comp == 1) {
        // elt at start greater than median
        DEBUG_MACRO(printf("last greater  ")); 
        DEBUG_MACRO(print_suffix_elt(ss, start));
        end--;
      } else {
        // elt at start index, less than median
        DEBUG_MACRO(printf("last less  ")); 
        DEBUG_MACRO(print_suffix_elt(ss, start));
        start++;
      };
      DEBUG_PRINTF("end of median-based move [%d:%d][%d:%d][%d:%d][%d:%d]\n", init_start, equal_start - 1, equal_start, start - 1, start, equal_end, equal_end + 1, init_end);

      // moving elt equal to median into position
      unsigned i, j;
      if (equal_start != start) for (i = init_start, j = start - 1; i < equal_start; ++i, --j) swap(i, j);

      unsigned k, l;
      if (end != equal_end) for (k = start, l = init_end; l > equal_end; ++k, --l) swap(k, l);

      unsigned left_equal_count = equal_start - init_start;
      unsigned right_equal_count = init_end - equal_end;

      DEBUG_PRINTF("left_equal_count  = %d\n", left_equal_count);
      DEBUG_PRINTF("right_equal_count = %d\n", right_equal_count);

      // after median reordering
      DEBUG_PRINTF("after re-ordering [%d:%d][%d:%d][%d:%d]\n", init_start, start - left_equal_count - 1, 
      start - left_equal_count, start + right_equal_count - 1, start + right_equal_count, init_end);

      /** recursive sort of the three sub parts */
      // less-than sort
      sub_sort(ss, init_start, start - left_equal_count - 1, depth, param); 
      // equal-to sort
      sub_sort(ss, start - left_equal_count, start + right_equal_count - 1, depth + 8, param);
      // greater-than sort
      sub_sort(ss, start + right_equal_count, init_end, depth, param);
      

   }
}


