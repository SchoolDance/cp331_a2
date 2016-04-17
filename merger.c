#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

long long * arrayer(long);
int cmpfunc (const void *, const void *);

int main (int argc, char** argv) {
  //SET THIS TO 1 IF YOU WANT TO SEE DEBUGGING INFO
  int debug = 0;
  long long k1 = atoi(argv[1]);
  long long k2 = atoi(argv[2]);
  srand((unsigned) time(NULL));

  long long i = 0;
  int p;
  int rank;
  long long nA = pow(2, k1);
  long long nB = pow(2, k2);
  long long k = log2(nA);
  MPI_Status status[2];

  long long *A = arrayer(nA);
  long long *B = arrayer(nB);

  //initialize MPI
  MPI_Init(&argc, &argv); // start mpi
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // process rank
  MPI_Comm_size(MPI_COMM_WORLD, &p); // find out the number of process

  //print A and B
  if (debug && rank == 0) {
    printf("**A = %d", rank);
    for (i = 0; i < nA; i++) {
      printf("%lld,", A[i]);
    }
    printf("\n\n");
    printf("**B = %d", rank);
    for (i = 0; i < nB; i++) {
      printf("%lld,", B[i]);
    }
    printf("\n\n");
  }

  //initialize A indicies
  long long startIndexA = rank * k;
  long long endIndexA = (rank + 1) * k;
  if (endIndexA >= nA) {
    endIndexA = nA;
  }
  long long tempIndex = (rank + 2) * k;

  if (debug) {
    printf("RANK %d -- A: %lld - %lld   nextA: %lld\n\n", rank, startIndexA, endIndexA, tempIndex);
  }

  //initialize B indicies
  long long startIndexB = 0;
  long long endIndexB = 0;

  //if rank 0, start at the beginning of B
  if (rank == 0) {
    startIndexB = 0;
    while (endIndexB < nB) {
      if (B[endIndexB] > A[endIndexA - 1]) {
        break;
      }
      endIndexB++;
    }
  }
  //figure out where the start and end index of B
  else {
    while (startIndexB < nB) {
      if (B[startIndexB] > A[startIndexA - 1]) {
        break;
      }
      startIndexB++;
    }

    while (endIndexB < nB) {
      if (B[endIndexB] > A[endIndexA - 1]) {
        break;
      }
      endIndexB++;
    }

    if (rank == p - 1) {
      endIndexB = nB;
    }
  }

  if (debug) {
    printf("RANK %d -- B: %lld - %lld\n\n", rank, startIndexB, endIndexB);
  }

  long long lengthOfA = endIndexA - startIndexA;
  //set length of B to 0 if it's empty (the +1 needs to be here, but messes up in this case)
  long long lengthOfB = endIndexB - startIndexB;
  long long newLength = lengthOfB + lengthOfA;
  long long merged[newLength];

  if (debug) {
    printf("RANK %d -- length A: %lld    length B: %lld\n\n", rank, lengthOfA, lengthOfB);
  }

  /*// copy A into new array
  for(int i = 0; i < lengthOfA; i++) {
    merged[i] = A[startIndexA + i];
  }*/

  if (debug) {
    printf("RANK %d -- A = ", rank);
    for (i = 0; i < lengthOfA; i++) {
      printf("%lld,", A[startIndexA + i]);
    }
    printf("\n\n");
    printf("RANK %d -- B = ", rank);
    for (i = 0; i < lengthOfB; i++) {
      printf("%lld,", B[startIndexB + i]);
    }
    printf("\n\n");
  }

  i = 0;
  long long j = 0;
  long long x = 0;
  while (i < lengthOfA && j < lengthOfB) {
    if (A[startIndexA + i] < B[startIndexB + j]) {
      merged[x] = A[startIndexA + i];
      i++;
    }
    else {
      merged[x] = B[startIndexB + j];
      j++;
    }
    x++;
  }
  if (i >= lengthOfA) {
    while (j < lengthOfB) {
      merged[x] = B[startIndexB + j];
      j++;
      x++;
    }
  }
  if (j >= lengthOfB) {
    while (i < lengthOfA) {
      merged[x] = A[startIndexA + i];
      i++;
      x++;
    }
  }


  /*//go through each element of the subset of B
  for(int i = 0; i < lengthOfB; i++) {
    int m_index = 0;

    //loop until you find the position where B[i] > merged[m_index]
    while (B[startIndexB + i] > merged[m_index] && m_index + 1 < newLength) {
      m_index++;
    }

    //shift everything from m_index over
    for(int a = newLength - 1; a >= m_index; a--)
    {
      merged[a] = merged[a - 1];
    }

    //insert B[i] at merged[m_index]
    merged[m_index] = B[startIndexB + i];
  }*/

  if (debug) {
    printf("RANK %d -- ", rank);
    for (i = 0; i < newLength; i++) {
      printf("%lld,", merged[i]);
    }
    printf("\n\n");
  }

  //BRING ALL THE RESULTS TOGETHER
  //if this is proc 0, listen for arrays from other procs
  if (rank == 0) {
    long long finalArray[nA + nB];
    long long offset = newLength;

    // copy merged into final array
    for(i = 0; i < newLength; i++) {
      finalArray[i] = merged[i];
    }

    //get the array from each source and add it to the final array
    int source;
    for (source = 1; source < p; source++) {
      long long length;

      //get the size of the array
      MPI_Recv (&length, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status[0]);

      //store the incoming array
      long long tempArray[length];
      MPI_Recv (&tempArray, length, MPI_INT, source, 1, MPI_COMM_WORLD, &status[1]);

      //add this received array to the final array
      for(i = 0; i < length; i++) {
        finalArray[offset + i] = tempArray[i];
      }

      //increment the offset for inserting into the final array
      offset += length;
    }

    //print the result of the final array
    printf("\nFINAL ARRAY -- ");
    for (i = 0; i < 4; i++) {
      printf("%lld,", finalArray[i]);
    }
    printf("\n\n");
  }
  else {
    //send size of array
    MPI_Send(&newLength, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    //send array
    MPI_Send(&merged, newLength, MPI_INT, 0, 1, MPI_COMM_WORLD);
  }

  //cleanup
  free(A);
  free(B);
  MPI_Finalize();
}

int cmpfunc (const void * a, const void * b) {
   return (*(long long*)a - *(long long*)b);
}

long long * arrayer (long size) {
  long long *arr = malloc(sizeof(long long) * size);
  long long i;

  for (i = 0; i < size; i++) {
    arr[i] = rand() % 2147483646;
  }

  qsort(arr, size, sizeof(long long), cmpfunc);
  return arr;
}
