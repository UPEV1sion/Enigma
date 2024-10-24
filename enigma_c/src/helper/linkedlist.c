#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"
#include "helper.h"

//
// Created by Emanuel on 07.09.2024.
//

typedef struct LLNode
{
    struct LLNode *next;
    struct LLNode *prev;
    char payload[];
} LLNode;

struct LinkedList
{
    size_t length;
    size_t node_size;
    size_t value_size;

    LLNode *head;
    LLNode *tail;

    LLNode *tombstone_head;
};

static inline void push_tomb_list(LinkedList ll, LLNode *node)
{
    node->next = ll->tombstone_head;
    if (ll->tombstone_head != NULL)
        ll->tombstone_head->prev = node;
    ll->tombstone_head = node;
}

static LLNode* create_node(LinkedList ll, const void *payload, LLNode *next, LLNode *prev)
{
    // Benchmarking needed
    LLNode *node = malloc(ll->node_size);
    assertmsg(node != NULL, "malloc failed");
    node->next = next;
    node->prev = prev;
    memcpy(node->payload, payload, ll->value_size);
    return node;
}

LinkedList ll_create(const size_t value_size)
{
    LinkedList ll = malloc(sizeof(*ll));
    assertmsg(ll != NULL, "malloc failed");
    ll->length     = 0;
    ll->node_size  = sizeof(LLNode) + value_size;
    ll->value_size = value_size;
    ll->head = ll->tail = NULL;
    ll->tombstone_head = NULL;

    return ll;
}

int ll_push(LinkedList ll, const void *data)
{
    LLNode *node = create_node(ll, data, ll->head, NULL);
    if (ll->head != NULL)
        ll->head->prev = node;
    else
        ll->tail = node;
    ll->head = node;
    ll->length++;

    return 0;
}

int ll_add(LinkedList ll, const void *data)
{
    LLNode *node = create_node(ll, data, NULL, ll->tail);
    if (ll->tail != NULL)
        ll->tail->next = node;
    else
        ll->head = node;
    ll->tail = node;
    ll->length++;
    return 0;
}

int ll_offer(LinkedList ll, const void *data)
{
    return ll_add(ll, data);
}

void* ll_pop(LinkedList ll)
{
    assertmsg(ll->head != NULL, "ll head == NULL");
    LLNode *old_head = ll->head;
    ll->head = old_head->next;
    if (ll->head != NULL)
        ll->head->prev = NULL;
    else
        ll->tail = NULL;
    push_tomb_list(ll, old_head);
    ll->length--;

    return old_head->payload;
}

void* ll_poll(LinkedList ll)
{
    if(ll->head == NULL) return NULL;
    LLNode *old_head = ll->head;
    ll->head = old_head->next;
    if (ll->head != NULL)
        ll->head->prev = NULL;
    else
        ll->tail = NULL;
    push_tomb_list(ll, old_head);
    ll->length--;

    return old_head->payload;
}

void* ll_poll_last(LinkedList ll)
{
    if(ll->tail == NULL) return NULL;
    LLNode *old_tail = ll->tail;
    ll->tail = old_tail->prev;
    if (ll->tail != NULL)
        ll->tail->next = NULL;
    else
        ll->head = NULL;
    push_tomb_list(ll, old_tail);
    ll->length--;

    return old_tail->payload;
}

int ll_remove(LinkedList ll, size_t index)
{
    if (index >= ll->length) return 1;
    LLNode *node = ll->head;
    while (node && index--)
        node = node->next;

    if (!node) return 1;

    if (node->prev)
        node->prev->next = node->next;
    else
        ll->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        ll->tail = node->prev;

    ll->length--;
    return 0;
}

void* ll_peek(LinkedList ll)
{
    return ll->head != NULL ? ll->head->payload : NULL;
}

void *ll_peek_last(LinkedList ll)
{
    return ll->tail != NULL ? ll->tail->payload : NULL;
}

void* ll_get(LinkedList ll, const size_t index)
{
    if (index >= ll->length) return NULL;
    size_t counter = 0;
    for(LLNode *temp = ll->head; temp != NULL; temp = temp->next)
    {
        if (counter++ == index) return temp->payload;
    }

    return NULL;
}

size_t ll_length(LinkedList ll)
{
    return ll->length;
}

bool ll_is_empty(LinkedList ll)
{
    return ll->length == 0;
}

bool ll_contains(LinkedList ll, const void *data)
{
    for (LLNode *node = ll->head; node != NULL; node = node->next)
    {
        if(memcmp(node->payload, data, ll->value_size) == 0) return true;
    }

    return false;
}


int ll_destroy(LinkedList ll)
{
    if (ll == NULL) return 1;

    for (LLNode *current = ll->head; current != NULL; )
    {
        LLNode *next = current->next;
        free(current);
        current = next;
    }

    for (LLNode *current = ll->tombstone_head; current != NULL; )
    {
        LLNode *next = current->next;
        free(current);
        current = next;
    }

    free(ll);
    return 0;
}
