#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef ASCII_SIZE
#define ASCII_SIZE 256
#endif
#ifndef ALPHABET_SIZE
#define ALPHABET_SIZE 26
#endif

/**
* @brief assert function with an error message.
* @note do-while for correct indentation
* @param cond the condition to be asserted
* @param msg the output when condition isn't fulfilled
*/
#define assertmsg(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "%s at function %s, file %s, line %d.\n", \
                    msg, __func__, __FILE__, __LINE__); \
        exit(1); \
    } \
} while (0)

/**
* @brief This a branching hint macro for CPU pipelining, but MSVC doesn't currently support this.
* @note !! to convert to a strict boolean value.
* @param cond the condition for which the hint should take place
*/
#if defined(__clang__) || defined(__GNUC__)
#define expected_true(cond) __builtin_expect(!!(cond), 1)
#define expected_false(cond) __builtin_expect(!!(cond), 0)
#else
#define expected_true(cond) (cond)
#define expected_false(cond) (cond)
#endif

// A bit hack for NaN because including without including the math library.
// IEEE 745-1985 NaN coding: s = 0||1, exponent = 11111111111, mantissa != 0
// (uint64_t){0x7FFFFFFFFFFFFFFF} is a compound literal and the {} is necessary
// ...Turns out this can be done way easier with 0.0/0.0 but this is a cool bit hack, so I leave it here
#define NaN (*(double*)&((uint64_t){0x7FFFFFFFFFFFFFFF}))


int32_t get_number_from_string(const char *str, int32_t *number);
int32_t to_uppercase(char *input);
int32_t remove_non_alnum(char *input);
int32_t remove_non_alpha(char *input);
bool is_permutation(const char *first, const char *second);
uint8_t* get_int_array_from_string(const char *str);
char* get_string_from_int_array(const uint8_t *array, size_t size);
bool has_duplicates(const char *str);
bool contains_spaces(const char *str);
size_t count_alphas(const char *str);
size_t count_c(const char *str, char c);
bool is_substring(const char *str1, const char *str2);
double calc_index_of_coincidence(const uint8_t *arr, size_t len);
size_t my_getline(char *str, size_t lim);
