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
    BombeNode *bombe_node;
} LLNode;

struct LinkedList
{
    size_t length;

    LLNode *head;
    LLNode *tail;
};

static LLNode* create_node(BombeNode *bombe_node, LLNode *next, LLNode *prev)
{
    // Benchmarking needed
    LLNode *node = malloc(sizeof(LLNode));
    assertmsg(node != NULL, "malloc failed");
    node->next = next;
    node->prev = prev;
    node->bombe_node = bombe_node;
    return node;
}

LinkedList ll_create(void)
{
    LinkedList ll = malloc(sizeof(*ll));
    assertmsg(ll != NULL, "malloc failed");
    ll->length = 0;
    ll->head = ll->tail = NULL;

    return ll;
}

int ll_push(LinkedList ll, BombeNode *data)
{
    LLNode *node = create_node(data, ll->head, NULL);
    if(ll->head == NULL && ll->tail == NULL)
    {
        ll->head = ll->tail = node;
    }
    else
    {
        ll->head->prev = node;
        node->next = ll->head;
        ll->head = node;
    }
    ll->length++;

    return 0;
}

int ll_add(LinkedList ll, BombeNode *data)
{
    LLNode *node = create_node(data, NULL, ll->tail);
    if(ll->head == NULL && ll->tail == NULL)
    {
        ll->head = ll->tail = NULL;
    }
    else
    {
        ll->tail->next = node;
        node->prev = ll->tail;
        ll->tail = node;
    }
    ll->length++;
    return 0;
}

int ll_offer(LinkedList ll, BombeNode *data)
{
    return ll_add(ll, data);
}

BombeNode* ll_pop(LinkedList ll)
{
    assertmsg(ll->head != NULL, "ll head == NULL");
    LLNode *old_head = ll->head;
    BombeNode *node = old_head->bombe_node;
    ll->head = old_head->next;
    if (ll->head != NULL)
        ll->head->prev = NULL;
    else
        ll->tail = NULL;
    ll->length--;
    free(old_head);

    return node;
}

BombeNode* ll_poll(LinkedList ll)
{
    if(ll->head == NULL) return NULL;
    LLNode *old_head = ll->head;
    BombeNode *node = old_head->bombe_node;
    ll->head = old_head->next;
    if (ll->head != NULL)
        ll->head->prev = NULL;
    else
        ll->tail = NULL;
    ll->length--;
    free(old_head);

    return node;
}

BombeNode* ll_poll_last(LinkedList ll)
{
    if(ll->tail == NULL) return NULL;
    LLNode *old_tail = ll->tail;
    ll->tail = old_tail->prev;
    BombeNode *node = old_tail->bombe_node;
    if (ll->tail != NULL)
        ll->tail->next = NULL;
    else
        ll->head = NULL;
    ll->length--;
    free(old_tail);

    return node;
}

int32_t ll_remove(LinkedList ll, size_t index)
{
    if (index >= ll->length) return 1;
    LLNode *node = ll->head;
    while (node && index--)
        node = node->next;

    if (node == NULL) return 1;

    if (node->prev)
        node->prev->next = node->next;
    else
        ll->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        ll->tail = node->prev;

    ll->length--;
    free(node);

    return 0;
}

BombeNode* ll_peek(LinkedList ll)
{
    return ll->head != NULL ? ll->head->bombe_node : NULL;
}

BombeNode *ll_peek_last(LinkedList ll)
{
    return ll->tail != NULL ? ll->tail->bombe_node : NULL;
}

BombeNode* ll_get(LinkedList ll, const size_t index)
{
    if (index >= ll->length) return NULL;
    size_t counter = 0;
    for(LLNode *temp = ll->head; temp != NULL; temp = temp->next)
    {
        if (counter++ == index) return temp->bombe_node;
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

bool ll_contains(LinkedList ll, const BombeNode *data)
{
    for (LLNode *node = ll->head; node != NULL; node = node->next)
    {
        if(node->bombe_node == data) return true;
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

    free(ll);
    return 0;
}
