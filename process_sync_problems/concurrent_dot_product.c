#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
//
long int VECTOR_SIZE;
long int NUM_CALCULATIONS;
double SCALAR_PRODUCT = 0;
double SCALAR_PRODUCT_SEQUENTIAL = 0, SCALAR_PRODUCT_CONCURRENT = 0;
double* VECTOR_A, *VECTOR_B, *REDUCTION_SUM;
//
int NUM_THREADS = 4;
//
double calculate_product_sequential(){
	long int i;
	double result = 0.0;
	for(i = 0; i < VECTOR_SIZE; i++)
		result += VECTOR_A[i]*VECTOR_B[i];
	return result;
}
//
void * worker( void* id){
	long int i, thread_id = (long int) id;
	double local_sum = 0.0;
	//printf("Worker %d starting..\n", thread_id);
	for(i = thread_id; i < VECTOR_SIZE; i += NUM_THREADS)
		local_sum += VECTOR_A[i]*VECTOR_B[i];
	//
	//printf("Worker %d finished..\n", thread_id);
	REDUCTION_SUM[thread_id] = local_sum;
}
//
double calculate_product_concurrent(){
	long int i; double result = 0.0;
	pthread_t threads[NUM_THREADS];
	REDUCTION_SUM = (double*) malloc( NUM_THREADS * sizeof(double)); // results of each thread
	for(i = 0; i < NUM_THREADS; i++) // throw threads
		pthread_create(&threads[i], NULL, worker, (void*) i);
	for(i = 0; i < NUM_THREADS; i++) // wait threads
		pthread_join(threads[i], NULL);
	// Reduction sum
	for(i=0; i < NUM_THREADS; i++)
		result += REDUCTION_SUM[i];
	return result;
}
int main(){
	// Alocate memory
	long int i; double result; int time_milis;
	struct timeval time_start, time_end;
	printf("Insert the vectors size:\n");
	scanf("%lld", &VECTOR_SIZE);
	VECTOR_A = (double*) malloc( VECTOR_SIZE * sizeof(double));
	VECTOR_B = (double*) malloc( VECTOR_SIZE * sizeof(double));
	// Generate random vectors
	for(i = 0; i < VECTOR_SIZE; i++){
		VECTOR_A[i] = rand() % 1000;
		VECTOR_B[i] = rand() % 1000;
	}
	printf("Vectors Memory Alocated: %ldbytes\n", VECTOR_SIZE * sizeof(double)*2);
	printf("Insert the number of Calculations:\n");
	scanf("%ld", &NUM_CALCULATIONS);
	printf("Starting....\n");
	// Run Sequential
	gettimeofday(&time_start, NULL);
	for(i=0; i < NUM_CALCULATIONS; i++)
		result = calculate_product_sequential();
	gettimeofday(&time_end, NULL); 
	time_milis = (int) (1000*(time_end.tv_sec - time_start.tv_sec) + (time_end.tv_usec - time_start.tv_usec)/1000);
	printf(" Scalar Product Sequential: %.1lf in %dms \n", result, time_milis);
	// Run Concurrent
	gettimeofday(&time_start, NULL);
	for(i=0; i < NUM_CALCULATIONS; i++)
		result = calculate_product_concurrent();
	gettimeofday(&time_end, NULL);
	time_milis = (int) (1000 *(time_end.tv_sec - time_start.tv_sec) + (time_end.tv_usec - time_start.tv_usec)/1000);
	printf(" Scalar Product Concurrent: %.1lf in %dms \n", result, time_milis);
	//
	return 0;
}
