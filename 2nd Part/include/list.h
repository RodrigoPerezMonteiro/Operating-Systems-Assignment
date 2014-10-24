#ifndef __LIST_H_
#define __LIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <kos_client.h>

typedef struct lst_iitem {
   char key[KV_SIZE], value[KV_SIZE];
   struct lst_iitem *next;
   long file_pos; //Posição do par Key-Value no ficheiro
} lst_iitem_t;

typedef struct {
   lst_iitem_t * first;
} list_t;

// aloca memória para uma nova lista e inicializa-a
list_t* lst_new();

// liberta a memória associada à list
void lst_destroy(list_t *);

// insere um novo item com valor "value" na list
void lst_insert(list_t *list, char *key, char *value);

// remove o 1º item com a key "key" da list
void lst_remove(list_t *list, char *key);

// imprime o conteúdo da list
void lst_print(list_t *list);

char* lst_search(list_t *list, char *key);

long lst_getFilePos(list_t *list, char *key);

void lst_setFilePos(list_t *list, char *key, long pos);

void lst_resetAllFilePos(list_t *list);

#endif