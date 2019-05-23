#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0

void use_resource(char* resource){
	printf("%d using resource <%s>\n", (int)pthread_self(), resource);
	fflush(stdout);
}
//////////////////////////
// test threads
//////////////////////////
sem_t sem_A, sem_B;
sem_t resource_A, resource_B, resource_C;
void *thread_A(void *args){
	while(TRUE){
		sem_wait(&sem_B);// request resource B
		sem_wait(&sem_A);// request resource A
		// use both resources
		use_resource("A");
		use_resource("B");
		// free both resources
		sem_post(&sem_A);// free resource A
		sem_post(&sem_B);// free resource B
		printf("------------- OK ");fflush(stdout);
	}
	pthread_exit(NULL);
}
void *thread_B(void *args){
	while(TRUE){
		sem_wait(&sem_A);// request resource A
		sem_wait(&sem_B);// request resource B
		// use both resources
		use_resource("A");
		use_resource("B");
		// free both resources
		sem_post(&sem_A);// free resource A
		sem_post(&sem_B);// free resource B
		printf("------------- OK ");fflush(stdout);
	}
	pthread_exit(NULL);
}
//////////////////////////
// test main
//////////////////////////
int main(){
	sem_init(&sem_A, 0, 1);
	sem_init(&sem_B,0, 1);
	pthread_t t_A, t_B;

	pthread_create(&t_A, NULL, thread_A, NULL);
	pthread_create(&t_B, NULL, thread_B, NULL);
	pthread_join(t_A, NULL);
	pthread_join(t_B, NULL);
	return 0;
}

