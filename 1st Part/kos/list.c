/*
 * list.c - implementation of the integer list functions 
 */

#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <string.h>

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

void lst_remove(list_t *list, char* key) {
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