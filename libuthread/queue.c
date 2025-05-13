#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>  //for malloc()

#include "queue.h"

/*
queue_t = "struct queue*" (avoids having to type both things every time)
**it's a pointer to a struct queue**
*/

struct node {
    void* data;         //pointer for stored data (void because we don't know what kind of data it is)
    struct node* next;  //pointer to the next node in the queue
};

struct queue {
	int size;		   //to keep track of the size of it
    struct node *head; //the head of the queue
    struct node *tail; //the tail of the queue
};

queue_t queue_create(void)
{
	queue_t Queue = (queue_t) malloc(sizeof(struct queue)); //allocate space on the heap, otherwise it goes out of scope
	if(Queue == NULL){ return NULL; } //error with malloc

	Queue->size = 0; 
	Queue->head = NULL;
	Queue->tail = NULL;

	return Queue;
}

int queue_destroy(queue_t queue)
{
	if(queue == NULL || queue->size != 0){
		return -1;
	}

	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if(queue == NULL || data == NULL){
		return -1;
	}

	struct node* new_node = (struct node*) malloc(sizeof(struct node)); //add new node to heap
	if(new_node == NULL){
		return -1;
	}

	new_node->data = data;
	new_node->next = NULL;

	if(queue->size == 0){
		queue->head = new_node;
	}
	else{
		queue->tail->next = new_node;
	}
	queue->tail = new_node;

	queue->size++;

	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if(queue == NULL || data == NULL || queue->size == 0){
		return -1;
	}

	*data = queue->head->data;

	struct node* temp = queue->head; //in order to free it later
	queue->head = queue->head->next;

	queue->size--;
	if(queue->size == 0){
		queue->head = NULL;
		queue->tail = NULL;
	}

	free(temp);

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if(queue == NULL || data == NULL || queue->size == 0){
		return -1;
	}

	struct node* iterator = queue->head;
	struct node* previous = NULL;

	while(iterator != NULL){ //while there are still nodes

		if(iterator->data == data){

			if(iterator == queue->head){ //if deleting the head node
				queue->head = iterator->next;
				if(queue->head == NULL){ //if queue is now empty
					queue->tail = NULL;
				}
			}
			else{ 
				previous->next = iterator->next;
				if(iterator == queue->tail){ //if deleting the tail
					queue->tail = previous;
				}
			}

			free(iterator);
			queue->size--;
			return 0;

		}

		previous = iterator;
		iterator = iterator->next;
	}

	return -1;

}

int queue_iterate(queue_t queue, queue_func_t func)
{	
	if(queue == NULL || func == NULL){
		return -1;
	}

	struct node* iterator = queue->head;

	while(iterator != NULL){ //while there are still nodes
		struct node* next = iterator->next; //in case iterator gets deleted, we still have access to the next node
		func(queue, iterator->data);
		iterator = next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if(queue == NULL){
		return -1;
	}

	return queue->size;
}

/*Sources: 
https://stackoverflow.com/questions/14768230/malloc-for-struct-and-pointer-in-c //wanted to see an implementation of malloc for struct pointers
https://stackoverflow.com/questions/1591361/understanding-typedefs-for-function-pointers-in-c //I wanted to understand typedefs for function pointers
*/