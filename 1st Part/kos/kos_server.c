#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <kos_server.h>
#include <hash.h>
#include <buffer.h>
#include <delay.h>
#include <pthread.h>

#define delayON 1

HT_t **shards;
pthread_mutex_t *trinco;

int kos_server_init(int num_server_threads, int buf_size, int num_shards)
{
  shards = (HT_t**) malloc(sizeof(HT_t*) * num_shards);
  trinco = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * num_shards);

  int i, value = 0;
  
  for (i=0; i<num_shards; i++){
    shards[i] = ht_init(i);
    pthread_mutex_init(&trinco[i], NULL);
  }
  
  if(shards == NULL) 
  { value = -1; }
  
  return value;
}

void kos_server_processRequest(int *id)
{
  int clientid = *id;

  while(1) {
    TAREFA_BUFFER *currentRequest = pop(clientid);
    
    int requestType = currentRequest->request;
    if ((requestType >= 1) && (requestType <= 4)){ //VERIFICAÇÃO DE VALIDADE DO PEDIDO  
      
      if(delayON) { delay(); }
      
      pthread_mutex_lock(&trinco[currentRequest->IDshard]);
      
      if (requestType == 1) { //GET
	currentRequest->answer = ht_get(shards[currentRequest->IDshard], currentRequest->key);
      }
      else if (requestType == 2) { //PUT
	currentRequest->answer = ht_insert(shards[currentRequest->IDshard], currentRequest->key, currentRequest->value);
      }
      else if (requestType == 3) { //REMOVE
	currentRequest->answer = ht_remove(shards[currentRequest->IDshard], currentRequest->key);
      }
      else if (requestType == 4) { //GET ALL KEYS
	currentRequest->kval = ht_get_all_keys(shards[currentRequest->IDshard], currentRequest->dim);
      }
      
      pthread_mutex_unlock(&trinco[currentRequest->IDshard]);
    }
    sem_post(currentRequest->semaforo);
  }
}
