#include "KHeap.h"
#include "MEM.h"
#include "VMM.h"
#include "print.h"
#include "Logging.h"

struct KHeapNode* kheap_start;
struct KHeapNode* kheap_end;
struct KHeapNode* kheap_curr;

extern unsigned int _kernel_end;
extern uint64_t end_of_mapped_memory;

void kheap_init(){
    // leave 1 Page-Size padding space between kernel and heap
    kheap_start = (struct KHeapNode*) ((uint64_t)&_kernel_end + PAGE_SIZE);
    kheap_curr = kheap_start;
    kheap_end = kheap_start;
    kheap_curr->size = 0x1000;
    kheap_curr->free = true;
    kheap_curr->next = NULL;
    kheap_curr->prev = NULL;
}

size_t align(size_t size) {
    return (size / KHEAP_ALIGNMENT + 1) * KHEAP_ALIGNMENT;
}

struct KHeapNode* create_kheap_node(struct KHeapNode* node, size_t size)
{
    struct KHeapNode* new_node = (struct KHeapNode*)((uint64_t)node + NODE_SIZE(size));

    new_node->free = true;
    new_node->size = node->size - NODE_SIZE(size);
    new_node->prev = node;
    new_node->next = node->next;

    if(node->next != NULL) {
        (node->next)->prev = new_node;
    }

    node->next = new_node;

    if(node == kheap_end) {
        kheap_end = new_node;
    }

    return new_node;
}

uint64_t compute_kheap_end() {
    return (uint64_t)kheap_end + NODE_SIZE(kheap_end->size);
}

uint8_t can_merge(struct KHeapNode* cur_node) {
    struct KHeapNode* prev_node = cur_node->prev;
    struct KHeapNode* next_node = cur_node->next;
    uint8_t available_merges = 0;
    if( prev_node != NULL && prev_node->free ) {
        uint64_t prev_address = (uint64_t) prev_node + sizeof(struct KHeapNode) + prev_node->size;
        if ( prev_address == (uint64_t) cur_node ) {
            available_merges = available_merges | MERGE_LEFT;
        }
    }
    if( next_node != NULL && next_node->free ) {
        uint64_t next_address = (uint64_t) cur_node + sizeof(struct KHeapNode) + cur_node->size;
        if ( next_address == (uint64_t) cur_node->next ) {
            available_merges = available_merges | MERGE_RIGHT;
        }

    }

    return available_merges;
}

void merge_kheap_nodes(struct KHeapNode* left, struct KHeapNode* right) {
    if(left == NULL || right == NULL)
        return;
    if(((uint64_t)left + left->size + sizeof(struct KHeapNode)) != (uint64_t)right)
        return;
    left->size += right->size + sizeof(struct KHeapNode);
    left->next = right->next;

    if(right->next != NULL) {
        (right->next)->prev = left;
    }
}

void expand_heap(size_t required_size) {
    size_t number_of_pages = required_size / PAGE_SIZE + 1;
    uint64_t heap_end = compute_kheap_end();
    if( heap_end > end_of_mapped_memory ) {
        kprintf("\nOOM");
        logf("\n : OOM");
        map_vaddress_range((uint64_t *) heap_end, 0, number_of_pages);
    }
    struct KHeapNode* new_tail = (struct KHeapNode*) heap_end;
    new_tail->next = NULL;
    new_tail->prev = kheap_end;
    new_tail->size = PAGE_SIZE * number_of_pages;
    new_tail->free = true;
    kheap_end->next = new_tail;
    // kheap_end = new_tail;
    uint8_t available_merges = can_merge(new_tail);
    if ( available_merges & MERGE_LEFT) {
        merge_kheap_nodes(new_tail->prev, new_tail);
        return;
    }
    kheap_end = new_tail;
}

void *kmalloc(size_t size) {
    struct KHeapNode* current_node = kheap_start;
    if( size == 0 )
        return NULL;
    while( current_node != NULL ) {
        size_t bytes_required = NODE_SIZE(size);
        bytes_required = align(bytes_required);
        if( current_node->free && current_node->size >= bytes_required) {
            uint64_t space_left_after_alloc = current_node->size - bytes_required;
            if( space_left_after_alloc > KHEAP_MIN_ALLOC_SIZE ) {
                create_kheap_node(current_node, bytes_required);
                current_node->size = bytes_required;
            }
            // use complete node
            current_node->free = false;

            return (void *) ((uint64_t)current_node + sizeof(struct KHeapNode));
        }
        // log_astr("reahed heap end: ");
        
        if( current_node == kheap_end ) {
            expand_heap(bytes_required);
            if( current_node->prev != NULL) {
                current_node = current_node->prev;
            }
        }
        current_node = current_node->next;
    }
    return NULL;
}

void kfree(void *ptr) {
    // Before doing anything let's check that the address provided is valid: not null, and within the heap space
    if(ptr == NULL) {
        return;
    }

    if ( (uint64_t) ptr < (uint64_t) kheap_start || (uint64_t) ptr > (uint64_t) kheap_end) {
        return;
    }

    // Now we can search for the node containing our address
    struct KHeapNode* current_node = kheap_start;
    while( current_node != NULL ) {
        if( ((uint64_t) current_node + sizeof(struct KHeapNode)) == (uint64_t) ptr) {
            current_node->free = true;
            uint8_t available_merges = can_merge(current_node);
 
            if( available_merges & MERGE_RIGHT ) {
                merge_kheap_nodes(current_node, current_node->next);
            }
           
            if( available_merges & MERGE_LEFT ) {
                merge_kheap_nodes(current_node->prev, current_node);
            }
            return;
            
        }
        current_node = current_node->next;
    }
}
