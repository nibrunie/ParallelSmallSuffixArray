#include "prefix_doubling_sort.h"
#include "PSSA_utility.h"


/** sort ISA[start:end] suffixes based with key
 *  for i = N[ISA[i]+depth]
 * @return the newest value of N for the index end
 * @param ps structure used to store input data and temporary sorting structure
 * @param start beginning index in ISA
 * @param end   ending index in ISA
 * @param depth offset for key index to compare
 * @param previous_N is the value of the previous bucket in order incicated by increase
 * @param increase indicate wheter block are processed in increase or decrease order
 */
int prefix_sub_sort(prefix_suffix_t* ps, int start, int end, int depth, int previous_N, int increase, int* rcount);

#define PREFIX_FTAB_SIZE 256

void prefix_initial_sort(prefix_suffix_t* ps) 
{

  //unsigned long long timing = cycles();
  int ftab[PREFIX_FTAB_SIZE+1] = {0};
  int ftab_cpy[PREFIX_FTAB_SIZE+1] = {0};

  unsigned i;
  // computing frequency table for each byte
  for (i = 0; i < ps->length; ++i) {
    ftab[ps->A[i]]++;
  };

#ifdef DEBUG
  printf("ftab\n");
  for (i = 0; i < PREFIX_FTAB_SIZE + 1; ++i) if (ftab[i] != 0) printf("ftab[%d] = %d\n", i, ftab[i]);
#endif

  // computing incrementing frequency table for count sort
  ftab_cpy[0]  = ftab[0];
  for (i = 1; i < PREFIX_FTAB_SIZE + 1; ++i) {
    ftab[i] += ftab[i-1];
    // copy for N
    ftab_cpy[i] = ftab[i];
  };

#ifdef DEBUG
  //printf("ftab_cpy\n");
  //for (i = 0; i < PREFIX_FTAB_SIZE + 1; ++i) if (ftab_cpy[i] != 0) printf("ftab_cpy[%d] = %d\n", i, ftab_cpy[i]);
#endif

  // computing initial suffix array, inverse suffix array and N values
  for (i = 0; i < ps->length; ++i) {
    ps->ISA[--ftab[ps->A[i]]] = i;
    ps->SA[i] = ftab[ps->A[i]];
    // N[i] set to max index of i's bucket
    //ps->N[ps->n_index][i] = ps->A[i] != 0 ? ftab_cpy[ps->A[i] - 1] : 0;
    ps->N[ps->n_index][i] = ftab_cpy[ps->A[i]] - 1;

  }

  //timing = cycles() - timing;
  //printf("initial sort timing is %llu cycles\n", timing);
};

void display_ps(prefix_suffix_t* ps, int n_index) 
{
#ifdef DEBUG
  unsigned i;
  // display N
  printf("i:       ");
  for (i = 0; i < ps->length; ++i) printf("%3d ", i);
  printf("\n");
  printf("N:       ");
  for (i = 0; i < ps->length; ++i) printf("%3d ", ps->N[n_index][i]);
  printf("\n");
  printf("ISA:     ");
  for (i = 0; i < ps->length; ++i) printf("%3d ", ps->ISA[i]);
  printf("\n");
  printf("N's ISA: ");
  for (i = 0; i < ps->length; ++i) printf("%3d ", ps->N[n_index][ps->ISA[i]]);
  printf("\n");
  printf("N2's ISA:");
  for (i = 0; i < ps->length; ++i) printf("%3d ", ps->N[1 - n_index][ps->ISA[i]]);
  printf("\n");
  printf("SA:      ");
  for (i = 0; i < ps->length; ++i) printf("%3d ", ps->SA[i]);
  printf("\n");
#endif

}

int prefix_sort_single_pass_increase(prefix_suffix_t* ps, int depth) 
{
  int previous_N = 0;
  // remaining count
  int rcount = 0;
  int i = 0;
  while (i < ps->length) {
    if (ps->N[ps->n_index][ps->ISA[i]] < 0) {
      int init_equal_index = i;
      i += -ps->N[ps->n_index][ps->ISA[i]];
      while (i < ps->length && ps->N[ps->n_index][ps->ISA[i]] < 0) i += -ps->N[ps->n_index][ps->ISA[i]];
      int end_equal_index = i >= ps->length ? ps->length - 1 : i -1;
      ps->N[1 - ps->n_index][ps->ISA[init_equal_index]] = - (end_equal_index - init_equal_index + 1);
      ps->N[1 - ps->n_index][ps->ISA[end_equal_index]]  = - (end_equal_index - init_equal_index + 1);

      DEBUG_PRINTF("[INC] skipping already sorted list from %d to %d\n", init_equal_index, end_equal_index);
      previous_N = 0;

    } else {
      int start_index = i;
      int end_index  = ps->N[ps->n_index][ps->ISA[i]];
      previous_N = prefix_sub_sort(ps, start_index, end_index, depth, previous_N, /* increase */ 1, &rcount);
      i = end_index + 1;
    }
  }

#ifdef DEBUG
  //display_ps(ps, ps->n_index);
#endif

  return rcount;
}

