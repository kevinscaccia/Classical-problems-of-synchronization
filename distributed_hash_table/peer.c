#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#define TRUE 1
#define FALSE 0
//
// Constants
//
int NUM_PEERS = 4;  // Number of peers in the DHT
int PEER_ID_LOCAL = 1;  // Local peer ID
int PEER_ID_PREV = 0;
int LOCAL_PORT = 10001;  // Local port (bind to receive requests)
int MAX_STORAGE = 10;  // Max items that this pair can store
int STORAGE_COUNT = 0;  // Current count of itens stored
int REQUEST_SIZE_GET = 32;
int REQUEST_SIZE_PUT = 64;
int REQUEST_SIZE_HAS = 32;
int RESPONSE_SIZE_BUFFER = 64; 
///
// Neighbors Peers Information 
struct sockaddr_in peer_next_addr, peer_prev_addr;
int PEER_NEXT_ID = 2, PEER_NEXT_PORT = 10001;
int PEER_PREV_ID = 0, PEER_PREV_PORT = 10000;
//
// Types 
//
typedef int key_t;
typedef int value_t;
// Item Structure (key, value)
typedef struct item_s{
	key_t key;
	value_t value;
} item_t;
// Request Structure
typedef struct request_s{
	char method[3];
	key_t key;
	value_t value;
} request_t;
//
// Global Variables - 172.21.209.175
//
item_t *LOCAL_STORAGE; // Local (key,value) storage
//
// Local HASH Procedures
//
// Hash function 
int get_peer_by_key(key_t key){
	return key % NUM_PEERS;
}
// HAS key
int storage_has_key(key_t key){
	int i;
	for(i=0; i < STORAGE_COUNT; i++){// Percorre LOCAL_STORAGE procurando pela chave key
		if( LOCAL_STORAGE[i].key == key ){
			printf("[+] HIT key %d\n", key);
			return TRUE;
		}
	}
	printf("[+] MISS key %d\n", key);
	return FALSE;
}
// PUT Item
int storage_put_item(item_t item){
	if( STORAGE_COUNT < MAX_STORAGE && !storage_has_key(item.key) ){ // Don't accept same keys
		LOCAL_STORAGE[STORAGE_COUNT++] = item;
		return TRUE; // Item added to local storage
	}
	return FALSE; // Don't have enough memory or key hit
}
// GET Item
int storage_get_item(key_t key, item_t *item_return){
	int i;
	for(i=0; i < STORAGE_COUNT; i++){// Percorre LOCAL_STORAGE procurando pela chave key
		if( LOCAL_STORAGE[i].key == key ){
			printf("[+] HIT key %d\n", key);
			*item_return =  LOCAL_STORAGE[i];
			return TRUE;
		}
	}
	printf("[+] MISS key %d\n", key);
	return FALSE;
}
//
void print_item(item_t item){
	printf("Item: (%d, %d)\n", item.key, item.value);
}
//
// DHT Backend Procedures
//
int configure_next_node_addr(){
		
}
int dht_send_get(key_t key){
	int syscall_flag, udp_socket;
	// 
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	// Format Request Message
	char request[REQUEST_SIZE_GET];
	sprintf(request, "GET %d", key);
	//
	
	// Send request to the next peer in the chain
	syscall_flag = sendto(udp_socket, request, REQUEST_SIZE_GET, MSG_CONFIRM,(struct sockaddr *) &peer_next_addr,
		sizeof(peer_next_addr));
	if(syscall_flag > 0){
		printf("[+] OK - Sent Request: '%s'\n", request);
		printf("[+] WAIT - Waiting for Response..\n");
		//
		char response_buffer[RESPONSE_SIZE_BUFFER];
		int response_len;
		syscall_flag = recvfrom(udp_socket, (char *)response_buffer, RESPONSE_SIZE_BUFFER,  
                MSG_WAITALL, (struct sockaddr *) &peer_next_addr, &response_len); 
        if(syscall_flag > 0){
			printf("[+] OK - Received Response\n");
		}else{
			printf("[-] ERR - Cannot Receive Response\n");	
		}       
		return TRUE;
	}
	return FALSE;
}
//
int dht_send_put(key_t key, value_t value){
	int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	return TRUE;
}
//
int dht_proccess_request(){
	return FALSE;
}
//
int dht_return_response(){
	return FALSE;
}
int start_node(){
	// get local port
	// Local Storage Memory Alloc
	LOCAL_STORAGE = (item_t*) malloc(MAX_STORAGE * sizeof(item_t) );
	// Configuring Next Node Information
	peer_next_addr.sin_family = AF_INET; 
	peer_next_addr.sin_port = htons(PEER_NEXT_PORT); 
	peer_next_addr.sin_addr.s_addr = INADDR_ANY; 
	// Configuring Previus Node Information
	peer_prev_addr.sin_family = AF_INET; 
	peer_prev_addr.sin_port = htons(PEER_PREV_PORT); 
	peer_prev_addr.sin_addr.s_addr = INADDR_ANY;
	return TRUE; 
}
//
// Main
//
int main(int argc, char **argv){
	// Initializing Local Node
	if( start_node() ){
		printf("[+] OK : Node Online\n");
	}else{
		printf("[+] ERR : Cannot Initialize Local Node\n");
	}
	// Send request
	if( dht_send_get(17))
		printf("[+] OK : Request Sent\n");
	else
		printf("[+] ERRR : Cannot send the Request\n");

	
	return 0;
}

