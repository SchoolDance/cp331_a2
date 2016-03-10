#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

int main (int argc, char** argv) {
  int A[16] = {1, 5, 15, 18, 19, 21, 23, 24, 27, 29, 30, 31, 32, 37, 42, 49};
  int B[16] = {2, 3, 4, 13, 15, 19, 20, 22, 28, 29, 38, 41, 42, 43, 48, 50};
  
  int p;
  int rank;
  int nA = sizeof(A);
  int nB = sizeof(B);
  double k = log2(nA);
  
  //initialize MPI
  MPI_Init(&argc, &argv); // start mpi
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // process rank
  MPI_Comm_size(MPI_COMM_WORLD, &p); // find out the number of process
  
  //initialize A indicies
  int startIndexA = rank * k;
  int endIndexA = (rank + 1) * k;
  int tempIndex = (rank + 2) * k;
  
  //initialize B indicies
  int startIndexB = -1;
  int endIndexB = -1;
  if (rank == 0) {
    startIndexB = 0; 
    for (int i = 0; i < nB; i++) {
      if (B[i] > A[endIndexA - 1]) {
        endIndexB = B[i-1] - 1;
      }
    }
  }
  else {
    for (int i = 0; i < nB; i++) {
      if (startIndexB == -1 && B[i] > A[endIndexA - 1]) {
        startIndexB = B[i-1];
      }
      
      if (B[i] > A[tempIndex - 1]) {
        endIndexB = B[i-1] - 1;
        break;
      }
    }
  }
  
  // array A, B
  
  int len_B = endIndexB - startIndexB;
  int len_A = endIndexA - startIndexA;
  
  int len_new = len_B + len_A + 1;
    
  int merged[len_new]; 
  
  // copy A into new array
  
  for(int i = 0; i < len_A; i++) {    
    merged[i] = A[startIndexA + i];
  }
  
  // Do sort 
  int m_index = 0;
  
  
  int non_empty = len_A;
  while(non_empty < len_new){
  
    for(int i = 0; i < len_B; i++){
      
      if(B[i] > merged[m_index]){
        // move pointer of new array over
        m_index++;  
        
      } else if(B[i] < merged[m_index]){
        // position found, shift then insert
        for(int n = m_index; n < (len_new - m_index); n++){
          merged[n + 1] = merged[n];
          non_empty++;
        }
        // Replace with new B[i] value
        merged[m_index] = B[i];
        non_empty++;
        
      } else {
        // Probably reached end of array 
        merged[m_index] = B[i];
      }
      
      for (int i = 0; i < sizeof(merged); i++) {
        printf("%d \t",merged[i]);  
      }
      printf("\n");
    }
    MPI_Finalize();
  }

  
 
    
    
  
  
    
    
  
  
}