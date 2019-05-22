#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
//////////////////////////
// constants
//////////////////////////
#define TRUE 1
#define FALSE 0
//////////////////////////
// type definition
//////////////////////////
typedef struct edge_s edge_t;
typedef struct node_s node_t;
typedef struct graph_s graph_t;
//////////////////////////
// graph structs
//////////////////////////
struct graph_s{ node_t *head;};
struct node_s{ node_t *next; int is_resource; void *id; edge_t *adj;};
struct edge_s{ node_t *dest; edge_t *link; };
//////////////////////////
// graph procedures 
//////////////////////////
graph_t *graph_init();
void graph_insert_node( graph_t* graph, void *id, int is_resource );
void graph_remove_node( graph_t* graph,void *id );
void graph_insert_edge( graph_t* graph, void *id_u, void *id_v);
void graph_remove_edge( graph_t* graph, void *id_u, void *id_v);
node_t *find( graph_t* graph, void *id );
void display( graph_t* graph );
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
		new_node->is_resource = is_resource;
		graph->head = new_node;
		return;
	}
	node_t *cursor = graph->head;
	while( cursor->next != NULL){
		if(cursor->id == id){
			printf("--> vertice ja existe no grafo\n");
			return;
		}
		cursor = cursor->next;
	}
	if( cursor->id == id ){
			printf("--> vertice ja existe no grafo\n");
			return;
	}
	// add node
	node_t* new_node = (node_t *) malloc(sizeof(node_t));
	new_node->id = id; new_node->next = NULL; new_node->adj = NULL;
	new_node->is_resource = is_resource;
	cursor->next = new_node;
}
void graph_remove_node(  graph_t* graph, void *id ){
	node_t *tmp;
	if( graph->head->id == id ){
		tmp = graph->head;
		graph->head = graph->head->next;// first element deleted 
		free(tmp);
		return;
	}
	node_t *cursor = graph->head;
	while( cursor->next->next != NULL ){
		if( cursor->next->id == id){// element deleted in between 
			tmp = cursor->next;
			cursor->next = tmp->next;
			free(tmp);
			return;
		}
		cursor = cursor->next;
	}
	if( cursor->next->id == id){// last element deleted 
		tmp = cursor->next;
		free(tmp);
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
	if(node_u->adj == NULL){ // first edge in adj list
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
	node_t *node_u, *node_v;
	edge_t *cursor, *tmp,*q;
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
	

	if( q->link->dest->id == id_v) {// last element deleted 
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
//////////////////////////
// test main
//////////////////////////
graph_t* RESOURCE_GRAPH;
int main(){
	RESOURCE_GRAPH = graph_init();
	int i, N;
	N = 3;
	sem_t resources[N];
	for(i = 0; i < N; i++)
		sem_init(&resources[i], 0, 1);
	






	graph_insert_node(RESOURCE_GRAPH, &resources[0], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[1], TRUE);
	graph_insert_node(RESOURCE_GRAPH, &resources[2], TRUE);
	graph_insert_edge(RESOURCE_GRAPH, &resources[0], &resources[1]);
	graph_insert_edge(RESOURCE_GRAPH, &resources[0], &resources[2]);
	graph_remove_edge(RESOURCE_GRAPH, &resources[0], &resources[2]);	

	display(RESOURCE_GRAPH);


	return 0;
}
