#include <stdlib.h>
#include <string.h>

#include "hashmap.h"
#include "helper.h"


//
// Created by Emanuel on 10.10.24.
//

// TODO test

#define MIN_CAPACITY    (1 << 4)
#define MAX_CAPACITY    (1 << 30)
#define SCALING_FACTOR  2
#define LOAD_FACTOR     0.75

#define ERR_TOO_BIG     1
#define ERR_NULL_PTR    2

typedef struct Bucket Bucket;

struct Bucket
{
    char *key;
    char *val;
    size_t num_buckets_left;

    Bucket *next;
};

struct HashMap
{
    size_t size;
    size_t capacity;

    Bucket **buckets;
};

static size_t hash(const char *key)
{
    size_t hash_val = 5381;

    while(*key != 0)
        hash_val = (hash_val << 5) + hash_val + *key++;

    return hash_val;
}

static inline double calc_load_factor(HashMap hm)
{
    return (double) hm->size / (double) hm->capacity;
}

static int32_t hm_resize(HashMap hm)
{
    const size_t new_capacity = hm->capacity * SCALING_FACTOR;
    if(new_capacity >= MAX_CAPACITY) return ERR_TOO_BIG;

    Bucket **new_buckets = calloc(new_capacity, sizeof (Bucket*));
    assertmsg(new_buckets!= NULL, "calloc failed");

    for(size_t i = 0; i < hm->capacity; ++i)
    {
        Bucket *bucket = hm->buckets[i];
        hm->buckets[i] = NULL;

        while(bucket != NULL)
        {
            Bucket *next = bucket->next;
            const size_t new_hash = hash(bucket->key) % new_capacity;

            bucket->next = new_buckets[new_hash];
            new_buckets[new_hash] = bucket;
            bucket = next;
        }
    }

    hm->buckets = new_buckets;
    hm->capacity = new_capacity;
    return 0;
}

static Bucket* create_bucket(char *key, char *val)
{
    Bucket *bucket = malloc(sizeof (Bucket));
    assertmsg(bucket != NULL, "malloc failed");
    bucket->key = key;
    bucket->val = val;
    bucket->num_buckets_left = 0;

    return bucket;
}

HashMap hm_create(size_t size)
{
    HashMap hm = malloc(sizeof(*hm));
    assertmsg(hm != NULL, "malloc failed");
    hm->capacity = (size < MIN_CAPACITY) ? MIN_CAPACITY : size;
    hm->buckets  = calloc(hm->capacity, sizeof(Bucket*));
    hm->size     = 0;

    return hm;
}

static int32_t hm_destroy_list(HashMap hm, size_t i)
{
    Bucket *bucket = hm->buckets[i];
    if(bucket == NULL) return ERR_NULL_PTR;
    for(Bucket *temp = bucket; temp != NULL;)
    {
        Bucket *next_bucket = temp->next;
        free(temp);
        temp = next_bucket;
    }

    return 0;
}

int32_t hm_destroy(HashMap hm)
{
    if(hm == NULL) return ERR_NULL_PTR;
    for(size_t i = 0; i < hm->capacity; ++i)
    {
        (void) hm_destroy_list(hm, i);
    }
    free(hm->buckets);
    free(hm);

    return 0;
}

int32_t vl_destroy(ValueList *vl)
{
    if(vl == NULL) return ERR_NULL_PTR;
    free(vl->values);
    free(vl);

    return 0;
}

int32_t hm_put(HashMap hm, char *key, char *val)
{
    if(hm == NULL)
    {
        return ERR_NULL_PTR;
    }
    int32_t err_code = 0;
    if(calc_load_factor(hm) >= LOAD_FACTOR)
    {
        err_code = hm_resize(hm);
    }

    const size_t hash_val = hash(key) % hm->capacity;
    Bucket *cur_bucket = hm->buckets[hash_val];

    Bucket *bucket = create_bucket(key, val);
    bucket->next = cur_bucket;
    if(cur_bucket != NULL)
    {
        bucket->num_buckets_left = cur_bucket->num_buckets_left + 1;
    }
    else
    {
        bucket->num_buckets_left = 0;
    }
    hm->buckets[hash_val] = bucket;

    hm->size++;

    return err_code;
}

ValueList* hm_get(HashMap hm, char *key)
{
    const size_t hash_val = hash(key) % hm->capacity;
    Bucket *bucket = hm->buckets[hash_val];
    if(bucket == NULL) return NULL;

    ValueList *value_list = malloc(sizeof (ValueList));
    value_list->values = malloc(sizeof(char *) * (bucket->num_buckets_left + 1));
    value_list->list_size = bucket->num_buckets_left + 1;

    for(size_t i = 0; bucket != NULL && bucket->num_buckets_left >= 0; ++i)
    {
        value_list->values[i] = bucket->val;
        bucket = bucket->next;
    }

    return value_list;
}

int32_t hm_remove(HashMap hm, char *key)
{
    size_t hash_val = hash(key) % hm->capacity;
    Bucket *bucket = hm->buckets[hash_val];
    if(hm_destroy_list(hm, hash_val) == 0)
    {
        hm->size -= bucket->num_buckets_left;
        return 0;
    }

    return ERR_NULL_PTR;
}

size_t hm_size(HashMap hm)
{
    return hm->size;
}

bool hm_contains(HashMap hm, char *key)
{
    Bucket *bucket = hm->buckets[hash(key) % hm->capacity];
    while (bucket != NULL) {
        if (strcmp(bucket->key, key) == 0) {
            return true;
        }
        bucket = bucket->next;
    }
    return false;
}

