#ifndef __KOS_SERVER_H__
#define __KOS_SERVER_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int kos_server_init(int num_server_threads, int buf_size, int num_shards);
void kos_server_processRequest(int *id);

#endif