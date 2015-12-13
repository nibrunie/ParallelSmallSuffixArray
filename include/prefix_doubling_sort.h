#ifndef __PREFIX_DOUBLING_SORT_H__
#define __PREFIX_DOUBLING_SORT_H__

/* sturcture storing sort inputs and temporary
 * arrays
 */
typedef struct {
  unsigned char* A;
  int length;
  int* ISA;
  int* SA;
  int*   N[2];
  int  n_index;
} prefix_suffix_t;


/* full suffix store using prefix doubling technique similar
 * to Sadakane's method
 * @param ps prefix suffix structure
 * @param nthread number of thread used for sorting (only 1 is currently supported)
 */
void prefix_full_sort(prefix_suffix_t* ps, int nthread); 

#endif /* __PREFIX_DOUBLING_SORT_H__ */
