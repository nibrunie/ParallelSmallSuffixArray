
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
 */
void prefix_full_sort(prefix_suffix_t* ps); 
