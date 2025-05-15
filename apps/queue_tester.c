#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "queue.h"
#include "uthread.h"

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
    int data = 420, *ptr;

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data);
}


void test_queue_length(void) {
    fprintf(stderr, "*** TEST test_queue_length *** \n");
    queue_t q = queue_create();
    TEST_ASSERT(queue_length(q) == 0);

    int data1 = 100, data2 = 200, data3 = 300;
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
    int data1 = 5, data2 = 10, data3 = 15;
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
    int data = 57;
    queue_enqueue(q2, &data);
    TEST_ASSERT(queue_destroy(q2) == -1); // Should fail since queue is not empty
}

void test_queue_multiple_enqueue_dequeue(void) {
    fprintf(stderr, "*** TEST test_queue_multiple_enqueue_dequeue *** \n");
    queue_t q = queue_create();
    int data1 = 420, data2 = 69, data3 = 333;
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

    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

void test_queue_null_handling(void) {
    fprintf(stderr, "*** TEST test_queue_null_handling *** \n");
    queue_t q = queue_create();
    int *ptr;

    TEST_ASSERT(queue_enqueue(NULL, &ptr) == -1);
    TEST_ASSERT(queue_dequeue(NULL, (void**)&ptr) == -1);
    TEST_ASSERT(queue_delete(NULL, &ptr) == -1);
    TEST_ASSERT(queue_length(NULL) == -1);

    queue_destroy(q);
}

void increment_iterate(queue_t queue, void *data) {
    (void)queue;//we dont use the queue
    (*(int *)data)++;
}

void delete_middle_iterate(queue_t queue, void *data) {
    if (*(int *)data == 2) { //delete the element with value 2
        queue_delete(queue, data);
    }
}

void test_queue_iterate_increment(void) {
    fprintf(stderr, "*** TEST queue_iterate_increment ***\n");
    queue_t q = queue_create();
    int data1 = 1, data2 = 2, data3 = 3;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_iterate(q, increment_iterate);

    TEST_ASSERT(data1 == 2);
    TEST_ASSERT(data2 == 3);
    TEST_ASSERT(data3 == 4);

    queue_destroy(q);
}

void test_queue_iterate_deletion(void) {
    fprintf(stderr, "*** TEST queue_iterate_deletion ***\n");
    queue_t q = queue_create();
    int data1 = 1, data2 = 2, data3 = 3;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_iterate(q, delete_middle_iterate);

    TEST_ASSERT(queue_length(q) == 2);
    TEST_ASSERT(queue_delete(q, &data1) == 0);
    TEST_ASSERT(queue_delete(q, &data3) == 0);

    queue_destroy(q);
}

int main(void){
    test_create();
    test_queue_simple();
    test_queue_length();
    test_queue_order();
    test_queue_delete();
    test_queue_delete_middle();
    test_queue_destroy();
    test_queue_multiple_enqueue_dequeue();
    test_queue_null_handling();
    test_queue_iterate_increment();
    test_queue_iterate_deletion();

    return 0;
}