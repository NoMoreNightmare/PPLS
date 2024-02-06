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


// Print a helpful message followed by the contents of an array
// Controlled by the value of SHOWDATA, which should be defined
// at compile time. Useful for debugging.
struct Param{
	int start;
	int end;
	int *data;
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
	struct Param param = *(struct Param *)arg;
	int *data = param.data;
	int start = param.start;
	int end = param.end;
	
	for(int i = start + 1; i < end; i++){
		data[i] = data[i] + data[i - 1];
	}
	
	
	
}


// YOU MUST WRITE THIS FUNCTION AND ANY ADDITIONAL FUNCTIONS YOU NEED
void parallelprefixsum (int *data, int n) {
	int numPerThreads = n / NTHREADS;
	pthread_t thread[NTHREADS];
	struct Param param[NTHREADS];
	for(int i = 0; i < NTHREADS - 1; i++){
		param[i].start = i * numPerThreads;
		param[i].end = param[i].start + numPerThreads;
		param[i].data = data;
		pthread_create(&thread[i], NULL, calculate, &param[i]);
	}
	
	
	
	param[NTHREADS - 1].start = (NTHREADS - 1) * numPerThreads;
	param[NTHREADS - 1].end = NITEMS;
	param[NTHREADS - 1].data = data;
	pthread_create(&thread[NTHREADS - 1], NULL, calculate, &param[NTHREADS - 1]);
	
	for(int i = 0; i < NTHREADS; i++){
		pthread_join(thread[i], NULL);
	}
	
	//use BARRIER to coordinate all the work thread
	
	
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
