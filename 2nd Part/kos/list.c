#include "list.h"


list_t* lst_new()
{
  list_t *list;
  list = (list_t*) malloc(sizeof(list_t));
  list->first = NULL;
  return list;
}

void lst_insert(list_t *list, char *key, char *value)
{
  lst_iitem_t *lst = (lst_iitem_t*) malloc(sizeof(lst_iitem_t));
  strcpy(lst->key, key);
  strcpy(lst->value, value);
  
  lst->next = list->first;
  list->first = lst;
}

void lst_remove(list_t *list, char* key) 
{
  lst_iitem_t *l, *aux;
  l = list->first;
  
  while(l != NULL)
  {
    if(!strcmp(l->key , key)) 
    {
	    if(l == list->first) 
	    {
	      list->first = l->next;
	      free(l);
	      return;
	    } 
	    
	    else
	    {
	      aux->next = l->next;
	      free(l);
	      return;
	    }
    } //if

    else
    {
      aux = l;
      l = l->next;
    }
  }
}


void lst_print(list_t *list)
{
  lst_iitem_t *aux;
  
  aux = list->first;
  
  while(aux) 
  {
    printf("%s\n", aux->key);
    aux = aux->next;
  }
}


char* lst_search(list_t *list, char *key)
{
  lst_iitem_t *aux;
  
  if (list->first != NULL)
  {
    aux = list->first;
  
    while(aux) 
    {
      if (!strcmp(key, aux->key))
      {
	return (aux->value); 
      }
	aux = aux->next;
    }
  }
  
  return NULL;
}

long lst_getFilePos(list_t *list, char *key)
{
  lst_iitem_t *aux;
  long pos = -1;
  
  if (list->first != NULL)
  {
    aux = list->first;
  
    while(aux) 
    {
      if (!strcmp(key, aux->key))
      {
	pos = aux->file_pos; 
	break;
      }
	aux = aux->next;
    }
  }
  return pos;
}

void lst_setFilePos(list_t *list, char *key, long pos)
{
  lst_iitem_t *aux;
  
  if (list->first != NULL)
  {
    aux = list->first;
  
    while(aux) 
    {
      if (!strcmp(key, aux->key))
      {
	aux->file_pos = pos;
	break;
      }
	aux = aux->next;
    }
  }
}

void lst_resetAllFilePos(list_t *list)
{
  lst_iitem_t *aux;
  
  if (list->first != NULL)
  {
    aux = list->first;
  
    while(aux) 
    {
      aux->file_pos = -1;
      aux = aux->next;
    }
  }
}