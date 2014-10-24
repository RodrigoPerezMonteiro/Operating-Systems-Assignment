#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hash.h>

HT_t* hashtable;
 
int hash(char* key) { 
    int i=0;
 
    if (key == NULL)
    return -1;
 
    while (*key != '\0') {
        i+=(int) *key;
        key++;
    }
    
    i=i % HT_SIZE;
    return i;
}

HT_t* ht_init(int shard_id){
  int i;
  hashtable = (HT_t*) malloc(sizeof(HT_t));
  hashtable->total = 0;
  hashtable->shard_id = shard_id;
  for(i=0;i<HT_SIZE;i++){
    hashtable->nodes[i] = lst_new();
  }
  return hashtable;
}

char * ht_insert(HT_t* hashtable,char* key, char* value){
  int posicao = hash(key);
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(hashtable->nodes[posicao],key);
  if(lastValuePtr == NULL){
    lst_insert(hashtable->nodes[posicao],key,value);
    hashtable->total++;
    return NULL;
  } else { 
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
    lst_remove(hashtable->nodes[posicao],key);
    lst_insert(hashtable->nodes[posicao],key,value);
    return lastValue;
  }
}

char * ht_remove(HT_t* hashtable,char* key){
  int posicao = hash(key);
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(hashtable->nodes[posicao],key);
  if(lastValuePtr != NULL){
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
    lst_remove(hashtable->nodes[posicao],key);
    hashtable->total--;
    return lastValue;
  }
  return NULL;
}

char * ht_get(HT_t* hashtable, char* key){
  int posicao = hash(key);
  char* lastValue = NULL;
  char* lastValuePtr = lst_search(hashtable->nodes[posicao],key);
  if(lastValuePtr != NULL){
    lastValue = (char*) malloc(sizeof(char)*strlen(lastValuePtr));
    strcpy(lastValue,lastValuePtr);
    return lastValue;
  }
  return NULL;
}

KV_t * ht_get_all_keys(HT_t* hashtable, int *dim){
  int copied = 0, pos = 0, keys_pos = 0, total = hashtable->total;
  
  KV_t * keys = (KV_t *) malloc (sizeof(KV_t) * total);
  *dim = total;
  
  while(copied < total)
  {
    lst_iitem_t * temp = (lst_iitem_t *) malloc (sizeof(lst_iitem_t));
    temp = hashtable->nodes[pos]->first;
    
    while (temp != NULL)
    {
      strcpy(keys[keys_pos].key, temp->key);
      strcpy(keys[keys_pos].value, temp->value);
      temp = temp->next;
      keys_pos ++;
    }
    pos ++;
  }
  return keys;
}
