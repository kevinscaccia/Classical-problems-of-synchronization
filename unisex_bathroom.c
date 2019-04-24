/*************************************** 
 * Kevin Costa Scaccia RA: 120478
 ***************************************
 ***************************************/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define TRUE 1
#define FALSE 0
/*************************************** 
 * Semaforos e contadores Auxiliares
 ***************************************
 ***************************************/
sem_t sala, sala_mutex_x, sala_mutex_y;
// Auxiliares: 
int quantidade_x = 0;
int quantidade_y = 0;
#define TEMPO_SLEEP 1000000
/*************************************** 
 * Estrutura de um Usuario
 ***************************************
 ***************************************/
typedef struct usuario{
	char* nome;
	char grupo;
} Usuario;
/*************************************** 
 * Procedimento de que Atende Usuario
 ***************************************
 ***************************************/
void atender(Usuario *usr){ // faz alguma computacao
	//printf("Atendendo usuario %s, do grupo %c \n", usr->nome, usr->grupo);
	int i;
	int n = 10;
	for(i = 0; i < 100; i++)
		n = n*n + 10 - 7*n*100;
	//fflush(stdin);
	//usleep(10000);
}
/*************************************** 
 * Thread que Gerencia os Usuarios 
 ***************************************
 ***************************************/
void *cliente(void *args){
	Usuario *usr = (Usuario*) args;
	char grupo = usr->grupo;  
	while(TRUE){

		if(grupo == 'X'){  // Codigo grupo X
			sem_wait(&sala_mutex_x);
			quantidade_x++;
			if( quantidade_x == 1 ){  // caso ele seja o primeiro do grupo X, bloquear grupo Y
				sem_wait(&sala);
				printf("\033[0;31m Primeiro do grupo X \033[0m\n ");fflush(stdin);
				usleep(TEMPO_SLEEP);
			}
			sem_post(&sala_mutex_x);
			atender(usr);
			sem_wait(&sala_mutex_x);
			quantidade_x--;
			if(quantidade_x == 0){  // ultimo do grupo X, portanto, liberar o grupo Y
				sem_post(&sala);
				printf("\033[0;31m Ultimo do grupo X \033[0m \n ");
				fflush(stdin);	
			}
			sem_post(&sala_mutex_x);

		}else{  // Codigo grupo Y
			sem_wait(&sala_mutex_y);
			quantidade_y++;
			if( quantidade_y == 1 ){  // caso ele seja o primeiro do grupo Y, bloquear grupo X
				sem_wait(&sala);
				printf("\033[0;31m Primeiro do grupo Y \033[0m\n ");fflush(stdin);
				usleep(TEMPO_SLEEP);
			}
			sem_post(&sala_mutex_y);
			atender(usr);
			sem_wait(&sala_mutex_y);
			quantidade_y--;
			if(quantidade_y == 0){  // ultimo do grupo Y, portanto, liberar o grupo X
				sem_post(&sala);
				printf("\033[0;31m Ultimo do grupo Y \033[0m \n ");
				fflush(stdin);	
			}
			sem_post(&sala_mutex_y);
		}	
	}  // fim while

	pthread_exit(NULL);
}
/*************************************** 
 * Main
 ***************************************
 ***************************************/
int main(int argc, char** argv){
	sem_init(&sala_mutex_x, 0, 1);  // variavel count de x
	sem_init(&sala_mutex_y, 0, 1);  // variavel count de x
	sem_init(&sala, 0, 1);
	
	Usuario user0; user0.nome = "Kevin"; user0.grupo = 'X';
	Usuario user1; user1.nome = "Rafael"; user1.grupo = 'X';
	Usuario user2; user2.nome = "Pedro"; user2.grupo = 'Y';
	Usuario user3; user3.nome = "Carlos"; user3.grupo = 'Y';
	Usuario user4; user4.nome = "Robson"; user4.grupo = 'Y';
	
	pthread_t t_user0, t_user1, t_user2, t_user3, t_user4;

	pthread_create(&t_user0, NULL, cliente, &user0);
	pthread_create(&t_user2, NULL, cliente, &user2);
	pthread_create(&t_user4, NULL, cliente, &user4);
	pthread_create(&t_user1, NULL, cliente, &user1);
	pthread_create(&t_user3, NULL, cliente, &user3);

	pthread_join(t_user0, NULL);
	pthread_join(t_user1, NULL);
	return 0;
}
