#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
/*************************************** 
 * Variaveis do Sistema Operacional e da MMU
 ***************************************
 ***************************************/
#define TRUE 1
#define FALSE 0
#define ENDERECO_VIRTUAL_BITS 16 // qtd bits endereco virtual
#define ENDERECO_REAL_BITS 15 // qtd bits endereco fisico
#define PAGE_BITS 12 // bits deslocamento na pagina, 2^12 = K
//
#define QTD_PAGES (int) pow(2, ENDERECO_VIRTUAL_BITS - PAGE_BITS)
#define QTD_FRAMES (int) pow(2, ENDERECO_REAL_BITS - PAGE_BITS)
#define PAGE_SIZE (int) pow(2, PAGE_BITS)
#define LIMIT_ENDERECO_VIRTUAL (int) pow(2, ENDERECO_VIRTUAL_BITS)
//
int endereco_fisico(int frame_index, int deslocamento);
void init_lru();
int random_page();
int PAGE_FAULTS = 0;
//
// Entrada na tabela de páginas
typedef struct page_tab_entry{
	int frame_n;
	int presente; // bit presente
} page_tab_entry_t;
page_tab_entry_t* PAGE_TABLE;
// Entrada na tabela de QUADROS
typedef struct frame_tab_entry{
	int modificado; // bit modificada
	int referenciado;  // bit referenciada
	int pagina_associada;
} frame_tab_entry_t;
frame_tab_entry_t* FRAME_TABLE;
/*************************************** 
 * Configura estruturas de dados
 ***************************************
 ***************************************/
void init(){
	printf("---------\n--------\n");
	printf("NUMERO DE PAGINAS: %d\n", QTD_PAGES);
	printf("NUMERO DE FRAMES: %d\n", QTD_FRAMES);
	printf("---------\n--------\n");
	PAGE_TABLE = (page_tab_entry_t*) malloc(sizeof(page_tab_entry_t) * QTD_PAGES);
	FRAME_TABLE = (frame_tab_entry_t*) malloc(sizeof(frame_tab_entry_t) * QTD_FRAMES);
	int i;
	for(i=0; i < QTD_FRAMES; i++){
		FRAME_TABLE[i].pagina_associada = -1;// inicialmente sem pagina associada
	}

	init_lru();// ALGORITMO LRU
	srand(time(NULL));// ALGORITMO RANDOM_PAGE

}
/*************************************** 
 * Manipulações de BITS
 ***************************************
 ***************************************/
int indice_pagina(int endereco_virtual){
	// pega os primeiros bits do endereco(desloca o tamanho da pagina)
	int page_index = (int) (endereco_virtual >> PAGE_BITS);
	return page_index;
}																																												
//
int deslocamento_quadro(int endereco_virtual){
	// pega os ultimos bits do endereco
	int complemento = PAGE_SIZE - 1;// 000000001111111... 
	int deslocamento = endereco_virtual & complemento;// and bitwise
	return deslocamento;
}
//
int endereco_fisico(int frame_index, int deslocamento){
	frame_index = frame_index << PAGE_BITS;// rotaciona indice para a direita
	return frame_index | deslocamento;// bitwise or
}
/*************************************** 
 * ALGORÌTMOS de SUBSTITUIÇÂO DE PÀGINA
 ***************************************
 ***************************************/
int select_frame(){
	int frame_index = random_page();//random_page();
	return frame_index;
}
// Least Recently Used
int** matrix_LRU;
void init_lru(){
	matrix_LRU = (int**) malloc(sizeof(int*) * QTD_FRAMES);// aloca matriz
	int i, j;
	for(i=0; i < QTD_FRAMES; i++){
		matrix_LRU[i] = (int*) malloc(sizeof(int)*QTD_FRAMES);
		for(j=0; j < QTD_FRAMES; j++)
			matrix_LRU[i][j] = 0;
	}
}
void lru_acessa_quadro(int k){
	int i;
	for(i=0; i < QTD_FRAMES; i++){
		matrix_LRU[k][i] = 1;// linha k = 1
		matrix_LRU[i][k] = 0;// coluna k = 0
	}
	
}
void print_matrix(){
	int i, j;
	for(i=0; i < QTD_FRAMES; i++){
		for(j=0; j < QTD_FRAMES; j++)
			printf("%d ",matrix_LRU[i][j]);
		printf("\n");
	}
}
int lru_algorithm(){
	int i, j, sum = 0, sum_menor = QTD_FRAMES, menos_recentemente_usada = 0;
	for(i=0; i < QTD_FRAMES; i++){// para cada linha
		// soma linha
		for(j=0; j < QTD_FRAMES; j++)
			sum += matrix_LRU[i][j];
		if(sum < sum_menor){
			sum_menor = sum;
			menos_recentemente_usada = i;// se torna a menos recentemente usada
		}
		sum = 0;
	}
	return menos_recentemente_usada;
}
// Random
int random_page(){
	return (int) (rand() % QTD_FRAMES);
}
/*************************************** 
 * MMU e SO
 ***************************************
 ***************************************/
