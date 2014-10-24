#include <hash.h>

HT_t* ht_shard;

int hash(char* key) 
{ 
    int i=0;
 
    if (key == NULL) { return -1; }
 
    while (*key != '\0') 
    {
      i+=(int) *key;
      key++;
    }
    
    i = i % HT_SIZE;
    
    return i;
}

HT_t* ht_init(int shard_id){
  int i;
  ht_shard = (HT_t*) malloc(sizeof(HT_t));
  ht_shard->total = 0;
  ht_shard->shard_id = shard_id;
  ht_shard->last = -1;
  ht_shard->empty_lines = (long *) malloc(sizeof(long)*SIZE);
  
  pthread_mutex_init(&(ht_shard->ficheiros), NULL);  
  pthread_mutex_init(&(ht_shard->vector), NULL);
    
  if(P2_OPTIMIZATION) { ht_shard->full_lines = ht_shard->deleted_lines = 0; }
  
  for(i=0;i<HT_SIZE;i++)
  {  
      ht_shard->nodes[i] = lst_new();
      
      ht_shard->nleitores[i] = 0;
      ht_shard->em_escrita[i] = 0; //FALSE
      ht_shard->leitores_espera[i] = 0;
      ht_shard->escritores_espera[i] = 0;
      
      pthread_mutex_init(&(ht_shard->mutex[i]), NULL);
      
      sem_init(&(ht_shard->leitores[i]), 0, 0);
      sem_init(&(ht_shard->escritores[i]), 0, 0);   
      
      ht_shard->empty_lines[i] = 0;
  }
  
  char filename[100];
  sprintf(filename, "fshard%d.txt", shard_id);
  long int offset = 0;
  
  if(fopen(filename, "r") != NULL)
  {
    if(READ_FROM_FILE)
    {      
      FILE *new = fopen(filename, "r");
      if(P2_OPTIMIZATION) { compressFile(ht_shard); } 
      
      int dim_line = (2 * KV_SIZE) + 3;
      char line[dim_line], key[KV_SIZE+1], value[KV_SIZE+1];
      
      strcpy(line, "");
      strcpy(key, "");
      strcpy(value, "");
      
      while(fgets(line, sizeof(line), new) != NULL)
      {
	sscanf(line, "%s %s\n", key, value);

	if(strcmp(key, "")) 
	{
	  ht_insert_fromFile(ht_shard, key, value, offset);
	}
	
	offset += (2* KV_SIZE) + 2;
	
	strcpy(line, "");
	strcpy(key, "");
	strcpy(value, "");
      }
      
      fclose(new);
    } 
  }
  
  else { ht_shard->ficheiro = fopen(filename, "a+"); }
  
  return ht_shard;
}

void inicia_leitura(HT_t *ht_shard, int pos)
{
  pthread_mutex_lock(&(ht_shard->mutex[pos]));
  
  if(DELAY) { delay(); }
  
  if(((ht_shard->em_escrita[pos]) == 1) || ((ht_shard->escritores_espera[pos]) > 0))
  {
      ht_shard->leitores_espera[pos] ++;
      
      pthread_mutex_unlock(&(ht_shard->mutex[pos]));
      sem_wait(&(ht_shard->leitores[pos]));
      pthread_mutex_lock(&(ht_shard->mutex[pos]));
      
      
      if((ht_shard->leitores_espera[pos]) > 0)
      {
	ht_shard->nleitores[pos] ++;
	ht_shard->leitores_espera[pos] --;
	sem_post(&(ht_shard->leitores[pos]));
      }
  }
  
  else { ht_shard->nleitores[pos] ++; }
  
  pthread_mutex_unlock(&(ht_shard->mutex[pos]));
}

void acaba_leitura(HT_t *ht_shard, int pos)
{
  pthread_mutex_lock(&(ht_shard->mutex[pos]));
  
  if(DELAY) { delay(); }
  
  ht_shard->nleitores[pos] --;
  
  if((ht_shard->nleitores[pos] == 0) && (ht_shard->escritores_espera[pos] > 0))
  {
    sem_post(&(ht_shard->escritores[pos]));
    ht_shard->em_escrita[pos] = 1;
    ht_shard->escritores_espera[pos] --;
  }
  
  pthread_mutex_unlock(&(ht_shard->mutex[pos]));
}

