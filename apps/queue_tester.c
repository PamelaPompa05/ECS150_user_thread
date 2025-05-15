#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "queue.h"


#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

void test_queue_simple(void)
{
    fprintf(stderr, "*** TEST test_queue_simple *** \n");
    queue_t q;
    int data = 3, *ptr;

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data);
}


void test_queue_length(void) {
    fprintf(stderr, "*** TEST test_queue_length *** \n");
    queue_t q = queue_create();
    TEST_ASSERT(queue_length(q) == 0);

    int data1 = 10, data2 = 20, data3 = 30;
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    TEST_ASSERT(queue_length(q) == 3);
}

void test_queue_order(void) {
    fprintf(stderr, "*** TEST test_queue_order *** \n");
    queue_t q = queue_create();
    int data1 = 1, data2 = 2, data3 = 3;
    int *ptr;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data1);

    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data2);

    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data3);
}

void test_queue_delete(void) {
    fprintf(stderr, "*** TEST test_queue_delete *** \n");
    queue_t q = queue_create();
    int data1 = 5, data2 = 10, data3 = 15;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    TEST_ASSERT(queue_length(q) == 3);
    queue_delete(q, &data2);
    TEST_ASSERT(queue_length(q) == 2);

    queue_delete(q, &data1);
    TEST_ASSERT(queue_length(q) == 1);

    queue_delete(q, &data3);
    TEST_ASSERT(queue_length(q) == 0);
}

void test_queue_delete_middle(void) {
    fprintf(stderr, "*** TEST test_queue_delete_middle *** \n");
    queue_t q = queue_create();
    int data1 = 100, data2 = 200, data3 = 300;
    int *ptr;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_delete(q, &data2); // Removing middle node

    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data1); // Head should still point to data1
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data3); // Next node should now point to data3
}

void test_queue_destroy(void) {
    fprintf(stderr, "*** TEST test_queue_destroy *** \n");
    queue_t q = queue_create();
    TEST_ASSERT(queue_destroy(q) == 0); // Queue should be successfully destroyed

    queue_t q2 = queue_create();
    int data = 42;
    queue_enqueue(q2, &data);
    TEST_ASSERT(queue_destroy(q2) == -1); // Should fail since queue is not empty
}

int main(void){
    test_create();
    test_queue_simple();
    test_queue_length();
    test_queue_order();
    test_queue_delete();
    test_queue_delete_middle();
    test_queue_destroy();
    
    return 0;
}