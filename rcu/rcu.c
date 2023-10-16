#include "rcu.h"


int rcu_insert(rcu_list *l, long key)
{
	node *new_node = malloc(sizeof(node));

	if (new_node == NULL){
		perror("Cannot create a node to insert");
		fflush(stderr);
		return -1;
	}
	
	new_node->key = key;
	new_node-> next = NULL;
	
	AUDIT
	{
		printf("rcu_insert: get write lock\n");
		fflush(stdout);
	}
insert_fun_retry_write_lock:
	
	if (!__sync_val_compare_and_swap(&l->write_lock, 0, 1)) goto insert_fun_retry_write_lock; // it is not good
	
	// critical section on the list
	
	node *temp = l->tail;
	l->tail->next = new_node;
	asm volatile ("mfence");
	l->tail = new_node;
	asm volatile ("mfence");
insert_fun_retry_write_unlock:
	
	if (!__sync_val_compare_and_swap(&l->write_lock, 1, 0)) goto insert_fun_retry_write_unlock; // it is not good	
	return 0;
}

int rcu_search(rcu_list *l, long key)
{
	unsigned long *epoch = (&(l -> epoch)); //+ offsetof(rcu_list, epoch);;
	unsigned long this_search_epoch;
	int epoch_index;
	int ret;
	long key_shifted = key ;
	
	node *cursor;
	
	this_search_epoch = __sync_fetch_and_add(epoch, 1);
	
	cursor = l -> head;
	
	
	
	while (cursor != NULL){
		if ( cursor -> key == key){
			break;
		}
		cursor = cursor -> next; 
	}
	
	__sync_fetch_and_add(&l -> reads[this_search_epoch], 1);
	
	ret = cursor ? 0:1;
	
	__sync_fetch_and_sub(&l -> reads[this_search_epoch], 1);
	
	return ret;
}

int rcu_remove(rcu_list *l, long key)
{
	node * cursor;
	node *remove_element = NULL;
	int grace_period;
	unsigned long last_epoch;
	unsigned long update_epoch;
	unsigned long grace_period_threads;
	int index;
	
remove_fun_retry_write_lock:
	if (!__sync_val_compare_and_swap(&l->write_lock, 0, 1)) goto remove_fun_retry_write_lock;
	
	cursor = l->head;
	
	if(cursor != NULL && cursor->key == key)
	{
		remove_element = cursor;
		l->head = remove_element->next;
		asm volatile("mfence");//make it visible to readers
	}
	else
	{
		while(cursor != NULL){
			if ( cursor->next != NULL && cursor->next->key == key){
				remove_element = cursor->next;
				cursor->next = cursor->next->next;
				asm volatile("mfence");
				break;
			}
		}
	}
	
	update_epoch = l -> epoch ^ 0x1; // may be synchro
	last_epoch = __atomic_exchange_n(&l -> epoch, update_epoch, __ATOMIC_SEQ_CST);
	
	while (l -> reads[l -> epoch]);
	
	l->reads[last_epoch] = 0;
remove_fun_retry_write_unlock:
	if (!__sync_val_compare_and_swap(&l->write_lock, 1, 0)) goto remove_fun_retry_write_unlock;	
	
	if (!remove_element){
		return 1;
	}
	
	free(remove_element);
	return 0;
}
