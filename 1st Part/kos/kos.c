#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <kos_client.h>
#include <kos_server.h>
#include <hash.h>
#include <buffer.h>

int kos_init(int num_server_threads, int buf_size, int num_shards) {	
    int i;

    if ((kos_server_init(num_server_threads, buf_size, num_shards) !=0) ||
	(buffer_init(buf_size) != 0))
    { return -1; }
    
    // THREADS

    pthread_t *serverTasks;
    serverTasks = (pthread_t*) malloc(sizeof(pthread_t) * num_server_threads);
    
    int *tasksid = (int*) malloc(sizeof(int) * num_server_threads);
    
    for (i=0; i<num_server_threads; i++){
      tasksid[i] = i;
      if (pthread_create(&serverTasks[i], NULL, (void*)kos_server_processRequest, &tasksid[i]) != 0){
	printf("Error: Thread creation.\n");
	return -1;
      }
    }
    
    return 0;
}

char* kos_get(int clientid, int shardId, char* key) {
    TAREFA_BUFFER *pedido = (TAREFA_BUFFER*) malloc(sizeof(TAREFA_BUFFER));
    sem_t semaforo;
    sem_init(&semaforo, 0, 0);
    
    pedido->IDclient = clientid;
    pedido->IDshard = shardId;
    pedido->key = key;
    pedido->request = 1;
    pedido->semaforo = &semaforo;
    
    push(pedido);
    
    sem_wait(&semaforo);
    
    char* answer = pedido->answer;
    
    free(pedido);
    
    return answer;
}

char* kos_put(int clientid, int shardId, char* key, char* value) {
    TAREFA_BUFFER *pedido = (TAREFA_BUFFER*) malloc(sizeof(TAREFA_BUFFER));  
    sem_t semaforo;
    sem_init(&semaforo, 0, 0);
    
    pedido->IDclient = clientid;
    pedido->IDshard = shardId;
    pedido->key = key;
    pedido->value = value;
    pedido->request = 2;
    pedido->semaforo = &semaforo;
    
    push(pedido);
    
    sem_wait(&semaforo);
    
    char* answer = pedido->answer;
    
    free(pedido);
    
    return answer;
}

char* kos_remove(int clientid, int shardId, char* key) {
    TAREFA_BUFFER *pedido = (TAREFA_BUFFER*) malloc(sizeof(TAREFA_BUFFER));
    sem_t semaforo;
    sem_init(&semaforo, 0, 0);
    
    pedido->IDclient = clientid;
    pedido->IDshard = shardId;
    pedido->key = key;
    pedido->request = 3;
    pedido->semaforo = &semaforo;
    
    push(pedido);
    
    sem_wait(&semaforo);
    
    char* answer = pedido->answer;
    
    free(pedido);
    
    return answer;
}

KV_t* kos_getAllKeys(int clientid, int shardId, int* dim) {
    TAREFA_BUFFER *pedido = (TAREFA_BUFFER*) malloc(sizeof(TAREFA_BUFFER));
    sem_t semaforo;
    sem_init(&semaforo, 0, 0);
    
    pedido->IDclient = clientid;
    pedido->IDshard = shardId;
    pedido->dim = dim;
    pedido->request = 4;
    pedido->semaforo = &semaforo;
    
    push(pedido);
    
    sem_wait(&semaforo);
    
    KV_t* answer = pedido->kval;
    
    free(pedido);
    
    return answer;
}