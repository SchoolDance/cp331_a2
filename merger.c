#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

int * arrayer(int);
int cmpfunc (const void *, const void *);

int main (int argc, char** argv) {
  //SET THIS TO 1 IF YOU WANT TO SEE DEBUGGING INFO
  int debug = 1;
  int k1 = atoi(argv[1]);
  int k2 = atoi(argv[2]);
  srand((unsigned) time(NULL));


  int p;
  int rank;
  int nA = pow(2, k1);
  int nB = pow(2, k2);
  double k = log2(nA);
  MPI_Status status[2];

  int *A = arrayer(nA);
  int *B = arrayer(nB);
  // int A[16] = {1, 5, 15, 18, 19, 21, 23, 24, 27, 29, 30, 31, 32, 37, 42, 49};
  // int B[16] = {2, 3, 4, 13, 15, 19, 20, 22, 28, 29, 38, 41, 42, 43, 48, 50};
  
  //initialize MPI
  MPI_Init(&argc, &argv); // start mpi
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // process rank
  MPI_Comm_size(MPI_COMM_WORLD, &p); // find out the number of process

  //initialize A indicies
  int startIndexA = rank * k;
  int endIndexA = (rank + 1) * k;
  if (endIndexA >= nA)
    endIndexA = nA;
  int tempIndex = (rank + 2) * k;

  if (debug) printf("RANK %d -- A: %d - %d   nextA: %d\n\n", rank, startIndexA, endIndexA, tempIndex);

  //initialize B indicies
  int startIndexB = -1;
  int endIndexB = -1;

  //if rank 0, start at the beginning of B
  if (rank == 0) {
    startIndexB = 0;
    for (int i = 0; i < nB; i++) {
      if (B[i] > A[endIndexA - 1]) {
        endIndexB = i - 1;
        break;
      }
      
      //if all elements in B are less than the smallest in A
      if (endIndexB == -1) {
        endIndexB = nB - 1; 
      }
    }
  }
  //figure out where the start and end index of B
  else {
    for (int i = 0; i < nB; i++) {
      if (startIndexB == -1 && B[i] > A[startIndexA - 1]) {
        startIndexB = i;
      }

      if (B[i] > A[endIndexA - 1]) {
        endIndexB = i - 1;
        break;
      }
    }

    //if this is the last proc, set the end index of B to include the rest of the array
    if (startIndexB != -1 && rank + 1 == p) {
      endIndexB = nB - 1;
    }
  }

  if (debug) printf("RANK %d -- B: %d - %d\n\n", rank, startIndexB, endIndexB);

  int lengthOfA = endIndexA - startIndexA;
  //set length of B to 0 if it's empty (the +1 needs to be here, but messes up in this case)
  int lengthOfB = startIndexB == -1 ? 0 : endIndexB - startIndexB + 1;
  int newLength = lengthOfB + lengthOfA;
  int merged[newLength];

  if(debug) printf("RANK %d -- length A: %d    length B: %d\n\n", rank, lengthOfA, lengthOfB);

  /*// copy A into new array
  for(int i = 0; i < lengthOfA; i++) {
    merged[i] = A[startIndexA + i];
  }*/

  if (debug) {
    printf("RANK %d -- A = ", rank);
    for (int i = startIndexA; i < endIndexA; i++) {
      printf("%d,", A[i]);
    }
    printf("\n\n");
    printf("RANK %d -- B = ", rank);
    for (int i = 0; i < lengthOfB; i++) {
      printf("%d,", B[startIndexB + i]);
    }
    printf("\n\n");
  }

  int i = 0;
  int j = 0;
  int x = 0;
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
    for (int i = 0; i < newLength; i++) {
      printf("%d,", merged[i]);
    }
    printf("\n\n");
  }

  //BRING ALL THE RESULTS TOGETHER
  //if this is proc 0, listen for arrays from other procs
  if (rank == 0) {
    int finalArray[nA + nB];
    int offset = newLength;

    // copy merged into final array
    for(int i = 0; i < newLength; i++) {
      finalArray[i] = merged[i];
    }

    //get the array from each source and add it to the final array
    for (int source = 1; source < p; source++) {
      int length;

      //get the size of the array
      MPI_Recv (&length, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status[0]);

      //store the incoming array
      int tempArray[length];
      MPI_Recv (&tempArray, length, MPI_INT, source, 1, MPI_COMM_WORLD, &status[1]);

      //add this received array to the final array
      for(int i = 0; i < length; i++) {
        finalArray[offset + i] = tempArray[i];
      }

      //increment the offset for inserting into the final array
      offset += length;
    }

    //print the result of the final array
    printf("\nFINAL ARRAY -- ");
    for (int i = 0; i < nA + nB; i++) {
      printf("%d,", finalArray[i]);
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
   return (*(int*)a - *(int*)b);
}

int * arrayer (int size) {
  int *arr = malloc(sizeof(int) * size);

  for (int i = 0; i < size; i++) {
    arr[i] = rand() % 10000;
  }

  qsort(arr, size, sizeof(int), cmpfunc);
  return arr;
}
