/*
 * LU : Block Decomposition with MTE and PE data Transfer ...
 * block/block.c
 *
 * This program parallelizes a LU algorithm using Block Decomposition of the Matrix
 * to get its LU.
 *
 * see ReadMe and the report for full description.
 *
 * Date   9th November 2001
 *
 * Copyright (c)  Manish M Kochhal
 */

#include <ssi/inspector.h>
#include <clasm/quadinfo.h>
#include <ssi/ssi.h>
#include <ssi/ssiconst.h>
#include <ssi/semaphore.h>
#include <clasm/mtclib.h>
#include <stdio.h>
#include <stdlib.h>

//#define USING_MTE            // comment out to use PE instead

#define SIZE 128
#define PES 4
#define ROWS SIZE/PES	// Number of Rows to each process

#define LOCAL_MAT_ROWS  (SIZE/PES)           // Local matrix's number of Rows ...                              
#define LOCAL_SIZE  LOCAL_MAT_ROWS*SIZE
#define BYTES_PER_ELEMENT  4          // Float takes 4 bytes of memory ...
#define MOVE_TO_LOCAL_MEMORY  0
#define MOVE_TO_SDRAM  1


char app[] _SD = "BLOCK";

float matA[SIZE][SIZE] _SD; // Original Matrix ...
float matB[SIZE][SIZE] _SD; // Used during checking final results of this program ...

float local_matA[LOCAL_MAT_ROWS][SIZE] _SL;
float pivot_row [SIZE] _SL;
float pivot_col [SIZE] _SL;

int done_pe _SD=0;			//count of PES that have done their task
static semaphore_info_t CheckSem _SD;	// global semaphore

// MTC structure... 

SF_MtcBlock MTC_ParmBlock _PL;
SF_MtcBlock *ptr_MTC_ParmBlock _PL;

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
	  
     k=0;
      
	 for(i=0; i<SIZE; i++,k++)
	 for(j=0; j<SIZE; j++)
	  {
		  matB[i][j]=testMatrix[j+k];
	  }
     
}
    
/*
 * barrier(int k, int my_peid)
 * 
 * The Barrier Function implements a barrier for synchronizing all 
 * the intermediate results of one PE for use by other PEs.  It 
 * uses the global semaphore to check how many PEs have entered the barrier. 
 * The last PE to enter the barrier then Frees all other waiting Processes 
 * including itself ...
*/
void barrier(int k, int my_peid){
	int toOpenBarrier = -1;
	int tempDone;
 
	semaphore_lock(CheckSem.p);
	
	tempDone = done_pe;
	tempDone++;
	if(tempDone == PES)
	  toOpenBarrier = my_peid;
    else
	 done_pe++;
	
	semaphore_unlock(CheckSem.p);
	
	while (done_pe < PES)
	{
		 if(toOpenBarrier != -1)
		  {
			semaphore_lock(CheckSem.p);
			done_pe++;
			semaphore_unlock(CheckSem.p);
	      }	  
		_pe_delay(1);
	} 

	if(toOpenBarrier != -1)
	{
		semaphore_lock(CheckSem.p);
		done_pe=0;
		//printf("\n PE %d has reinitialized done_pe to 0\n",my_peid);
		semaphore_unlock(CheckSem.p);
	}
   	 	 
 }// end of barrier..

 /*
 * move_data_using_mte()
 *
 * shows how to move data between the SDRAM and the local memory using the
 * MTE. This function is used to move a specified block of data of matA into
 * the local memory. The calls used in this function are from 'mtclib.h'
 * Here the movement of data is done in a very simple manner.
 * The PE initiates the movement of data and then waits for completion.
 * Basically the PE is just polling until the MTE finishes moving the data.
 * The MTE is used to move the block of data.
 * The count parameter specifies which block we need, and the
 * second parameter specifies the direction that the data is moving in.
 */
 
