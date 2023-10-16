#include "rcu.h"
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define READERS (8)
#define WRITERS (1)

rcu_list __attribute__((aligned(64))) *list;

void *writer(void *id)
{
	int me = (long) id;
	AUDIT
	{
		printf("Thread_w[%d] active\n", me);
		fflush(stdout);
	}
}

void *reader(void *id)
{
	int me = (long) id;
	
	AUDIT
	{
		printf("Thread_r[%d] active\n", me);
		fflush(stdout);
	}
}

void list_init(rcu_list *l)
{
	node *head = malloc(sizeof(node));
	if (head == NULL)
	{
		perror("Cannot create head");
		fflush(stderr);
		exit(EXIT_FAILURE);
	}
	
	head->key = 0;
	l->tail = NULL;
	head->next = l->tail;
		
}

int main(int argc, char **argv)
{
	
	pthread_t *tid_w = malloc(sizeof(pthread_t) * WRITERS);
	pthread_t *tid_r = malloc(sizeof(pthread_t) * READERS);
	long i;	
	list = malloc(sizeof(rcu_list));
	
	if (list == NULL){
		perror("Cannot create the list");
		fflush(stderr);		
		exit(EXIT_FAILURE);
	}	
	
	memset(list, 0, sizeof(rcu_list));
	list_init(list);

	for (i = 0; i < WRITERS; i++){
	
ptc_w:
		if (pthread_create(&tid_w[i], NULL, writer, (void *)i))
		{
			if (errno == EINTR) goto ptc_w;
			perror("Cannot create thread writer");
			fflush(stderr);
			exit(EXIT_FAILURE);
		}	
	}
	
	for (i = 0; i < READERS; i++){
ptc_r:	
		if (pthread_create(&tid_r[i], NULL, reader, (void *)i))
		{
			if (errno == EINTR) goto ptc_r;
			perror("Cannot create thread reader");
			fflush(stderr);
			exit(EXIT_FAILURE);		
		}
	}
	
	pause();
	
}