int prefix_sort_single_pass_decrease(prefix_suffix_t* ps, int depth) 
{
  int previous_N = 0;
  int rcount = 0;
  int i = ps->length-1;
  while (i >= 0) {
    if (ps->N[ps->n_index][ps->ISA[i]] < 0) {
      int end_equal_index = i;
      i += ps->N[ps->n_index][ps->ISA[i]];
      while (i >= 0 && ps->N[ps->n_index][ps->ISA[i]] < 0) i += ps->N[ps->n_index][ps->ISA[i]];
      int init_equal_index =  i > 0 ? i + 1 : 0;
      ps->N[1 - ps->n_index][ps->ISA[init_equal_index]] = - (end_equal_index - init_equal_index + 1);
      ps->N[1 - ps->n_index][ps->ISA[end_equal_index]]  = - (end_equal_index - init_equal_index + 1);

      DEBUG_PRINTF("[DEC] skipping already sorted list from %d to %d\n", init_equal_index, end_equal_index);
      previous_N = 0;
    } else {
      int end_index = i;
      int start_index  = ps->N[ps->n_index][ps->ISA[i]];
      previous_N = prefix_sub_sort(ps, start_index, end_index, depth, previous_N, /* increase */ 0, &rcount);
      i = start_index - 1;
    }
  }

#ifdef DEBUG
  //display_ps(ps, ps->n_index);
#endif

  return rcount;
}

void prefix_full_sort(prefix_suffix_t* ps) 
{

  ps->n_index = 0;
  prefix_initial_sort(ps);
#ifdef DEBUG
  printf("after initial sort\n");
  display_ps(ps, ps->n_index);
#endif
  int k = 1;
  while (1) {
    DEBUG_PRINTF("\n launching prefix_sort_single_pass_increase k=%d, ps->n_index=%d\n", k, ps->n_index);
    int rcount = prefix_sort_single_pass_increase(ps, k);
    DEBUG_PRINTF("after increase pass sort, rcount=%d \n", rcount);
#ifdef DEBUG
    display_ps(ps, ps->n_index);
#endif
    if (!rcount) break;
    k = 2 * k;
    ps->n_index = 1 - ps->n_index;
    DEBUG_PRINTF("\n launching prefix_sort_single_pass_decrease k=%d, ps->n_index=%d\n", k, ps->n_index);
    rcount = prefix_sort_single_pass_decrease(ps, k);
    DEBUG_PRINTF("after decrease pass sort, rcount=%d \n", rcount);
#ifdef DEBUG
    display_ps(ps, ps->n_index);
#endif
    if (!rcount) break;
    k = 2 * k;
    ps->n_index = 1 - ps->n_index;
  }
}

static inline int get_prefix_key(prefix_suffix_t* ps, int index, int depth) 
{
  int* ISA = ps->ISA;
#ifdef BWT
  int N_index = ISA[index] + depth > ps->length ? ISA[index] + depth - ps->length : ISA[index] + depth;
  int key = ps->N[ps->n_index][N_index];
  return key < 0 ?  ps->SA[N_index] : key;
#else 
  //int key = ISA[index] + depth > ps->length ? -(ISA[index] + depth) : ps->N[ps->n_index][ISA[index] + depth];
  int N_index = ISA[index] + depth;
  if (N_index > ps->length) {
    return -(ISA[index] + depth);
  } else {
    int key = ps->N[ps->n_index][ISA[index]+depth];
    if (key < 0) return ps->SA[N_index];
    else return key;
  }
#endif
}

#define swap(x, y) {\
  int tmp = ISA[x];\
  ISA[x] = ISA[y];\
  ISA[y] = tmp;\
  SA[ISA[x]] = x; \
  SA[ISA[y]] = y; \
}

/** sort ISA[start:end] suffixes based with key
 *  for i = N[ISA[i]+depth]
 * @return the newest value of N for the index end
 * @param ps structure used to store input data and temporary sorting structure
 * @param start beginning index in ISA
 * @param end   ending index in ISA
 * @param depth offset for key index to compare
 * @param previous_N is the value of the previous bucket in order incicated by increase
 * @param increase indicate wheter block are processed in increase or decrease order
 */
