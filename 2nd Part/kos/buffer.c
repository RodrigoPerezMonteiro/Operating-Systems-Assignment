#include <buffer.h>

TAREFA_BUFFER** buffer;
sem_t pode_prod;
sem_t pode_cons;
pthread_mutex_t mutex_p;
pthread_mutex_t mutex_c;
int BUFFER_SIZE = 0;
int prodptr = 0, consptr = 0;

int buffer_init(int buffer_size) 
{
  int value = 0;
	
  BUFFER_SIZE = buffer_size;
  buffer = (TAREFA_BUFFER**) malloc (sizeof(TAREFA_BUFFER*)*buffer_size);
  
  sem_init(&pode_prod, 0, buffer_size);
  sem_init(&pode_cons, 0, 0);
  
  pthread_mutex_init(&mutex_p, NULL);
  pthread_mutex_init(&mutex_c, NULL);
	
  if(buffer == NULL) { value = -1; }

  return value;
}

void push(TAREFA_BUFFER* pedido) 
{
  sem_wait(&pode_prod);
  
  pthread_mutex_lock(&mutex_p);
  
  buffer[prodptr] = pedido;
  prodptr = (prodptr+1) % BUFFER_SIZE;
  
  pthread_mutex_unlock(&mutex_p);
  
  sem_post(&pode_cons);  
}
 
TAREFA_BUFFER* pop() 
{
  TAREFA_BUFFER *reply;
  
  sem_wait(&pode_cons);
    
  pthread_mutex_lock(&mutex_c);
  
  reply =  buffer[consptr];
  consptr = (consptr+1) % BUFFER_SIZE;
  
  pthread_mutex_unlock(&mutex_c);
  
  sem_post(&pode_prod);
  return reply;
}