void inicia_escrita(HT_t *ht_shard, int pos)
{
  pthread_mutex_lock(&(ht_shard->mutex[pos]));
  if(DELAY) { delay(); }
  
  if((ht_shard->em_escrita[pos] == 1) || (ht_shard->nleitores[pos] > 0) || (ht_shard->leitores_espera[pos] > 0))
  {
    ht_shard->escritores_espera[pos] ++;
    
    pthread_mutex_unlock(&(ht_shard->mutex[pos]));
    sem_wait(&(ht_shard->escritores[pos]));
    pthread_mutex_lock(&(ht_shard->mutex[pos]));
  }
  
  ht_shard->em_escrita[pos] = 1;
  pthread_mutex_unlock(&(ht_shard->mutex[pos]));  
}

void acaba_escrita(HT_t *ht_shard, int pos)
{
  pthread_mutex_lock(&(ht_shard->mutex[pos]));
  if(DELAY) { delay(); }
  
  ht_shard->em_escrita[pos] = 0;
  
  if(ht_shard->leitores_espera[pos] > 0)
  {
    sem_post(&(ht_shard->leitores[pos]));
    ht_shard->nleitores[pos] ++;
    ht_shard->leitores_espera[pos] --;
  }

  else if(ht_shard->escritores_espera[pos] > 0)
  {
    sem_post(&(ht_shard->escritores[pos]));
    ht_shard->em_escrita[pos] = 1;
    ht_shard->escritores_espera[pos] --;
  }
  
  pthread_mutex_unlock(&(ht_shard->mutex[pos]));
}

char * ht_insert(HT_t* ht_shard, char* key, char* value)
{
  int posicao = hash(key);
  
  inicia_escrita(ht_shard, posicao);  
  
  ////////////////////////////////////////////////////////////////////////
  /////////////////////PERSISTÊNCIA NO KOS (FICHEIROS)////////////////////
  ////////////////////////////////////////////////////////////////////////
  
  pthread_mutex_lock(&(ht_shard->ficheiros));  
  
  long search;
  int size_linha = (2 * KV_SIZE) + 3;
  char shard_str[100];
  char linha[size_linha];
  
  sprintf(shard_str,"fshard%d.txt",ht_shard->shard_id);
  
  sprintf(linha,"%-20s %-20s\n", key, value);
  
  if(P1_OPTIMIZATION)
  {
    if(lst_getFilePos(ht_shard->nodes[posicao], key) == -1)
    {
      search = procura_linha(shard_str, key); //-1
    }

    else { search = lst_getFilePos(ht_shard->nodes[posicao], key); }
  }
  
  else { search = procura_linha(shard_str, key); }
  
  if(P3_OPTIMIZATION)
  {
    if(search == -1)
    {
      pthread_mutex_lock(&(ht_shard->vector));
      
      if(ht_shard->last > -1) 
      {
	search = ht_shard->empty_lines[0];
	sortVector(ht_shard);
	
	if(DEBUG_P3_OPTIMIZATION)
	{
	  printf("Inserting @ Pos: %ld, Vector: %d\n", ht_shard->empty_lines[0], 0);
	}
	
	if(P2_OPTIMIZATION) 
	{
	  ht_shard->deleted_lines --; 
	}
	
	ht_shard->last --;
      }  
      pthread_mutex_unlock(&(ht_shard->vector));
    }
  }
      
  insere_linha(shard_str, linha, search, posicao, key);
  
  fflush(ht_shard->ficheiro);
  
  if(P2_OPTIMIZATION) 
  {
    if((ht_shard->deleted_lines >= (P2_OPTIMIZATION_RACIO * ht_shard->full_lines)) && (ht_shard->deleted_lines > P2_MIN))
    {        
	compressFile(ht_shard);
    }  
    
    else { ht_shard->full_lines ++; }
  }
  
  pthread_mutex_unlock(&(ht_shard->ficheiros));  
  
  ////////////////////////////////////////////////////////////////////////
  /////////////////////PERSISTÊNCIA NO KOS (FICHEIROS)////////////////////
  ////////////////////////////////////////////////////////////////////////
  
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(ht_shard->nodes[posicao],key);
  
  if(lastValuePtr == NULL)
  {
    lst_insert(ht_shard->nodes[posicao],key,value);
    ht_shard->total++;

    lastValue = NULL;
    
    if(P1_OPTIMIZATION) { lst_setFilePos(ht_shard->nodes[posicao], key, (long)-1); }
  }
  
  else 
  { 
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
    lst_remove(ht_shard->nodes[posicao],key);
    lst_insert(ht_shard->nodes[posicao],key,value);
  }
    
  acaba_escrita(ht_shard, posicao);
  
  return lastValue;
}

