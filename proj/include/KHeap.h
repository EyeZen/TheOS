#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct KHeapNode {
    uint64_t size;
    bool free;
    struct KHeapNode* prev;
    struct KHeapNode* next;
};

extern struct KHeapNode* kheap_start;
extern struct KHeapNode* kheap_end;
extern struct KHeapNode* kheap_curr;

#define NODE_SIZE(size) (size + sizeof(struct KHeapNode))

#define KHEAP_ALIGNMENT 0x10
#define KHEAP_MIN_ALLOC_SIZE 0x20   // every node contains atleast 32 bytes

#define MERGE_RIGHT 0b01
#define MERGE_LEFT  0b10
#define MERGE_BOTH  0b11
#define MERGE_NONE  0b00

void kheap_init();

size_t align(size_t size);

struct KHeapNode* create_kheap_node(struct KHeapNode* node, size_t size);

uint64_t compute_kheap_end();

uint8_t can_merge(struct KHeapNode* cur_node);

void merge_kheap_nodes(struct KHeapNode* left, struct KHeapNode* right);

void expand_heap(size_t required_size);

void* kmalloc(size_t size);

void kfree(void *ptr);

#endif