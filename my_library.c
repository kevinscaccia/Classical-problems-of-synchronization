/* Copyright (C) Kevin Costa Scaccia - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Kevin Scaccia <kevin_developer@outlook.com>, March 2019
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
//////////////////////////
// Type definition
//////////////////////////
typedef struct edge_s edge_t;
typedef struct node_s node_t;
typedef struct graph_s graph_t;
//////////////////////////
// Graph Structs
//////////////////////////
struct graph_s{ node_t *head; int v; };
struct node_s{ node_t *next; int is_resource; void *id; edge_t *adj; int visited; int recStack; };
struct edge_s{ node_t *dest; edge_t *link; };
//////////////////////////
// Constants and Global variables
//////////////////////////
#define TRUE 1
#define FALSE 0
sem_t RESOURCE_GRAPH_MUTEX;
int RESOURCE_GRAPH_INIT = FALSE;
graph_t *RESOURCE_GRAPH=NULL;
//////////////////////////
// Graph Procedures Prototype 
//////////////////////////
graph_t *graph_init();
void graph_insert_node( graph_t* graph, void *id, int is_resource );
void graph_remove_node( graph_t* graph,void *id );
void graph_insert_edge( graph_t* graph, void *id_u, void *id_v);
void graph_remove_edge( graph_t* graph, void *id_u, void *id_v);
node_t *find( graph_t* graph, void *id );
void display( graph_t* graph );
int graph_has_cicle_rec( graph_t *graph, node_t *current);
int graph_has_cicle( graph_t* graph, void *id_root );
//////////////////////////
//My semaphore implementation
//////////////////////////
int (*_sem_init)(sem_t *, int, unsigned int) = NULL;
int (*_sem_wait)(sem_t *) = NULL;
int (*_sem_post)(sem_t *) = NULL;
//
void lock(){
	_sem_wait(&RESOURCE_GRAPH_MUTEX);
}
void unlock(){
	_sem_post(&RESOURCE_GRAPH_MUTEX);
}
//
int sem_init(sem_t *sem, int pshared, unsigned int value){
	if (!_sem_init) {
		_sem_init = dlsym(RTLD_NEXT, "sem_init");
		_sem_wait = dlsym(RTLD_NEXT, "sem_wait");
		_sem_post = dlsym(RTLD_NEXT, "sem_post");
		//printf("Originais inicializados\n");fflush(stdout);
	}
	if(!RESOURCE_GRAPH_INIT){
		RESOURCE_GRAPH_INIT = TRUE;
		_sem_init(&RESOURCE_GRAPH_MUTEX, 0, 1);
		lock();
		RESOURCE_GRAPH = graph_init();
		unlock();
	}
	lock();	
	graph_insert_node(RESOURCE_GRAPH, sem, TRUE ); // insert resource
	unlock();
	return _sem_init(sem, pshared, value);
}
int k = 0;
int sem_wait(sem_t *sem) {
	printf("k: %d\n",k++);
	int teste;
	int r;
	sem_getvalue(sem, &teste);
	pthread_t tid = pthread_self();
	lock();
	graph_insert_node(RESOURCE_GRAPH, &tid, FALSE );
	unlock();
	if(teste <= 0){
		lock();
		graph_insert_edge(RESOURCE_GRAPH, &tid, sem);//Arco que aponta do processo para o recurso 
		if(graph_has_cicle(RESOURCE_GRAPH,sem)){
			graph_remove_edge(RESOURCE_GRAPH, &tid, sem);
			printf("!!!!!!!!!!!!! DEADLOCK !!!!!!!!!!!!!");fflush(stdout);
			unlock();
			exit(-1);
			return -1;
		}	
		printf("--> %d: SEM_WAIT(%d)\n", ((int) tid) % 100, ((int) &sem ) %100 );fflush(stdout);	
		unlock();
		r = _sem_wait(sem);
		printf("pass2\n");fflush(stdout);
		lock();
		graph_remove_edge(RESOURCE_GRAPH, &tid, sem);
		graph_insert_edge(RESOURCE_GRAPH, sem, &tid); //Apos o recurso ser alocado, arco aponta do recurso para o processo
		unlock();
		return r;
	}else{
		lock();
		printf("--> %d: SEM_WAIT(%d)\n", ((int) tid) % 100, ((int) &sem ) %100 );fflush(stdout);	
		unlock();
		r = _sem_wait(sem);
		lock();
		printf("pass1\n");fflush(stdout);	
		graph_insert_edge(RESOURCE_GRAPH, sem, &tid); //Apos o recurso ser alocado, arco aponta do recurso para o processo
		unlock();
		return r;
	}
}

int sem_post(sem_t *sem) {
	int r;
	pthread_t tid;
	tid = pthread_self();
	lock();
	graph_remove_edge(RESOURCE_GRAPH, sem, &tid); //Remover arco que aponta do recurso para o processo
	printf("--> %d: SEM_POST(%d)\n", ((int) tid) % 100, ((int) &sem) % 100);fflush(stdout);	
	unlock();
	r = _sem_post(sem);
	return(r);
}
//////////////////////////
// Graph Procedures Implementation
//////////////////////////
graph_t* graph_init(){
	graph_t* g = (graph_t*) malloc(sizeof(graph_t));
	g->head = NULL;
	return g;
}
void graph_insert_node(  graph_t* graph, void *id, int is_resource ){
	if( graph->head==NULL ){ // empty graph
		node_t* new_node = (node_t *) malloc(sizeof(node_t));
		new_node->id = id; new_node->next = NULL; new_node->adj = NULL;
		new_node->is_resource = is_resource; new_node->visited = FALSE; 
		graph->head = new_node; new_node->recStack = FALSE;
		graph->v++;
		return;
	}
	node_t *cursor = graph->head;
	while( cursor->next != NULL){
		if(cursor->id == id){
			//printf("--> vertice ja existe no grafo\n");fflush(stdout);
			return;
		}
		cursor = cursor->next;
	}
	if( cursor->id == id ){
			//printf("--> vertice ja existe no grafo\n");fflush(stdout);
			return;
	}
	// add node
	node_t* new_node = (node_t *) malloc(sizeof(node_t));
	new_node->id = id; new_node->next = NULL; new_node->adj = NULL;
	new_node->is_resource = is_resource; new_node->visited = FALSE;
	cursor->next = new_node; new_node->recStack = FALSE;
	graph->v++;
}
void graph_remove_node(  graph_t* graph, void *id ){
	node_t *tmp;
	if( graph->head->id == id ){
		tmp = graph->head;
		graph->head = graph->head->next;// first element deleted 
		free(tmp);
		graph->v--;		return;
	}
	node_t *cursor = graph->head;
	while( cursor->next->next != NULL ){
		if( cursor->next->id == id){// element deleted in between 
			tmp = cursor->next;
			cursor->next = tmp->next;
			free(tmp);
			graph->v--;
			return;
		}
		cursor = cursor->next;
	}
	if( cursor->next->id == id){// last element deleted 
		tmp = cursor->next;
		free(tmp);
		graph->v--;
		cursor->next = NULL;
	}
}
void graph_insert_edge(  graph_t* graph, void *id_u, void *id_v){
	node_t *node_u, *node_v;	
	node_v = find(graph, id_v);
	node_u = find(graph, id_u);

	if(node_u == NULL || node_v == NULL){
		printf("--> algum dos vertices nao existente\n");
		return;
	}
	if(node_u->adj == NULL){ // first edge in adj list
		edge_t *new_edge = (edge_t*) malloc(sizeof(edge_t));
		new_edge->dest = node_v;  // next node
		new_edge->link = NULL;
		node_u->adj = new_edge;
		return;
	}
	edge_t *cursor;
	cursor = node_u->adj;
	
	while( cursor->link != NULL ){
		if( cursor->dest->id == node_v->id ){
			printf("--> aresta ja existe no grafo\n");
			return;
		}
		cursor = cursor->link;
	}
	if( cursor->dest->id == node_v->id ){
			printf("--> aresta ja existe no grafo\n");
			return;
	}
	edge_t *new_edge = (edge_t*) malloc(sizeof(edge_t));
	new_edge->dest = node_v;  // next node
	new_edge->link = NULL;
	cursor->link = new_edge;
}
void graph_remove_edge(  graph_t* graph, void *id_u, void *id_v){
	node_t *node_u;
	edge_t *tmp,*q;
	node_u = find(graph, id_u);

	if( node_u == NULL ){
		printf("vertice u nao presente\n");
		return;
	}
	if( node_u->adj == NULL)  // sem nodos adjacentes
		return;
	if( node_u->adj->dest->id == id_v){// first element 
		tmp = node_u->adj;
		node_u->adj = node_u->adj->link;
		free(tmp);
		return;
	}
	q = node_u->adj;
	while( q->link->link != NULL ){
		if( q->link->dest->id == id_v){// element deleted in between
			tmp = q->link;
			q->link = tmp->link;
			free(tmp);
			return;
		}
		q = q->link;
	}
	if( q->link->dest->id == id_v) {// last element deleted 
		tmp = q->link;
		free(tmp);
		q->link = NULL;
		return;
	}
	printf("edge nao presente no grafo\n");
}
node_t *find(  graph_t* graph, void *id ){
	node_t *cursor, *ret;
	cursor = graph->head;
	while( cursor != NULL ){
		if( cursor->id == id){// found
			ret = cursor;
			return ret;
		}
		cursor = cursor->next;
	}
	return NULL;
}
void display( graph_t* graph){
	node_t *cursor;
	edge_t *cursor_edge;

	cursor = graph->head;
	while( cursor != NULL ){
		printf("%p ->", cursor->id );
		cursor_edge = cursor->adj;
		while(cursor_edge != NULL ){
			printf(" %p ->", cursor_edge->dest->id);
			cursor_edge = cursor_edge->link;
		}
		printf(" / \n");	
		cursor = cursor->next;
	 }
}
int graph_has_cicle_rec( graph_t *graph, node_t *current) { 
        if (current->recStack == TRUE) 
            return TRUE; 
        if (current->visited == TRUE) 
            return FALSE; 
        current->visited = TRUE;
        current->recStack = TRUE;
        //
        edge_t *cursor;
		cursor = current->adj;
		while( cursor != NULL ){
			if (graph_has_cicle_rec(graph, cursor->dest)) 
                return TRUE; 
			cursor = cursor->link;
		}
        current->recStack = FALSE; 
        return FALSE; 	
} 
int graph_has_cicle( graph_t* graph, void *id_root ){
	node_t *cursor = graph->head;
	while( cursor != NULL){// reset nodes values
		cursor->visited = FALSE;
		cursor->recStack = FALSE;
		cursor = cursor->next;
	}
	int result;
	cursor = find(graph, id_root);// find root node
	if(cursor == NULL)
		result = FALSE;
	else
		result = graph_has_cicle_rec(graph, cursor);
	//
	return result;
}
