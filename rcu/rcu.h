#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

#define AUDIT if(0)
#define MAX_EPOCHS (2)
#define SHIFT_ELEM 3
#define MASK = 0x1

typedef struct node {
	long key;
	struct node *next;
}node;

typedef struct rcu_list{
	unsigned long epoch;
	unsigned long reads[MAX_EPOCHS];
	char write_lock;
	node *head;
	node *tail;
}__attribute__((packed)) rcu_list;


extern int rcu_insert(rcu_list *l, long key);
extern int rcu_search(rcu_list *l, long key);
extern int rcu_remove(rcu_list *l, long key); 