int prefix_sub_sort(prefix_suffix_t* ps, int start, int end, int depth, int previous_N, int increase, int* rcount) {
  DEBUG_PRINTF("sorting ISA[%d:%d] depth=%d, previous_N=%d, increase=%d\n", start, end , depth, previous_N, increase);


  int* ISA = ps->ISA;
  int* SA = ps->SA;
  int** N = ps->N;

  int init_start = start, init_end = end, equal_start = start, equal_end = end;

  // single element bucket ( => SORTED)
  if (start == end) {
    previous_N = previous_N <= 0 ? previous_N - 1 : -1;
    N[1 - ps->n_index][ISA[init_start]] = previous_N; 
    return previous_N;
  }
  if (start > end) return previous_N;

  // computing median
  int middle_index = start + (end - start) / 2;
  int start_value = get_prefix_key(ps, start, depth);
  int middle_value = get_prefix_key(ps, middle_index, depth);
  int end_value = get_prefix_key(ps, end, depth);

  int median_value = end_value;
  if ((start_value > middle_value) != (start_value > end_value)) median_value = start_value;
  else if ((middle_value > start_value) != (middle_value > end_value)) median_value = middle_value;

  while (start <= end) {
    start_value = get_prefix_key(ps, start, depth);
    if (start_value == median_value) {
      swap(start, equal_start);
      start++;
      equal_start++;
    } else if (start_value < median_value) {
      start++;
    } else if (start_value > median_value) {
      while (start <= end) {
        end_value = get_prefix_key(ps, end, depth);
        if (end_value > median_value) {
          end--;
        } else if (end_value == median_value) {
          swap(end, equal_end);
          end--;
          equal_end--;
        } else {
          // end value is less then median
          // and start value is greater than median
          // => swap them
          swap(end, start);
          end--;
          start++;
          break;
        }
      }
    }
  }
  // start > end
  
  // moving elts equal to median into position (middle of the bucket)
  unsigned left_equal_count = equal_start - init_start;
  unsigned right_equal_count = init_end - equal_end;
  int next_equal_start = start - left_equal_count;
  int next_equal_end = start + right_equal_count - 1;
  int next_greater_start = start + right_equal_count;

  int lesser_count = start - equal_start;
  int greater_count = equal_end - end;
  int equal_count = left_equal_count + right_equal_count;

  int next_equal_index = increase ? next_equal_start : next_equal_end;
  DEBUG_PRINTF("next_equal_index=%d\n", next_equal_index);
  
  unsigned i, j;
  if (equal_start != start) for (i = init_start, j = start - 1; i < equal_start; ++i, --j) {
    swap(i, j);
    // updating N array
    //DEBUG_PRINTF("L updating N[%d][%d] to %d\n", 1 - ps->n_index, ISA[j], next_equal_index);
    //N[1 - ps->n_index][ISA[j]] = next_equal_index;
  }

  unsigned k, l;
  if (end != equal_end) for (k = start, l = init_end; l > equal_end; ++k, --l) {
    swap(k, l);
    // updating N array
    //Nn[ISA[l]] = next_greater_start;
    //DEBUG_PRINTF("R updating N[%d][%d] to %d\n", 1 - ps->n_index, ISA[k], next_equal_index);
    //N[1 - ps->n_index][ISA[k]] = next_equal_index;
  }

  // updating N next for equal part
  if (equal_count > 1) for (i = start - left_equal_count; i < start + right_equal_count; ++i) {
    DEBUG_PRINTF("updating N[%d][%d] to %d\n", 1 - ps->n_index, ISA[i], next_equal_index);
    N[1 - ps->n_index][ISA[i]] = next_equal_index;
  }

  DEBUG_PRINTF("left_equal_count  = %d\n", left_equal_count);
  DEBUG_PRINTF("right_equal_count = %d\n", right_equal_count);

  // after median reordering
  DEBUG_PRINTF("after re-ordering [%d:%d][%d:%d][%d:%d]\n", init_start, start - left_equal_count - 1, 
  start - left_equal_count, start + right_equal_count - 1, start + right_equal_count, init_end);

  // updating N for remaining greater element
  /*for (k = next_greater_start; k <= equal_end; k++) {
    N[ISA[k]] = next_greater_start;
  }*/


  if (increase) {
    // increasing order
    // updating N array
    previous_N = prefix_sub_sort(ps, init_start, init_start + lesser_count - 1, depth, previous_N, increase, rcount);
    if (equal_count == 1) {
      previous_N = previous_N <= 0 ? previous_N - 1 : -1;
      N[1 - ps->n_index][ISA[next_equal_start]] = previous_N; 
    } else if (equal_count > 1) {
      previous_N = next_equal_start;
      *rcount += equal_count;
    }
    previous_N = prefix_sub_sort(ps, init_start + lesser_count + equal_count, init_end, depth, previous_N, increase, rcount);
  } else {
    // decreasing order
    // recursively sorting greater part
    previous_N = prefix_sub_sort(ps, init_start + lesser_count + equal_count, init_end, depth, previous_N, increase, rcount);
    if (equal_count == 1) {
      previous_N = previous_N <= 0 ? previous_N - 1 : -1;
      N[1 - ps->n_index][ISA[next_equal_start]] = previous_N; 
    } else if (equal_count > 1) {
      previous_N = next_equal_end;
      *rcount += equal_count;
    }
    // recursively sorting lower part
    previous_N = prefix_sub_sort(ps, init_start, init_start + lesser_count - 1, depth, previous_N, increase, rcount);

  }

  return previous_N;




}


