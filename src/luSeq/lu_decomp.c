/*
 * Assignment Part 1/lu_decomp.c
 *
 * This program does the LU decomposition in SDRAM/LocalMemory
 * For the case of matrix in Local Memory comment out "USING_SDRAM"
 *
 * see LU.README for full description
 *
 * Copyright (c) 2001 Manish M Kochhal (P000422865)
 */

#include <ssi/inspector.h>
#include <clasm/quadinfo.h>
#include <ssi/ssi.h>
#include <ssi/ssiconst.h>
#include <stdio.h>


#define SIZE 128

/*
 * matA is of SIZE*SIZE matrix
 * When this code is loaded onto 1PE, it will use
 * 1PE to calculate the L and U matrix of matA and
 * store the answer in the same matrix i.e matA 
 */

#define USING_SDRAM            // comment out to use local memory instead

#ifdef USING_SDRAM		
  float matA[SIZE][SIZE] _SD;     
#else 
  float matA[SIZE][SIZE] _SL;     
#endif 

float testMatrix [] _SD= // Random Test Matrix ...
{
4,2,23,2,8,4,23,5,3, 17,2,8,47,5,31,8,4,8,4,
9,6,4,12,6,2,7,3,7,9,7, 4,8,4,8,4,9,6,4,12,6,
8,4,8,4,9,6,4,12,6,2, 7,3,7,2,8,47,5,31,7,4,9,
4,8,4,8,4,9,23,5,3,17, 2,8,47,5,31,7,4,9,4,8,
5,3,17,2,8,47,5,31,7,8, 7,3,7,9,4,11,8,4,8,9,
4,2,23,2,8,4,23,5,3, 17,2,8,47,5,31,8,4,8,4,
9,6,4,12,6,2,7,3,7,9,7, 31,7,4,9,4,8,4,8,4,9,
6,4,12,6,2,7,3,7,9,4, 11,8,4,8,3,17,2,8,47,5,
31,7,4,9,4,8,4,8,47,5, 31,7,4,9,4,8,4,8,4,9,
16,8,2,8,4,23,5,3,17,2, 8,47,5,31,7,4,9,4,8,4,
4,9,6,4,12,6,2,7,3,7, 9,4,11,8,4,8,9,4,2,23,
2,8,47,5,31,7,4,9,4,8, 4,8,4,9,4,8,4,9,6,4,
12,6,2,7,3,7,9,4,8,4,8, 4,9,6,4,12,6,2,7,7,9,
4,11,8,4,8,9,4,2,23,9, 4,8,4,8,4,9,6,4,12,6,
2,7,3,8,4,8,4,9,6,4, 12,6,2,7,3,7,9,7,8,5,
2,8,47,5,31,7,4,9,4,8, 4,2,7,3,7,9,4,11,8,4,
6,4,12,6,2,7,3,7,9,4, 8,47,5,31,7,4,9,4,8,4,
4,11,8,4,8,9,4,2,23,9, 17,2,8,47,5,31,8,4,8,4,
4,2,23,2,8,4,23,5,3, 5,3,17,2,8,47,5,31,7,8,
4,2,7,3,7,9,4,11,8,4, 8,9,4,2,2,7,2,8,4,23,
5,3,17,2,8,47,5,31,9,4, 4,8,4,8,4,9,6,4,12,6,
2,7,3,4,23,5,3,17,2,8, 47,5,31,7,4,9,4,8,4,23
};

/* initialize()
 *
 * This function shall initialize the matrix whose LU is needed...
 * This is a SIZE*SIZE square matrix
 * The LU is updated in the original matrix itslef i.e. matA
 * In order to perform tests, we also maintain a back-up of the original matrix
 * and then get its LU using LU sequential algorithm which runs only on one PE ...
 * 
 * Both these Final Matrices are then checked to verify the correctness of this
 * Program ...
 * 
 */
void initialize()
{
	int i,j,k=0;
	
	printf("\n Test Data matrix dimension is  %d",SIZE);
	
	for(i=0; i<SIZE; i++,k++)
	 for(j=0; j<SIZE; j++)
	  {
		  matA[i][j]=testMatrix[j+k];
      }
	     
}
/*
 * calculate()
 *
 * This function does the matrix calculation.  The outer For Loop moves in a 
 * diagonal fashion, which we call basically the pivotRow and the rest are 
 * row-wise and column-wise indices ...
 *
 */

void calculate()
{
  int i=0;
  int j=0;
  int k=0;
  printf("\ninside the calculate function");

  for(k=0;k<SIZE;k++){
	    
		for(j=k+1; j<SIZE; j++){	//scale pivot row
			matA[k][j]=matA[k][j]/matA[k][k];
		}
		for(i=k+1;i<SIZE;i++){		// the columns 
			for(j=k+1;j<SIZE;j++){	// all the entries in the row..
				matA[i][j]=matA[i][j]-matA[i][k]*matA[k][j];
			}
		}
  }

}

void printLU()
{
	int i=0;
	int j=0;
	for(i=0;i<SIZE;i++){
		printf("\n");
		for(j=0;j<SIZE;j++){	
			printf(" %4.2f , ",matA[i][j]);
		}
	}
}
 
/*void check()
{
  int i,j;
  FILE *fp;
  fp = fopen("lu.dat", "w");
  
  if( != 0)
      {
        printf(" !!!! Error in LU Calculations !!! ");
        fprintf(fp,"1");
        fclose(fp);
        exit();
      }
 
  printf(" LU is correct as expected.\n");
  fprintf(fp,"There is no error");
  fclose(fp);
}
*/

/*
 * Main function. Will call the other functions.
 */
void main()
{
  initialize();
  calculate();
//  check();
 // printLU();
  printf("\nGood Bye :-) ");
}