void move_data_using_mte(int count, int direction)
{
  int marker;
  marker = count*LOCAL_MAT_ROWS;

  /*
   * Set all the MTC Parameter block fields
   */
  MTC_ParmBlock.Options = BYTES_PER_ELEMENT*LOCAL_SIZE; //num of bytes to xfer
  if( direction == 0 )  // if direction = 0. Move data into the local memory
  {
     MTC_ParmBlock.Destination = (int)&local_matA[0];
     MTC_ParmBlock.Source      = (int)&matA[marker][0];
  }
  else
  {
     if(direction == 1) // If direction = 1. Move data into the SDRAM
     {
        MTC_ParmBlock.Destination = (int)&matA[marker][0];
        MTC_ParmBlock.Source      = (int)&local_matA[0][0];
     }
     else
     {
        return;
     }
  }

  /*
   * Set pointer to MTC Parameter block for MTE library functions
   */
  ptr_MTC_ParmBlock = &MTC_ParmBlock;

  /*
   * MTE_Start() from mtclib.h passes to the MTE, a pointer to a structure
   * that holds the parameter block and a command. In this case the command
   * is MOVE_CMD, which means move data
   */
  MTE_Start(ptr_MTC_ParmBlock, MOVE_CMD);

  /*
   * MTE_wait() simply waits for the MTE to complete the transfer.
   */
  MTE_wait(ptr_MTC_ParmBlock);
  return;
}

/*
 * move_data_using_pe()
 *
 * This function moves data from the SDRAM to the Local Memory using the PE.
 * This function below does the same thing as move_data_using_mte.
 * Only it uses the PE to move data.
 * The PE moves each individual element one at a time.
 * We can use either function,
 * but using the MTE is faster and more efficient for large chunks of data.
 */
void move_data_using_pe(int count, int direction)
{
  int marker,i,j;

  /*
   * marker tells you which element do you have to start at for moving the
   * matA from SDRAM to LOCAL MEMORY
   */
  marker = count*LOCAL_MAT_ROWS;

  if(direction == 0)
    for(i=0; i<LOCAL_MAT_ROWS; i++)
      for(j=0; j<SIZE; j++)
        local_matA[i][j] = matA[marker+i][j];

  /*
   * Here marker is used to determine which element is the starting point
   * for moving matC from LOCAL MEMORY to the SDRAM.
   */
  if(direction == 1)
    for(i=0; i<LOCAL_MAT_ROWS; i++)
      for(j=0; j<SIZE; j++)
        matA[marker+i][j] = local_matA[i][j];
}

/*
 * moveDataToLocalMemory(int blockCount)
 *
 * This Function moves specified chunk/block of matrix 
 * from SDRAM (i.e. matA) to Local Memory (i.e local_matA)
 *
*/

void moveDataToLocalMemory(int blockCount)
 {
	 #ifdef USING_MTE		
    	move_data_using_mte(blockCount,MOVE_TO_LOCAL_MEMORY);
     #else 
	    move_data_using_pe(blockCount,MOVE_TO_LOCAL_MEMORY);
    #endif 
 } 

 /*
 * moveDataToSDRAM(int blockCount)
 *
 * This Function moves specified partial/intermediate results (i.e. local_matA)  
 * from Local Memory to corresponding part of matrix in SDRAM (i.e matA)
 *
*/

void moveDataToSDRAM(int blockCount)
 {
  #ifdef USING_MTE		
	move_data_using_mte(blockCount,MOVE_TO_SDRAM);
  #else 
	move_data_using_pe(blockCount,MOVE_TO_SDRAM);
  #endif
 }

 /*
 * initializePivotRow()
 *
 * This Function initializes the pivot row which is shared by all PEs 
 * and is in local memory in one row matrix i.e. pivot_row
 *
*/

void initializePivotRow()
 {
	int l = 0;
	for(l=0;l<SIZE;l++){
	  pivot_row[l]=0;		  
	}
}
	
/*
 * pivotalRowCalculations(int pivotRow)
 *
 * This Function calculates the pivot row given the "k" index value.
 * Here "k" is the index that moves diagonally from (0,0) to (SIZE-1, SIZE-1).
 * 
 * The Pivot Row so obtained is needed by all PEs and hence is 
 * stored in shared local memory in one row matrix i.e. pivot_row
*/

