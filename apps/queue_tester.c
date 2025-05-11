#include <stdio.h>
#include <assert.h>
#include "queue.h"

void test_create(void)
{
    queue_t q;

    q = queue_create();
    assert(q != NULL);
    printf("Created queue\n");
}

void test_queue_simple(void)
{
    queue_t q;
    int data = 3, *ptr;

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data);
    printf("Test enqueue/dequeue passed\n");
}

void test_queue_length(void) {
    queue_t q = queue_create();
    assert(queue_length(q) == 0);

    int data1 = 10, data2 = 20, data3 = 30;
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    assert(queue_length(q) == 3);
    printf("test queue_length passed!\n");
}

void test_queue_order(void) {
    queue_t q = queue_create();
    int data1 = 1, data2 = 2, data3 = 3;
    int *ptr;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data1);

    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data2);

    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data3);

    printf("test queue_order passed!\n");
}

void test_queue_delete(void) {
    queue_t q = queue_create();
    int data1 = 5, data2 = 10, data3 = 15;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    assert(queue_length(q) == 3);
    queue_delete(q, &data2);
    assert(queue_length(q) == 2);

    queue_delete(q, &data1);
    assert(queue_length(q) == 1);

    queue_delete(q, &data3);
    assert(queue_length(q) == 0);

    printf("test queue_delete passed!\n");
}

void test_queue_middle_deletion(void) {
    queue_t q = queue_create();
    int data1 = 100, data2 = 200, data3 = 300;
    int *ptr;

    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);

    queue_delete(q, &data2); // Removing middle node

    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data1); // Head should still point to data1
    queue_dequeue(q, (void**)&ptr);
    assert(ptr == &data3); // Next node should now point to data3

    printf("test queue_middle_deletion passed!\n");
}

void test_queue_destroy(void) {
    queue_t q = queue_create();
    assert(queue_destroy(q) == 0); // Queue should be successfully destroyed

    queue_t q2 = queue_create();
    int data = 42;
    queue_enqueue(q2, &data);
    assert(queue_destroy(q2) == -1); // Should fail since queue is not empty

    printf("test queue_destroy passed!\n");
}

int main(void) {
    test_create();
    test_queue_simple();
    test_queue_length();
    test_queue_order();
    test_queue_delete();
    test_queue_middle_deletion();
    test_queue_destroy();

    printf("All tests passed!\n");
    return 0;
}