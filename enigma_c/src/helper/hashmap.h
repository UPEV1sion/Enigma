//
// Created by Emanuel on 02.09.2024.
//

// This HashMap is available as a standalone at my own GitHub Account: https://github.com/UPEV1sion/HashMap_C

#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct HashMap *HashMap;
typedef size_t (*hash)(const void *restrict key, size_t key_size);

/**
 * @brief Creates a new HashMap with a hash function.
 *
 * @param hm_capacity The initial capacity of the HashMap
 * @param key_size The sizeof value of the key
 * @param value_size The sizeof value of the value
 * @param hash_func a custom hash function. Pass NULL for generic hashing.
 * @return HashMap
 */
HashMap hm_create(size_t hm_capacity, size_t key_size, size_t value_size, hash hash_func);

/**
 * @brief Destroys the HashMap.
 *
 * @param hm The HashMap
 * @return Success code
 */
int hm_destroy(HashMap hm);

/**
 * @brief Retrieves the value at the specified key.
 *
 * @param hm The HashMap
 * @param key The key
 * @return The value at the key
 */
void *hm_get(HashMap restrict hm, const void *restrict key);

/**
 * @brief Updates the value at the specified key.
 *
 * @param hm The HashMap
 * @param key The key
 * @param value The value
 * @return Success code
 */
int hm_set(HashMap restrict hm, const void *restrict key, const void *restrict value);

/**
 * @brief Adds a new key-value pair to the HashMap.
 *
 * @param hm The HashMap
 * @param key The key
 * @param value The value
 * @return Success code
 */
int hm_put(HashMap restrict hm, const void *restrict key, const void *restrict value);


/**
 * @brief Tests if the HashMap contains the specified key.
 *
 * @param hm The HashMap
 * @param key The key
 * @return True or Falsehood
 */
bool hm_contains(HashMap restrict hm, const void *restrict key);

/**
 * @brief Returns the size of the HashMap.
 *
 * @param hm The HashMap
 * @return The size
 */
size_t hm_size(HashMap restrict hm);

/**
 * @brief Removes the key-value pair from the HashMap.
 *
 * @param hm The HashMap
 * @param key The key
 * @return Success code
 */
int hm_remove(HashMap restrict hm, const void *restrict key);
