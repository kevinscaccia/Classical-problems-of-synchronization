
/* Copyright (C) Kevin Costa Scaccia - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Kevin Scaccia <kevin_developer@outlook.com>, March 2019
 */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
//////////////////////////
// Constants and Global variables
//////////////////////////
#define TRUE 1
#define FALSE 0
sem_t RESOURCE_GRAPH_MUTEX;
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
// Procedures Implementation
//////////////////////////
graph_t* graph_init(){
	sem_init(&RESOURCE_GRAPH_MUTEX, 0, 1);
	graph_t* g = (graph_t*) malloc(sizeof(graph_t));
	g->head = NULL;
	return g;
}
void graph_insert_node(  graph_t* graph, void *id, int is_resource ){
	sem_wait(&RESOURCE_GRAPH_MUTEX);
	if( graph->head==NULL ){ // empty graph
		node_t* new_node = (node_t *) malloc(sizeof(node_t));
		new_node->id = id; new_node->next = NULL; new_node->adj = NULL;
		new_node->is_resource = is_resource; new_node->visited = FALSE; 
		graph->head = new_node; new_node->recStack = FALSE;
		graph->v++;
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	node_t *cursor = graph->head;
	while( cursor->next != NULL){
		if(cursor->id == id){
			printf("--> vertice ja existe no grafo\n");
			sem_post(&RESOURCE_GRAPH_MUTEX);
			return;
		}
		cursor = cursor->next;
	}
	if( cursor->id == id ){
			printf("--> vertice ja existe no grafo\n");
			sem_post(&RESOURCE_GRAPH_MUTEX);
			return;
	}
	// add node
	node_t* new_node = (node_t *) malloc(sizeof(node_t));
	new_node->id = id; new_node->next = NULL; new_node->adj = NULL;
	new_node->is_resource = is_resource; new_node->visited = FALSE;
	cursor->next = new_node; new_node->recStack = FALSE;
	graph->v++;
	sem_post(&RESOURCE_GRAPH_MUTEX);
}
void graph_remove_node(  graph_t* graph, void *id ){
	sem_wait(&RESOURCE_GRAPH_MUTEX);
	node_t *tmp;
	if( graph->head->id == id ){
		tmp = graph->head;
		graph->head = graph->head->next;// first element deleted 
		free(tmp);
		graph->v--;
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	node_t *cursor = graph->head;
	while( cursor->next->next != NULL ){
		if( cursor->next->id == id){// element deleted in between 
			tmp = cursor->next;
			cursor->next = tmp->next;
			free(tmp);
			graph->v--;
			sem_post(&RESOURCE_GRAPH_MUTEX);
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
	sem_post(&RESOURCE_GRAPH_MUTEX);
}
void graph_insert_edge(  graph_t* graph, void *id_u, void *id_v){
	sem_wait(&RESOURCE_GRAPH_MUTEX);
	node_t *node_u, *node_v;	
	node_v = find(graph, id_v);
	node_u = find(graph, id_u);

	if(node_u == NULL || node_v == NULL){
		printf("--> algum dos vertices nao existente\n");
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	if(node_u->adj == NULL){ // first edge in adj list
		edge_t *new_edge = (edge_t*) malloc(sizeof(edge_t));
		new_edge->dest = node_v;  // next node
		new_edge->link = NULL;
		node_u->adj = new_edge;
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	edge_t *cursor;
	cursor = node_u->adj;
	
	while( cursor->link != NULL ){
		if( cursor->dest->id == node_v->id ){
			printf("--> aresta ja existe no grafo\n");
			sem_post(&RESOURCE_GRAPH_MUTEX);
			return;
		}
		cursor = cursor->link;
	}
	if( cursor->dest->id == node_v->id ){
			printf("--> aresta ja existe no grafo\n");
			sem_post(&RESOURCE_GRAPH_MUTEX);
			return;
	}
	edge_t *new_edge = (edge_t*) malloc(sizeof(edge_t));
	new_edge->dest = node_v;  // next node
	new_edge->link = NULL;
	cursor->link = new_edge;
	sem_post(&RESOURCE_GRAPH_MUTEX);
}
void graph_remove_edge(  graph_t* graph, void *id_u, void *id_v){
	sem_wait(&RESOURCE_GRAPH_MUTEX);
	node_t *node_u, *node_v;
	edge_t *cursor, *tmp,*q;
	node_u = find(graph, id_u);

	if( node_u == NULL ){
		printf("vertice u nao presente\n");
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	if( node_u->adj == NULL)  // sem nodos adjacentes
		return;
	if( node_u->adj->dest->id == id_v){// first element 
		tmp = node_u->adj;
		node_u->adj = node_u->adj->link;
		free(tmp);
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	q = node_u->adj;
	while( q->link->link != NULL ){
		if( q->link->dest->id == id_v){// element deleted in between
			tmp = q->link;
			q->link = tmp->link;
			free(tmp);
			sem_post(&RESOURCE_GRAPH_MUTEX);
			return;
		}
		q = q->link;
	}
	if( q->link->dest->id == id_v) {// last element deleted 
		tmp = q->link;
		free(tmp);
		q->link = NULL;
		sem_post(&RESOURCE_GRAPH_MUTEX);
		return;
	}
	printf("edge nao presente no grafo\n");
	sem_post(&RESOURCE_GRAPH_MUTEX);
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
	sem_wait(&RESOURCE_GRAPH_MUTEX);
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
	 sem_post(&RESOURCE_GRAPH_MUTEX);
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
	sem_wait(&RESOURCE_GRAPH_MUTEX);
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
	sem_post(&RESOURCE_GRAPH_MUTEX);
	return result;
	/*
	int i = 0;
	cursor = graph->head; 
    for(i = 0; i < graph->v; i++){
        if( graph_has_cicle_rec(graph, cursor) )
        return TRUE;      
		cursor = cursor->next;
    }
    sem_post(&RESOURCE_GRAPH_MUTEX);
    return FALSE; 
    */

}

//////////////////////////
// test main
//////////////////////////
graph_t* RESOURCE_GRAPH;
int main(){
	RESOURCE_GRAPH = graph_init();
	int i, N;
	N = 10;
	sem_t resources[N];
	for(i = 0; i < N; i++)
		sem_init(&resources[i], 0, 1);

	graph_insert_node(RESOURCE_GRAPH, &resources[0], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[1], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[2], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[3], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[4], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[5], TRUE);


	graph_insert_edge(RESOURCE_GRAPH, &resources[1], &resources[2]);
	graph_insert_edge(RESOURCE_GRAPH, &resources[2], &resources[3]);
	graph_insert_edge(RESOURCE_GRAPH, &resources[3], &resources[0]);
	graph_insert_edge(RESOURCE_GRAPH, &resources[3], &resources[5]);
	graph_insert_edge(RESOURCE_GRAPH, &resources[5], &resources[1]);
	graph_insert_edge(RESOURCE_GRAPH, &resources[5], &resources[4]);	
	graph_insert_edge(RESOURCE_GRAPH, &resources[4], &resources[0]);
	display(RESOURCE_GRAPH);
	int has = graph_has_cicle(RESOURCE_GRAPH, &resources[5]);
	if(has)
		printf("\nCOM ciclo\n");
	else
		printf("\nSEM ciclo\n");
	
	return 0;
}