char * ht_remove(HT_t* ht_shard,char* key)
{
  int posicao = hash(key);
  
  inicia_escrita(ht_shard, posicao);
  
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(ht_shard->nodes[posicao],key);
  
  ////////////////////////////////////////////////////////////////////////
  /////////////////////PERSISTÊNCIA NO KOS (FICHEIROS)////////////////////
  ////////////////////////////////////////////////////////////////////////
  
  pthread_mutex_lock(&(ht_shard->ficheiros));  
  
  long search;
  int size_linha = (2 * KV_SIZE) + 3;
  char shard_str[100];
  char linha[size_linha];
  
  sprintf(shard_str,"fshard%d.txt",ht_shard->shard_id);
  
  sprintf(linha,"%-20s %-20s\n", "", "");
  
  if(P1_OPTIMIZATION)
  {
    if(lst_getFilePos(ht_shard->nodes[posicao], key) == -1)
    {
      search = procura_linha(shard_str, key);
    }

    else { search = lst_getFilePos(ht_shard->nodes[posicao], key); }
  }
  
  else { search = procura_linha(shard_str, key); }
  
  insere_linha(shard_str, linha, search, posicao, key);
  
  fflush(ht_shard->ficheiro);
  
  if(P2_OPTIMIZATION) { ht_shard->deleted_lines ++; }
  
  if(P3_OPTIMIZATION)
  {
    pthread_mutex_lock(&(ht_shard->vector));
    
    ht_shard->last ++;
    ht_shard->empty_lines[ht_shard->last] = (long) search;
    
    pthread_mutex_unlock(&(ht_shard->vector));
    
    if(DEBUG_P3_OPTIMIZATION)
    {
      printf("Removing @ Pos: %ld, Vector: %d\n", ht_shard->empty_lines[ht_shard->last], ht_shard->last);
    }
  }
  
  pthread_mutex_unlock(&(ht_shard->ficheiros));    
  
  ////////////////////////////////////////////////////////////////////////
  /////////////////////PERSISTÊNCIA NO KOS (FICHEIROS)////////////////////
  ////////////////////////////////////////////////////////////////////////
  
  if(lastValuePtr != NULL)
  {
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
    lst_remove(ht_shard->nodes[posicao],key);
    ht_shard->total--;
  }
  
  acaba_escrita(ht_shard, posicao);
  
  return lastValue;
}

char * ht_get(HT_t* ht_shard, char* key)
{
  int posicao = hash(key);
  
  inicia_leitura(ht_shard, posicao);
  
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(ht_shard->nodes[posicao],key);
  
  if(lastValuePtr != NULL)
  {
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
  }
  
  acaba_leitura(ht_shard, posicao);
  
  return lastValue;
}

KV_t * ht_get_all_keys(HT_t* ht_shard, int *dim)
{
  int i;
  
  for(i=0; i<HT_SIZE; i++)
  {
    inicia_leitura(ht_shard, i);
  }
  
  int copied = 0, pos = 0, keys_pos = 0, total = ht_shard->total;
  
  KV_t * keys = (KV_t *) malloc (sizeof(KV_t) * total);
  *dim = total;
  
  while(copied < total)
  {
    
    lst_iitem_t * temp = (lst_iitem_t *) malloc(sizeof(lst_iitem_t));
    temp = ht_shard->nodes[pos]->first;
    
    while (temp != NULL)
    {
      strcpy(keys[keys_pos].key, temp->key);
      strcpy(keys[keys_pos].value, temp->value);
      temp = temp->next;
      keys_pos ++;
      copied ++;
    }
    pos ++;
  }
  
  for(i=0; i<HT_SIZE; i++)
  {
    acaba_leitura(ht_shard, i);
  }
  
  return keys;
}

