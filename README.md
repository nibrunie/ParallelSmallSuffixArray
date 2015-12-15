# ParallelSmallSuffixArray
Parallel implementation of small suffix array computation for BWT acceleration on SMP


## Source description:
  1. src/parallel_suffix_sort.c  implementation of simple 3-way quicksort of string, with final insertion sort for small buckets and initial counting sort for first symbol sort.
  2. src/prefix_doubling_sort.c  implementation of Sadakan's method for suffix array, with prefix doubling sort based on bucket index array. Bucket index is inversed when rocessing ISA upward (increase) or downward (decrease) to avoid unnecessary already sorted bucket processing 

## TODO:
  1. implements (through macro) selection between suffix array sort and Burrow-Wheeler (rotation) sort


## Author
 Nicolas Brunie, nibrunie@gmail.com
