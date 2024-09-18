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
* @brief assert function with an error message
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
* @brief This a branching hint macro for CPU pipelining but MSVC does not currently support this
* @param cond the condition for which the hint should take place
*/
#if defined(__clang__) || defined(__GNUC__)
#define expected_true(cond) __builtin_expect(!!(cond), 1)
#define expected_false(cond) __builtin_expect(!!(cond), 0)
#else
#define expected_true(cond) (cond)
#define expected_false(cond) (cond)
#endif

int32_t get_number_from_string(const char *str, int32_t *number);
int32_t to_upper_case(char *input);
int32_t remove_none_alnum(char *input);
int32_t remove_none_alpha(char *input);
bool is_permutation(const char *first, const char *second);
uint8_t* get_int_array_from_string(const char *str);
char* get_string_from_int_array(const uint8_t *array, size_t size);
bool has_duplicates(const char *str);
bool contains_spaces(const char *str);
int32_t count_alphas(const char *str);
int32_t count(const char *str, char c);
bool is_substring(const char *str1, const char *str2);
double calc_index_of_coincidence(const uint8_t *str, size_t len);
size_t mygetline(char *str, size_t lim);
