/**********************************************************************
 * DESCRIPTION:
 *   Serial Concurrent Wave Equation - C Version
 *   This program implements the concurrent wave equation
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265
#define MAXTHREADSIZE 500

void check_param(void);
void init_line(void);
void update (void);
void printfinal (void);

int nsteps,                   /* number of time steps */
    tpoints,            /* total points along string */
    rcode;                    /* generic return code */
float  values[MAXPOINTS+2];   /* values at time t */

float *cuda_values;
/**********************************************************************
 * Checks input values from parameters
 *********************************************************************/
void check_param(void)
{
   char tchar[20];

   /* check number of points, number of iterations */
   while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
      printf("Enter number of points along vibrating string [%d-%d]: "
           ,MINPOINTS, MAXPOINTS);
      scanf("%s", tchar);
      tpoints = atoi(tchar);
      if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
         printf("Invalid. Please enter value between %d and %d\n", 
                 MINPOINTS, MAXPOINTS);
   }
   while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
      printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
      scanf("%s", tchar);
      nsteps = atoi(tchar);
      if ((nsteps < 1) || (nsteps > MAXSTEPS))
         printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
   }

   printf("Using points = %d, steps = %d\n", tpoints, nsteps);

}

/**********************************************************************
 *     Initialize points on line
 *********************************************************************/
void init_line(void)
{
   int j;
   float x, fac, k, tmp;

   /* Calculate initial values based on sine curve */
   fac = 2.0 * PI;
   k = 0.0; 
   tmp = tpoints - 1;
   for (j = 1; j <= tpoints; j++) {
      x = k/tmp;
      values[j] = sin (fac * x);
      k = k + 1.0;
   } 
}

/**********************************************************************
 *      Calculate new values using wave equation
 *********************************************************************/
__device__ float do_math(float now_value, float old_value)
{
   float dtime, c, dx, tau, sqtau;

   dtime = 0.3;
   c = 1.0;
   dx = 1.0;
   tau = (c * dtime / dx);
   sqtau = tau * tau;
   return (2.0 * now_value) - old_value + (sqtau *  (-2.0)*now_value);
}

/**********************************************************************
 *     Update all values along line a specified number of times
 *********************************************************************/
__global__ void update(float *cuda_values, int nsteps, int tpoints)
{
   int i, j;
   j = blockIdx.x*MAXTHREADSIZE + threadIdx.x + 1;

   /* Update values for each time step */
   if(j <= tpoints){
      float now_value = cuda_values[j];
      float old_value = now_value;
      float new_value;
      for (i = 1; i<= nsteps; i++) {
         if ((j == 1) || (j  == tpoints))
            new_value = 0.0;
         else{
            new_value = do_math(now_value, old_value);
         }
         old_value = now_value;
         now_value = new_value;
      }
      cuda_values[j] = now_value;
   }
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal()
{
   int i;

   for (i = 1; i <= tpoints; i++) {
      printf("%6.4f ", values[i]);
      if (i%10 == 0)
         printf("\n");
   }
}

/**********************************************************************
 * Main program
 *********************************************************************/
int main(int argc, char *argv[])
{
   sscanf(argv[1],"%d",&tpoints);
   sscanf(argv[2],"%d",&nsteps);
   check_param();
   printf("Initializing points on the line...\n");
   init_line();
   cudaMalloc(&cuda_values, sizeof(float)*(MAXPOINTS+2));
   cudaMemcpy(cuda_values, values, sizeof(float)*(MAXPOINTS+2), cudaMemcpyHostToDevice);
   printf("Updating all points for all time steps...\n");
   int block_size = ceil((float)tpoints/MAXTHREADSIZE);
   update<<<block_size, MAXTHREADSIZE>>>(cuda_values, nsteps, tpoints);
   cudaMemcpy(values, cuda_values, sizeof(float)*(MAXPOINTS+2), cudaMemcpyDeviceToHost);
   printf("Printing final results...\n");
   printfinal();
   printf("\nDone.\n\n");
   cudaFree(cuda_values);
   return 0;
}