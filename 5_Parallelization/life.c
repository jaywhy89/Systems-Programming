
/*****************************************************************************

Following Game of Life will run with parallel version upto 10000 X 10000.
If size of the world is less than or equal to 32 X 32, it will run sequential
version of GameOfLife.  
If size of the world is greater than max size (e.g.,10000), it will return
error statment and exit gracefully (returning original inboard)

Optimized version of Game of Life implements the following changes.

1.  Multithreading with pthreads (8 threads)

2.  Loop unrolling by 4 times

3.  Reuse of blocks during neighbour cell calculation

4.  Optimization of helper functions (mod, alivep)

5.  Code Motion

6.  Simple arithmatcic optimizations by hand

7.  Optimization level set to -O3

< Performance Summary:  110.86 ---> 5.35 seconds >

-Our team had observed the major performance gain from modifications 1 to 3

-4 to 7 provided minor improvements, but since we are dealing with large array
10000x10000, small performance gains were still valuable.

There are 2 functions we've added.
1.  parallel_game_of_life (life.c - line 90)
2.  BOARDLICM (reaplcing original BOARD function - line 58)

/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

/** Swapping the two boards only involves swapping pointers, not copying values.  */
#define SWAP_BOARDS( b1, b2 )  do { \
 char* temp = b1; \
 b1 = b2; \
 b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])
#define BOARDLICM( __board, __i, __jLDA )  (__board[(__i) + __jLDA])



typedef struct thread_input{
	int tid;
	pthread_barrier_t* barrier;
	char* inboard;
	char* outboard;
	int nrows;
	int ncols;
	int gens_max;
	int start_index;
	int end_index;
}T_input;


/*****************************************************************************
Parallel_game_of_life is multi-threaded version of sequential_game_of_life.

> inorth, isouth, jwest, jeast from sequential version are replaced with up, down, left, right variables.
> if cell is located on the first/last column or row, they return the oppsite end index.  Otherwise, it does simple increment (++1) or (--1) from current position.

> Additional 4 variables are initialized (upscore, uprightscore, downscore, downrightscore) which will be re-used in next time step.
> Diagrams illustrating how we reuse neighbour cells are shown on report.pdf

> Loop interchange decreased the performance, theyrefore we left it as was (i-j sequence, traversing row-wise)

> As a synchronization primitive, Pthread_barrier is placed at the end of each generation step.
*****************************************************************************/

