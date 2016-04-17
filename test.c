#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int * tacos (int);

int main (int argc, char **argv) {
  int k = atoi(argv[1]);
  int *A = tacos(pow(2, k));
  int *B = tacos(pow(2, k));
  printf("%d\t%d", A[1], B[1]);

  // for (int j = 0; j < pow(2, k); j++) {
  //   printf("%d\t%d", A[j], B[j]);
  // }

  free(A);
  return(0);
}

int cmpfunc (const void * a, const void * b) {
   return (*(int*)a - *(int*)b);
}

int * tacos (int size) {
  int *arr = malloc(sizeof(int) * size);
  // srand((unsigned) time(NULL));
  time_t t;
  // srand((unsigned) time(&t));

  for (int i = 0; i < size; i++) {
    arr[i] = random();
  }

  qsort(arr, size, sizeof(int), cmpfunc);
  return arr;
}