void pivotalRowCalculations(int pivotRow)
 {
	int scaledPivotRow = pivotRow%LOCAL_MAT_ROWS; 
	int j; // Column Wise Index ...
	
  for(j=pivotRow+1; j<SIZE; j++)
   {		
	 local_matA[scaledPivotRow][j]=local_matA[scaledPivotRow][j]/local_matA[scaledPivotRow][pivotRow];
  	 pivot_row [j] = local_matA[scaledPivotRow][j];// Store this Pivotal Row
   }	
 }

/*
 * pivotalBlockSubmatrixCalculations(int pivotRow)
 *
 * This Function calculates the submatrix of a block whose pivot row is in the same block
 * Here Block Count is not needed as it is always the same index as my_peid
*/

void pivotalBlockSubmatrixCalculations(int pivotRow)
 {
	 int scaledPivotRow = pivotRow%LOCAL_MAT_ROWS; 
     int i = 0; // Row Wise Index 
     int j; // Column Wise Index ...
	
	 for(i=(scaledPivotRow+1) ; i<LOCAL_MAT_ROWS;i++)
	   {		
	     for(j=pivotRow+1;j<SIZE;j++) // all the entries in the row..
	      {
			local_matA[i][j]=local_matA[i][j]-local_matA[i][pivotRow]*pivot_row[j];
		  }
	   }
 }

/*
 * pivotalBlockCalculations(int pivotRow, int blockCount)
 *
 * This Function calculates combines both functions pivotalRowCalculations() 
 * and pivotalBlockSubmatrixCalculations() in addition to other necessary functions ...
 *
*/

void pivotalBlockCalculations(int pivotRow, int blockCount)
 {
	initializePivotRow();
	pivotalRowCalculations(pivotRow);
    pivotalBlockSubmatrixCalculations(pivotRow);
	moveDataToSDRAM(blockCount); 
 }

/*
 * pivotalBlockCalculations(int pivotRow, int blockCount)
 *
 * This Function calculates the submatrix of a block whose pivot row is NOT in the same block
 * 
*/

void nonPivotalBlockSubmatrixCalculations(int pivotRow)
 {
    int i = 0; // Row Wise Index 
    int j; // Column Wise Index ...
     
	 for(i=0 ; i<LOCAL_MAT_ROWS;i++) // the columns 
      {		
	    for(j=pivotRow+1;j<SIZE;j++) // all the entries in the row..
	      {	
		   	local_matA[i][j]=local_matA[i][j]-local_matA[i][pivotRow]*pivot_row[j];
	  	   }
	   }
  }
/*
 * nonpivotalBlockCalculations(int pivotRow, int blockCount)
 *
 * This Function calculates combines function nonpivotalBlockSubmatrixCalculations()
 * in addition to other necessary functions ...
 *
*/ 
void nonpivotalBlockCalculations(int pivotRow, int blockCount)
 {
   moveDataToLocalMemory(blockCount);
   nonPivotalBlockSubmatrixCalculations(pivotRow);
   moveDataToSDRAM(blockCount);
 }

/*
 * calculate()
 *
 * This function does the matrix calculation.  The outer For Loop moves in a 
 * diagonal fashion, which we call basically the pivotRow and the Block
 * Counts are accordingly depending on this Scaled pivotRow and the my_peid of a PE ...
 * Appropriate Functions are then called to perform the pivotal and non-pivotal Block 
 * calculations ...
 *
 */