void*
parallel_game_of_life (void* input)
{
	T_input* i_arg= (T_input*) input;

	char *inboard = i_arg->inboard;
	char *outboard = i_arg->outboard;
	const int nrows = i_arg->nrows;
	const int ncols = i_arg->ncols;
	const int start_index = i_arg->start_index;
	const int end_index = i_arg->end_index;
	const int gens_max = i_arg->gens_max;

    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
	const int LDA = nrows;


	int curgen, i, j;
	

	for (curgen = 0; curgen < gens_max; curgen++)
	{
		for (i = start_index; i < end_index; ++i)
		{

			int up = i? i-1 : nrows-1;
			int down = i == (nrows-1)? 0 : i+1;
			int upLicm = up * LDA;
			int downLicm = down * LDA;
			int iLicm = i *LDA;
			register int upscore, uprightscore, downscore, downrightscore, stateij;

			for (j = 0; j < ncols; j+=4)
			{
				int right = j ==( ncols-1 )? 0 : j+1;
				int left = j? j-1: ncols -1;

				upscore = BOARDLICM (inboard, j, upLicm);
				uprightscore = BOARDLICM (inboard, right, upLicm);
				downscore = BOARDLICM (inboard, j, downLicm);
				downrightscore = BOARDLICM (inboard, right, downLicm);

				const char neighbor_count = 
				BOARDLICM (inboard, left, upLicm)+
				upscore+
				uprightscore+
				BOARDLICM (inboard, left, iLicm)+
				BOARDLICM (inboard, right, iLicm)+
				BOARDLICM (inboard, left, downLicm)+
				downscore+
				downrightscore;

				stateij =BOARDLICM(inboard, j, iLicm);
				BOARDLICM(outboard, j, iLicm) = alivep (neighbor_count, stateij);

				int right2 = j+2;

				char neighbor_count2 = 
				upscore+
				uprightscore; 

				//update to this iteration
				upscore = uprightscore;
				uprightscore = BOARDLICM (inboard, right2, upLicm);
				neighbor_count2+=uprightscore +
				stateij+
				BOARDLICM (inboard, right2, iLicm)+
				downscore+
				downrightscore;

				//update to this iteration
				downscore = downrightscore;
				downrightscore = BOARDLICM (inboard, right2, downLicm);
				neighbor_count2 += downrightscore;

				stateij = BOARDLICM(inboard, j+1, iLicm);
				BOARDLICM(outboard, j+1, iLicm) = alivep (neighbor_count2, stateij);

				int right3 = j+3;

				char neighbor_count3 = 
				upscore+
				uprightscore; 

				//update to this iteration
				upscore = uprightscore;
				uprightscore = BOARDLICM (inboard, right3, upLicm);
				neighbor_count3+=uprightscore +
				stateij+
				BOARDLICM (inboard, right3, iLicm)+
				downscore+
				downrightscore;

				//update to this iteration
				downscore = downrightscore;
				downrightscore = BOARDLICM (inboard, right3, downLicm);
				neighbor_count3 += downrightscore;

				stateij =  BOARDLICM(inboard, j+2, iLicm);
				BOARDLICM(outboard, j+2, iLicm) = alivep (neighbor_count3, stateij);

				int right4 = j+3 ==( ncols-1 )? 0 : j+4;

				char neighbor_count4 = 
				upscore+
				uprightscore; 

				//update to this iteration
				upscore = uprightscore;
				uprightscore = BOARDLICM (inboard, right4, upLicm);
				neighbor_count4+=uprightscore +
				stateij+
				BOARDLICM (inboard, right4, iLicm)+
				downscore+
				downrightscore;

				//update to this iteration
				downscore = downrightscore;
				downrightscore = BOARDLICM (inboard, right4, downLicm);
				neighbor_count4 += downrightscore;

				stateij = BOARDLICM(inboard, j+3, iLicm);
				BOARDLICM(outboard, j+3, iLicm) = alivep (neighbor_count4, stateij);

			}
		}

		pthread_barrier_wait(i_arg->barrier);
		SWAP_BOARDS( outboard, inboard );

	}
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
     pthread_exit(NULL);
 }

/*****************************************************************************
  Game of life

If board size >= 32*32, program will run with sequential_game_of_life
If board size > 10000, program will run with parallel_game_of_life (8 threads)
Otherwise, program will exit gractfully with error message.

UG machine has 4 cores in architecture, but due to hyperthreading feature
8 threads haave shown better results.  Each thread will work on 1/8th row the board. 
> 4 threads: 8.65 seconds
> 8 threads: 5.45 seconds

Pthread join waits all threads to exit before we return final inboard.

  ****************************************************************************/
 char*
 game_of_life (char* outboard, 
 	char* inboard,
 	const int nrows,
 	const int ncols,
 	const int gens_max)
 {

 	if (nrows <= 32)
 		return sequential_game_of_life (outboard, inboard, nrows, ncols, gens_max);

 	if (nrows > 10000)
 	{
 		printf("ERROR: Too large size attempted.\n");
 		return inboard;
 	}

 	int num_threads=8;
 	pthread_t threads[num_threads];
 	pthread_barrier_t barrier;
 	pthread_barrier_init(&barrier, NULL, num_threads);


 	T_input inputs[num_threads];
 	int i;

 	for (i=0; i<num_threads; i++)
 	{
 		inputs[i].tid = i;
 		inputs[i].nrows = nrows;
 		inputs[i].ncols = ncols;
 		inputs[i].gens_max = gens_max;
 		inputs[i].inboard = inboard;
 		inputs[i].outboard = outboard;
 		inputs[i].start_index = (nrows/num_threads)*i;
 		inputs[i].end_index = inputs[i].start_index+(nrows/num_threads);
 		inputs[i].barrier = &barrier;
 	}

 	for (i=0; i<num_threads; i++)
 	{
 		pthread_create(&threads[i],NULL, parallel_game_of_life, (void*) &inputs[i]);
 	}

 	for (i=0; i<num_threads; i++)
 	{
 		pthread_join(threads[i], NULL);
 	}

 	return inboard;

 }
