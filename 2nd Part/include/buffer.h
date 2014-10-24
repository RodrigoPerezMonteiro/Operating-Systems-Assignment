#ifndef BUFFER_H
#define BUFFER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <kos_client.h> 

typedef struct
{
  KV_t* kval;
  char* answer;
  int request;
  int IDclient;
  int IDshard;
  char* key;
  char* value;
  int *dim;
  sem_t* semaforo;
  
} TAREFA_BUFFER;

int buffer_init(int buffer_size);
void push(TAREFA_BUFFER* pedido);
TAREFA_BUFFER* pop();

#endif