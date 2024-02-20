// PPLS Exercise 1 Starter File
//
// See the exercise sheet for details
//
// Note that NITEMS, NTHREADS and SHOWDATA should
// be defined at compile time with -D options to gcc.
// They are the array length to use, number of threads to use
// and whether or not to printout array contents (which is
// useful for debugging, but not a good idea for large arrays).

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> // in case you want semaphores, but you don't really
                       // need them for this exercise

void barrier_init(); //barrier initializing
void barrier();


// Print a helpful message followed by the contents of an array
// Controlled by the value of SHOWDATA, which should be defined
// at compile time. Useful for debugging.


/*
	The struct that stored the neccessary parameters required by the parallel calculation
 */
struct Param{
	int start;   // the start of local chunk
	int end; 		 // the end of local chunk
	int *data;   // the array
	int id;			 // the id of the current thread
};  



void showdata (char *message,  int *data,  int n) {
  int i; 

  if (SHOWDATA) {
    printf ("%s", message);
    for (i=0; i<n; i++ ){
     printf (" %d", data[i]);
    }
    printf("\n");
  }
}

// Check that the contents of two integer arrays of the same length are equal
// and return a C-style boolean
int checkresult (int* correctresult,  int *data,  int n) {
  int i; 

  for (i=0; i<n; i++ ){
    if (data[i] != correctresult[i]) return 0;
  }
  return 1;
}

// Compute the prefix sum of an array **in place** sequentially
void sequentialprefixsum (int *data, int n) {
  int i;

  for (i=1; i<n; i++ ) {
    data[i] = data[i] + data[i-1];
  }
}

void *calculate(void *arg){
	//get the parameters from the struct
	struct Param param = *(struct Param *)arg;
	int *data = param.data;
	int start = param.start;
	int end = param.end;
	int id = param.id;
	// printf("%d\n", id);

	//except for the start position, other position should perform a prefix sum sequentially
	for(int i = start + 1; i < end; i++){
		data[i] = data[i] + data[i - 1];
	}


	//use barrier() to make sure all the threads has finished the first phase: local chunk prefix sum
	
	//if barrier is not used, when updating the highest indexed position at next phase, the value 
	//at these positions may be wrong since some threads haven't finished the local chunk prefix sum
	barrier();

	// printf("NITEMS: %d\n", NITEMS);
	// printf("NTHREADS: %d\n", NTHREADS);

	//only worker thread 0 can work 
	if(id == 0){
		int len = NITEMS / NTHREADS;

		//performs a sequential prefix sum using just the highest indexed position from each chunk in place
		for(int i = 1; i < NTHREADS - 1; i++){ 
			data[(i + 1) * len - 1] = data[i * len - 1] + data[(i + 1) * len - 1];
		}

		data[NITEMS - 1] = data[NITEMS - 1] + data[(NTHREADS - 1) * len - 1]; //the last position is slightly different because the size of that chunk can vary

	}


	//user barrier() to make sure the prefix sum using highest indexed position from each chunk has finished
	
	//if the previous phase has not finished, the value that next phase use to update the local chunk may be not the correct prefix sum at the previous highest indexed position 
	barrier();

	//other work threads begin to update local chunks using the previous highest indexed position
	if(id != 0){
		for(int i = start; i < end - 1; i++){
			data[i] = data[i] + data[start - 1]; //update the local chunk except for worker 0
		}
	}

	
}


// YOU MUST WRITE THIS FUNCTION AND ANY ADDITIONAL FUNCTIONS YOU NEED
void parallelprefixsum (int *data, int n) {

	int numPerThreads = n / NTHREADS;

	pthread_t thread[NTHREADS];
	struct Param param[NTHREADS];  
	barrier_init();
	for(int i = 0; i < NTHREADS - 1; i++){
		//store parameters at the struct
		//calculate the start, end position of the local chunk for the thread i
		param[i].start = i * numPerThreads;
		param[i].end = param[i].start + numPerThreads;
		param[i].data = data;
		param[i].id = i; //store the thread id and pass it to the thread method
		pthread_create(&thread[i], NULL, calculate, &param[i]);
	}
	
	
	//the last local chunk is slightly different due to the size
	param[NTHREADS - 1].start = (NTHREADS - 1) * numPerThreads;
	param[NTHREADS - 1].end = NITEMS;
	param[NTHREADS - 1].data = data;
	param[NTHREADS - 1].id = NTHREADS - 1;
	pthread_create(&thread[NTHREADS - 1], NULL, calculate, &param[NTHREADS - 1]);
	
	//join all the worker threads
	for(int i = 0; i < NTHREADS; i++){
		pthread_join(thread[i], NULL);
	}
		
	
}





int main (int argc, char* argv[]) {

  int *arr1, *arr2, i;

  // Check that the compile time constants are sensible
  if ((NITEMS>10000000) || (NTHREADS>32)) {
    printf ("So much data or so many threads may not be a good idea! .... exiting\n");
    exit(EXIT_FAILURE);
  }

  // Create two copies of some random data
  arr1 = (int *) malloc(NITEMS*sizeof(int));
  arr2 = (int *) malloc(NITEMS*sizeof(int));
  srand((int)time(NULL));
  for (i=0; i<NITEMS; i++) {
     arr1[i] = arr2[i] = rand()%5;
  }
  showdata ("initial data          : ", arr1, NITEMS);

  // Calculate prefix sum sequentially, to check against later on
  sequentialprefixsum (arr1, NITEMS);
  showdata ("sequential prefix sum : ", arr1, NITEMS);

  // Calculate prefix sum in parallel on the other copy of the original data
  parallelprefixsum (arr2, NITEMS);
  showdata ("parallel prefix sum   : ", arr2, NITEMS);

  // Check that the sequential and parallel results match
  if (checkresult(arr1, arr2, NITEMS))  {
    printf("Well done, the sequential and parallel prefix sum arrays match.\n");
  } else {
    printf("Error: The sequential and parallel prefix sum arrays don't match.\n");
  }

  free(arr1); free(arr2);
  return 0;
}



 /*
 	The code below is borrowed from the example code in the Learn, which has implemented the functionality of the barrier
  */
struct BarrierData {
  pthread_mutex_t barrier_mutex;
  pthread_cond_t barrier_cond;
  int nthread; // Number of threads that have reached this round of the barrier
  int round;   // Barrier round id
} bstate;

void barrier_init() {
  pthread_mutex_init(&bstate.barrier_mutex, NULL);
  pthread_cond_init(&bstate.barrier_cond, NULL);
  bstate.nthread = 0;
}

void barrier() {
    pthread_mutex_lock(&bstate.barrier_mutex);
    bstate.nthread++;
    if(bstate.nthread == NTHREADS) {
        bstate.round++;
        bstate.nthread = 0;
        pthread_cond_broadcast(&bstate.barrier_cond);
    } else {
        int lround = bstate.round;
        do {
            pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
        } while(lround == bstate.round);
    }
    pthread_mutex_unlock(&bstate.barrier_mutex);
}