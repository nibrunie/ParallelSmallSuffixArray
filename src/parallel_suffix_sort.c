#include "parallel_suffix_sort.h"
#include "pthread.h"

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

void thread_sort(thread_arg_t *ta) {
  unsigned i;
  DEBUG_PRINTF("thread sort [%d:%d] \n", ta->start_index, ta->end_index);

  for (i = ta->start_index; i < ta->end_index; i++) {
    if (ta->ftab[i] - ta->ftab[i-1] > 0) sub_sort(ta->ss, ta->ftab[i-1], ta->ftab[i] - 1, ta->depth);
  }

  pthread_exit(0);
  return;
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
int compare_gtu_suffixes(unsigned n, unsigned char* arr, unsigned s1, unsigned s2, unsigned depth) {
  if (s2 + depth >= n) return 1;
  else if (s1 + depth >= n) return -1;
  else if (arr[s1+depth] == arr[s2+depth]) return 0;
  else return (arr[s1+depth] > arr[s2+depth]) ? 1 : -1;
}

void simple_sort(suffix_struct_t* ss, const int n_thread) {
  unsigned n = ss->length;
  unsigned char* arr = ss->A;
  unsigned *suffix_array = ss->ISA;
  // frequency tab
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


#if 1
  pthread_t threads[MAX_THREADS];
  thread_arg_t threads_args[MAX_THREADS];

  for (i = 0; i < n_thread; ++i) {

    threads_args[i].ss = ss;
    threads_args[i].start_index = thread_last_index[i];
    threads_args[i].end_index   = thread_last_index[i+1];
    threads_args[i].ftab        = ftab;
    threads_args[i].depth       = 1;

    pthread_create(threads + i, NULL, thread_sort, threads_args + i);

  }

  for (i = 0; i < n_thread; ++i) {
    pthread_join(threads[i], NULL);
  };

#else
  /** now suffix starting with byte <i> are listed
   * between indexes ftab[i] and ftab[i+1] - 1 in suffix_array
   */
  for (i = 1; i < FTAB_SIZE + 1; i++) {
    if (ftab[i] - ftab[i-1] > 0) sub_sort(ss, ftab[i-1], ftab[i] - 1, 0);
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


#define REC_SORT_LIMIT 0

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

/** sort suffix array between start and end indexes (included), after initial radix sort 
 *  so first byte of every entry is identical */
void sub_sort(suffix_struct_t* ss, int start, int end, unsigned depth) {
    DEBUG_PRINTF("call to sub_sort [%d:%d] (depth = %d)\n", start, end, depth);
    unsigned* suffix_array = ss->ISA;
    unsigned char* arr = ss->A;
    unsigned n = ss->length;

    int  init_start = start, init_end = end;
   if (start >= end) {
    DEBUG_PRINTF("  start <= end: early exit\n");
    return;

   } else if (end - start < REC_SORT_LIMIT) {
      // FXIME
   } else {
      // choose pivot
      //unsigned start_index = suffix_array[start];
      //unsigned end_index   = suffix_array[end];
      unsigned middle_index = start + (end - start) / 2;

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

      unsigned equal_start = start;
      unsigned equal_end   = end;

#define swap(x, y) {\
  unsigned tmp_index = suffix_array[x];\
  suffix_array[x] = suffix_array[y];\
  suffix_array[y] = tmp_index;\
}
      

      // partionning according to the median
      while (start < end) {
        switch (compare_gtu_suffixes(n, arr, suffix_array[start], suffix_array[init_start], depth)) {
        case -1: // less
          DEBUG_MACRO(printf("less  ")); 
          DEBUG_MACRO(print_suffix_elt(ss, start));
          start++; 
          break;
        case  0: // equal
          DEBUG_MACRO(printf("equal  ")); 
          DEBUG_MACRO(print_suffix_elt(ss, start));
          swap(equal_start, start);
          start++; equal_start++;
          break;
        case  1: // greater
          {
          DEBUG_MACRO(printf("greater  ")); 
          DEBUG_MACRO(print_suffix_elt(ss, start));
          int comp = compare_gtu_suffixes(n, arr, suffix_array[init_start], suffix_array[end], depth); 
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
            comp = compare_gtu_suffixes(n, arr, suffix_array[init_start], suffix_array[end], depth); 
          }
          if (start < end) {
            DEBUG_PRINTF("swapping start/end ISA[%d] = %d <-> %d = ISA[%d]  \n", start, suffix_array[start], suffix_array[end], end);
            swap(start, end);
            start++;
            end--;
          }
          }
        }
      }
      // FIXME last element
      int comp = compare_gtu_suffixes(n, arr, suffix_array[start], suffix_array[init_start], depth);
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
      sub_sort(ss, init_start, start - left_equal_count - 1, depth); 
      // equal-to sort
      sub_sort(ss, start - left_equal_count, start + right_equal_count - 1, depth + 1);
      // greater-than sort
      sub_sort(ss, start + right_equal_count, init_end, depth);
      

   }
}


