#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <buffer.h>
#include <pthread.h>

sem_t *semaphore;
TAREFA_BUFFER** buffer;

int BUFFER_SIZE = 0;

int buffer_init(int buffer_size) {
	
  int i, value = 0;
	
  BUFFER_SIZE = buffer_size;

  semaphore = (sem_t*) malloc (sizeof(sem_t)*buffer_size);
  buffer = (TAREFA_BUFFER**) malloc (sizeof(TAREFA_BUFFER*)*buffer_size);
	
  if(buffer == NULL || semaphore == NULL)
  { value = -1; }
			
  for (i=0; i<buffer_size; i++) { sem_init(&(semaphore[i]), 0, 0); }

  return value;
}

void push(TAREFA_BUFFER* pedido) {
	
  int pos = (pedido->IDclient) % BUFFER_SIZE;
  buffer[pos]=pedido;
	
  sem_post(&(semaphore[pos]));
}

TAREFA_BUFFER* pop(int pos) {
	
  TAREFA_BUFFER* reply;
  
  pos = pos % BUFFER_SIZE;
  
  sem_wait(&(semaphore[pos]));
  
  reply = buffer[pos];
	
  return reply;
}