void calculate()
{
  int k=0; // Row Wise Index, Column Wise Index and Diagonal Wise Index
  
  int my_quadid=_QUAD_INDEX; //QUAD Index 
  int my_peid=(my_quadid*4)+_PE_INDEX;	// Original PE ID 
  
  int isPivotalBlock = 0;
  int blockCount = 0;
     
  for(k=0;k<SIZE;k++)
   {
   	  isPivotalBlock = 1;	// First Row of Every Block is a Pivotal Row ...
   	  	
	  for (blockCount= k/LOCAL_MAT_ROWS; blockCount < (SIZE/LOCAL_MAT_ROWS); blockCount++) 
	    {
		  if(my_peid==blockCount)
		    {		
		  	   moveDataToLocalMemory(blockCount); // Transfer Sub-Block from SDRAM to Local Mem ...
			   if(isPivotalBlock == 1) 
	 	  	      pivotalBlockCalculations(k, blockCount);
			   else 
			      nonpivotalBlockCalculations(k, blockCount);
			 } // end of if(my_peid==blockCount)
		    
		   isPivotalBlock = 0;	// Re-initialize isPivotalBlock for next Idle PE
		   
		   barrier (k, my_peid); // Synchronize Results for Other Idle PE's
		   
	    }// end of for blockCount 
	    
   }// end of for loop : k 
   
}// End of calculate()

/*
 * luSequentialResults()
 *
 * This function is run sequentially by only PE 0 for final checking of results ...
 *
 */
 
void luSequentialResults(){
 int i=0, j=0, k=0;
  
 for(k=0;k<SIZE;k++)
  {
	for(j=k+1; j<SIZE; j++)
	  matB[k][j]=matB[k][j]/matB[k][k];
	 
	for(i=k+1;i<SIZE;i++)
	   for(j=k+1;j<SIZE;j++)
	     matB[i][j]=matB[i][j]-matB[i][k]*matB[k][j];
   }
}// End of luSequentialResults()

/*
 * compareLuSequentialAndParallelResults()
 *
 * This function is run sequentially by only PE 0 for final comparison 
 * of sequential and parallel results ...
 *
 */

int compareLuSequentialAndParallelResults()
{
	int i=0;
	int j=0;
	int check = 0;
	
	luSequentialResults();
	
	for(i=0;i<SIZE;i++)
	 for(j=0;j<SIZE;j++)	
	    check += (matA[i][j]-matB[i][j]);
	
	printf("\n Final Result check %d\n",check);
    return check;
			
}// end of funtion compareLuSequentialAndParallelResults()

/*
 * This function will perform an internal check to ensure that the
 * final matrix is correct.  It calls compareLuSequentialAndParallelResults()
 * which in turn calls luSequentialResults()
 *
 */

void check()
{
  int i,j;
  FILE *fp;
  fp = fopen("lu.dat", "w");
  
  if(compareLuSequentialAndParallelResults() != 0)
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

void printComparison()
{
	int i=0;
	int j=0;
	for(i=0;i<SIZE;i++){
		printf("\n");
		for(j=0;j<SIZE;j++){	
			printf(" %4.2f , ",matA[i][j]-matB[i][j]);
		}
	}
}

/*
 * start_other_pes()
 *
 * This function is used by PE0 to start other PEs
 * There are many ways in which this can be optimised
 * However we take the simple approach of PE0 going and
 * starting PEs sequentially, one after another.
 * Note that this function is called after PE0 has initialized
 * the matrices. This is because PE1 will start its computations
 * before PE0 has started all the PEs.
 */
void start_other_pes()
{
  int i=0;
  char *AppName;

  AppName = &app[0];
  for(i=1; i<PES; i++)
  {
    /*
     * This function is defined in "start_app.h"
     */
    start_app_on_pe(i,AppName);
  }
}


/*
 * Main function. Will call the other functions.
 */
void main()
{
  int x;
  int my_peid = (_QUAD_INDEX * 4) + _PE_INDEX;
  /*
   * Only PE0 enters to allocate semaphor 
   */
  printf("\n Inside main function PE =: %d ",my_peid);
   
  if(my_peid == 0)
   {
     if (semaphore_global_alloc(&CheckSem) < 0)
	   {
		  printf("ERROR: Global Semaphore allocation failed, exiting.\n");
		  exit(1);
       }
	
      initialize();
      start_other_pes();
   }
   
   barrier(-1, my_peid);
   calculate();
  
  if(my_peid == 0)
   {
     check();
    // printComparison();
	 printf("\nGood Bye :-) ");
   }
   barrier (-1, my_peid);

   stop_pe();

}
