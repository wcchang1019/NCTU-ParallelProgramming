#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#ifndef W
#define W 20                                    // Width
#endif
int main(int argc, char **argv) {
  int rank, size;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Status status;
  int L = atoi(argv[1]);                        // Length
  int iteration = atoi(argv[2]);                // Iteration
  srand(atoi(argv[3]));                         // Seed
  float d = (float) random() / RAND_MAX * 0.2;  // Diffusivity
  int *temp = malloc(L*W*sizeof(int));          // Current temperature
  int *next = malloc(L*W*sizeof(int));          // Next time step
  for (int i = 0; i < L; i++) {
    for (int j = 0; j < W; j++) {
      temp[i*W+j] = random()>>3;
    }
  }
  int count = 0, balance = 0, local_len_begin=L*rank/size, local_len_end=L*(rank+1)/size, total;
  while (iteration--) {     // Compute with up, left, right, down points
    balance = 0;
    count++;
    for (int i = local_len_begin; i < local_len_end; i++) {
      for (int j = 0; j < W; j++) {
        float t = temp[i*W+j] / d;
        t += temp[i*W+j] * -4;
        t += temp[(i - 1 <  0 ? 0 : i - 1) * W + j];
        t += temp[(i + 1 >= L ? i : i + 1)*W+j];
        t += temp[i*W+(j - 1 <  0 ? 0 : j - 1)];
        t += temp[i*W+(j + 1 >= W ? j : j + 1)];
        t *= d;
        next[i*W+j] = t ;
        if (next[i*W+j] != temp[i*W+j]) {
          balance = 1;
        }
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank > 0){
      MPI_Send(&next[(local_len_begin*W)], W, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
    }
    if(rank < size-1){
      MPI_Recv(&next[(local_len_end*W)], W, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status);
    }
    if(rank < size-1){
      MPI_Send(&next[((local_len_end-1)*W)], W, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
    }
    if(rank > 0){
      MPI_Recv(&next[((local_len_begin-1)*W)], W, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Allreduce(&balance, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (!total) {
      break;
    }
    int *tmp = temp;
    temp = next;
    next = tmp;
  }
  int min = 2147483647;
  for (int i = local_len_begin; i < local_len_end; i++) {
    for (int j = 0; j < W; j++) {
      if (temp[i*W+j] < min) {
        min = temp[i*W+j];
      }
    }
  }
  int ans;
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Allreduce(&min, &ans, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
  if(rank == 0){
    printf("Size: %d*%d, Iteration: %d, Min Temp: %d\n", L, W, count, ans); 
  }
  MPI_Finalize();
  return 0;
}