void resolve_frame(int frame_index, int page_index, int endereco_virtual){
	if( FRAME_TABLE[frame_index].modificado );
		//printf("Frame Modificado, copiando anterior no disco");
	// invalida pagina que estava apontando para esse frame, anteriormente
	int pagina_associada = FRAME_TABLE[frame_index].pagina_associada;
	if( pagina_associada == -1 );
		//printf("SEM PAGINA ASSOCIADA \n");

	PAGE_TABLE[pagina_associada].presente = FALSE;
	// associa frame com a nova página
	FRAME_TABLE[frame_index].pagina_associada = page_index;
	PAGE_TABLE[page_index].frame_n = frame_index;  // atualiza mapeamento
	PAGE_TABLE[page_index].presente = TRUE;  // define como presente
	// DEBUG
	int end_fisico = endereco_fisico(frame_index, 0);
	// encontra o endereço fisico do frame_index, sem deslocamento (ou seja, o ponto inicial do frame)
	//printf("Copiando dados do disco no FRAME %d: [%d-%d]\n", frame_index,
	//	end_fisico, end_fisico+PAGE_SIZE);
}


//
// Trata da falta de página
//
int page_fault(int page_index, int endereco_virtual){
	PAGE_FAULTS++;
	//printf("PAGE %d FAULT ",page_index);
	//printf("-----------------------------------------------------------------\n");
	int novo_quadro = select_frame();// seleciona o frame
	resolve_frame(novo_quadro, page_index, endereco_virtual);// resolve pendencias do frame(se ele precisa ser gravado em disco etc)
	return novo_quadro;
}
int virtual_to_real(int endereco_virtual){
	// verifica validade do endereco
	if(endereco_virtual < 0 || endereco_virtual > LIMIT_ENDERECO_VIRTUAL){
		//printf("ERRO! ENDEREÇO FORA DO ESPAÇO DE ENDEREÇAMENTO\n");
		return -1;
	}
	int page_index = indice_pagina(endereco_virtual);// encontra a pagina
	int frame_index;
	//printf("ENDEREÇO CAIU NA PAGINA %d\n", page_index);
	// verifica se está na memória 
	if( PAGE_TABLE[page_index].presente == FALSE)// pagina virtual nao esta mapeada para um quadro
		frame_index = page_fault(page_index, endereco_virtual);// chama o SO
	else{
		frame_index = PAGE_TABLE[page_index].frame_n;
		//printf("PAGINA PRESENTE EM MEMÓRIA, NO FRAME %d\n", frame_index);
	}
	// referenciou
	FRAME_TABLE[frame_index].referenciado = TRUE;
	// ALGORITMO LEAST RECENTLY USED
	lru_acessa_quadro(frame_index);
	//
	int deslocamento = deslocamento_quadro(endereco_virtual);// deslocamento dentro do quadro
	int end_fisico = endereco_fisico(frame_index, deslocamento);
	//
	//printf("DESLOCAMENTO: %d\n", deslocamento);
	return end_fisico;
}
void acessa_endereco(int endereco){
	//printf("----- acessando endereço VIRTUAL %d: \n",endereco);
	int endereco_fisico = virtual_to_real(endereco);
	//printf("endereço FÍSICO resultante: %d\n", endereco_fisico);
	//printf("-----------");
}
/*************************************** 
 * Teste
 ***************************************
 ***************************************/
void roda_experimento(int qtd_experimentos, int N_ACESSOS_MEMORIA){
	int k, min = N_ACESSOS_MEMORIA, max = -1;
	float sum_to_mean = 0.0;
	int page_faults[qtd_experimentos]; 
	for(k=0; k < qtd_experimentos; k++){
		int endereco = 0, endereco2 = 0, i = 0;
		endereco2 = (int) (rand() % LIMIT_ENDERECO_VIRTUAL);

		for(i = 0; i < N_ACESSOS_MEMORIA; i++){
			do{
				endereco = (int) (rand() % LIMIT_ENDERECO_VIRTUAL);
				// acessa o endereço aleatorio com probabilidade 0.2
				if((rand() % 100) < 20)
					break;

			// enquanto for distande do endereco anterior
			}while( abs(endereco - endereco2) > (int)(LIMIT_ENDERECO_VIRTUAL/8) );
			endereco2 = endereco;
			acessa_endereco(endereco);
		}
		//printf("\n\n ACESSOS A MEMORIA: %d\nPAGE FAULTS: %d \n\n", N_ACESSOS_MEMORIA, PAGE_FAULTS);
		page_faults[k] = PAGE_FAULTS;
		sum_to_mean += PAGE_FAULTS;
		if(min > PAGE_FAULTS)
			min = PAGE_FAULTS;
		if(max < PAGE_FAULTS)
			max = PAGE_FAULTS;
		PAGE_FAULTS = 0;

	}

	printf("EXPERIMENTOS: %d\nNUMERO DE ACESSOS: %d\nMEDIA DE PAGE FAULTS: %f\n", qtd_experimentos,
		N_ACESSOS_MEMORIA, (sum_to_mean/qtd_experimentos));
	printf("MINIMO DE PAGE_FAULTS: %d\nMAXIMO DE PAGE_FAULTS: %d\n", min, max);
}
int main(){
	init();
	roda_experimento(1000, 1000);
	return 0;
}