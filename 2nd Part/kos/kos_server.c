#include <kos_server.h>

#define DELAY OFF

HT_t **shards;
pthread_mutex_t *trinco;

int kos_server_init(int num_server_threads, int buf_size, int num_shards)
{
  shards = (HT_t**) malloc(sizeof(HT_t*) * num_shards);

  int i, value = 0;
  
  for (i=0; i<num_shards; i++) { shards[i] = ht_init(i); }
  
  if(shards == NULL) 
  { value = -1; }
  
  return value;
}

void kos_server_processRequest(int *id)
{
  
  while(1) 
  {
    TAREFA_BUFFER *currentRequest = pop();
    
    int requestType = currentRequest->request;
    if ((requestType >= MIN_REQUEST) && (requestType <= MAX_REQUEST))
    {
      if(DELAY) { delay(); }
      
      if (requestType == GET) {	currentRequest->answer = ht_get(shards[currentRequest->IDshard], currentRequest->key); }
      else if (requestType == PUT) { currentRequest->answer = ht_insert(shards[currentRequest->IDshard], currentRequest->key, currentRequest->value); }
      else if (requestType == REMOVE) { currentRequest->answer = ht_remove(shards[currentRequest->IDshard], currentRequest->key); }
      else if (requestType == GETALL) { currentRequest->kval = ht_get_all_keys(shards[currentRequest->IDshard], currentRequest->dim); }   
      
      sem_post(currentRequest->semaforo);
    }
  }
}