char * ht_insert_fromFile(HT_t* ht_shard, char* key, char* value, long offset)
{
  int posicao = hash(key);
  
  inicia_escrita(ht_shard, posicao);
  
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(ht_shard->nodes[posicao],key);
  
  if(lastValuePtr == NULL)
  {
    lst_insert(ht_shard->nodes[posicao],key,value);
    ht_shard->total++;
    
    lastValue = NULL;
  }
  
  else
  { 
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
    lst_remove(ht_shard->nodes[posicao],key);
    lst_insert(ht_shard->nodes[posicao],key,value);
  }
  
  if (P1_OPTIMIZATION) { lst_setFilePos(ht_shard->nodes[posicao], key, offset); }
  
  acaba_escrita(ht_shard, posicao);
  
  return lastValue;
}

void insere_linha(char *filename, char *linha, long size, int lista, char* key)
{
    FILE *fp = fopen(filename, "r+");
    
    if(fp != NULL)
    {      
      if(size == -1) 
      {
	fseek(fp, 0, SEEK_END);
	if(P1_OPTIMIZATION) { lst_setFilePos(ht_shard->nodes[lista], key, ftell(fp)); }
	fprintf(fp, linha);
      }
      else
      {
	fseek(fp, size, SEEK_SET);
	if(P1_OPTIMIZATION) { lst_setFilePos(ht_shard->nodes[lista], key, size); }
	fprintf(fp, linha);
      }
    }
    fclose(fp);
}

long procura_linha(char *filename, char *string)
{
    FILE *fp;
    int linha_pos = 1;
    int success = 0;
    long size = 0;
    
    if((fp = fopen(filename, "r")) == NULL) { return(-1); }
    
    int dim_line = KV_SIZE + KV_SIZE + 3;
    char line[dim_line], key[KV_SIZE], value[KV_SIZE];
	
    while(fgets(line, sizeof(line), fp) != NULL)
    {
      sscanf(line, "%s %s", key, value);
      size = ftell(fp) - strlen(line);
      
      if(!strcmp(key, string)) 
      {
	success ++;
	break;
      }

      linha_pos++;
    }

    if(success == 0) { size = -1; }
    
    if(fp) { fclose(fp); }
    
    return size;
}

void sortVector(HT_t *ht_shard)
{
  ht_shard->empty_lines[0] = ht_shard->empty_lines[ht_shard->last];
  ht_shard->empty_lines[ht_shard->last] = 0;  
}

void clearVector(HT_t *ht_shard)
{
  int i = 0;
  
  for(i=0; i < ht_shard->empty_lines[ht_shard->last]; i++)
  { ht_shard->empty_lines[i] = 0; }
}

void compressFile(HT_t *ht_shard)
{
  char filename[100];
  char compressed_filename[100];
  int i;
  
  sprintf(filename, "fshard%d.txt", ht_shard->shard_id);
  sprintf(compressed_filename, "fshard%d_compressed.txt", ht_shard->shard_id);
  
  long int offset = 0;
  int dim_line = (2 * KV_SIZE) + 3;
  char line[dim_line], key[KV_SIZE+1], value[KV_SIZE+1];
  
  strcpy(line, "");
  strcpy(key, "");
  strcpy(value, "");
  
  ht_shard->full_lines = 0;
  
  if(fopen(filename, "r") != NULL)
  {  
    FILE *file = fopen(filename, "r");
    FILE *compressed = fopen(compressed_filename, "w");
	
    while(fgets(line, sizeof(line), file) != NULL) 
    {
      sscanf(line, "%s %s\n", key, value);
      
      if(strcmp(key, ""))
      {
	fputs(line, compressed);
	offset += (2* KV_SIZE) + 2;
	ht_shard->full_lines ++;
      }	
      strcpy(line, "");
      strcpy(key, "");
      strcpy(value, "");
    }

    remove(filename);
    rename(compressed_filename, filename);
    
    fclose(file);
    fclose(compressed);
    ht_shard->deleted_lines = 0;
  }
  
  if(P1_OPTIMIZATION)
  {
    for(i=0; i<HT_SIZE; i++)
    {
      lst_resetAllFilePos(ht_shard->nodes[i]);
      printf("RESET DESLOCAMENTO: SHARD_ID: %d LIST_ID: %d\n", ht_shard->shard_id, i);
    }
  }
  
  if(DEBUG_P2_OPTIMIZATION) 
  { printf("Deleted Lines: %d | Full Lines: %d\n", ht_shard->deleted_lines, ht_shard->full_lines); }
}