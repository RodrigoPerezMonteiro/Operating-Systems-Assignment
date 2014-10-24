#ifndef __HASHMAP_H
#define __HASHMAP_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <list.h>
#include <delay.h>
#include <config.h>

#define HT_SIZE 10
#define SIZE 10000

typedef struct{
    list_t* nodes[HT_SIZE];
    int total, shard_id;
    
    int nleitores[HT_SIZE];
    int em_escrita[HT_SIZE];
    int leitores_espera[HT_SIZE];
    int escritores_espera[HT_SIZE];
    sem_t leitores[HT_SIZE];
    sem_t escritores[HT_SIZE]; 
    pthread_mutex_t mutex[HT_SIZE];
    
    pthread_mutex_t ficheiros;
    
    pthread_mutex_t vector;
    
    long *empty_lines;
    int last;
    
    int full_lines; //P2
    int deleted_lines;
    
    FILE *ficheiro;
} HT_t;

HT_t * ht_init(int shard_id);
char * ht_insert(HT_t *ht_shard, char *key, char *value);
char * ht_insert_fromFile(HT_t *ht_shard, char *key, char *value, long offset);
char * ht_remove(HT_t *ht_shard, char *key);
char * ht_get(HT_t *ht_shard, char *key);
KV_t * ht_get_all_keys(HT_t *ht_shard, int *dim);

void inicia_leitura(HT_t *ht_shard, int pos);
void acaba_leitura(HT_t *ht_shard, int pos);
void inicia_escrita(HT_t *ht_shard, int pos);
void acaba_escrita(HT_t *ht_shard, int pos);

void insere_linha(char *filename, char *linha, long size, int lista, char *key);
long procura_linha(char *filename, char *string);

void sortVector(HT_t *ht_shard);
void clearVector(HT_t *ht_shard);

void compressFile(HT_t *ht_shard);

#endif
