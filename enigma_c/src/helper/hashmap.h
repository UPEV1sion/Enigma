#pragma once

//
// Created by Emanuel on 10.10.24.
//

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    char **values;
    size_t list_size;
} ValueList;

typedef struct HashMap *HashMap;

HashMap hm_create(size_t size);
int32_t hm_destroy(HashMap hm);
int32_t hm_put(HashMap hm, char *key, char *val);
ValueList* hm_get(HashMap hm, char *key);
int32_t hm_remove(HashMap hm, char *key);
size_t hm_size(HashMap hm);
bool hm_contains(HashMap hm, char *key);

