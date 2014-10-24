#ifndef __HASHMAP_H
#define __HASHMAP_H
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "list.h"

#define HT_SIZE 10

typedef struct{
    list_t* nodes[HT_SIZE];
    int total, shard_id;
} HT_t;

HT_t * ht_init(int shard_id);
char * ht_insert(HT_t* hashtable,char* key, char* value);
char * ht_remove(HT_t* hashtable,char* key);
char * ht_get(HT_t* hashtable, char* key);
KV_t * ht_get_all_keys(HT_t* hashtable, int* dim);

#endif