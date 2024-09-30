#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "helper.h"


// I don't know why, but on my Linux WSL this is undefined
#ifndef ERANGE
#define ERANGE 34
#endif

/**
 * @brief The error codes - mainly for debugging.
 * @note In the code, the function return values are only asserted to be == 0, indicating no error.
 */
#define ERR_NULL_POINTER 1
//Also empty uint8_t array
#define ERR_EMPTY_STRING 2

#define ERR_INVALID_INPUT 3
#define ERR_OUT_OF_RANGE 4
#define ERR_PARTIAL_CONVERSION 5

/**
 * @brief Gets the number literal from a string
 * @param str the string containing the number literal
 * @param number the number to be extracted
 * @return int32_t: error code
 */
int32_t get_number_from_string(const char *str, int32_t *number)
{
    if (str == NULL) return ERR_NULL_POINTER;
    if (strlen(str) == 0) return ERR_EMPTY_STRING;
    char *endptr;

    while (isspace(*str)) str++;
    size_t len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) len--;
    if (len == 0) return ERR_EMPTY_STRING;

    errno          = 0;
    const long res = strtol(str, &endptr, 10);

    if (endptr == str)
    {
        return ERR_INVALID_INPUT;
    }

    if (errno == ERANGE || res < INT32_MIN || res > INT32_MAX)
    {
        return ERR_OUT_OF_RANGE;
    }

    if (*endptr != 0)
    {
        printf("Invalid input %s. Chars not converted: %s\n", str, endptr);
        return ERR_PARTIAL_CONVERSION;
    }

    *number = (int32_t) res;
    return 0;
}

/**
 * @brief Capitalizes the string
 * @param input the string to be capitalized
 * @return int32_t: error code
 */
int32_t to_uppercase(char *input)
{
    if (input == NULL) return ERR_NULL_POINTER;
    const size_t length = strlen(input);
    if (length == 0) return ERR_EMPTY_STRING;


    for (size_t i = 0; i < length; ++i)
    {
        input[i] = (char) toupper(input[i]);
    }

    return 0;
}

/**
 * @brief Removes all non-alphabetic and numeric chars from a string
 * @param input the string where the alnums chars should be removed
 * @return int32_t: error code
 */
int32_t remove_non_alnum(char *input)
{
    if (input == NULL) return ERR_NULL_POINTER;
    if (strlen(input) == 0) return ERR_EMPTY_STRING;
    int32_t i = 0;
    int32_t j = 0;

    while (input[i] != 0)
    {
        if (isalnum(input[i]))
        {
            input[j] = input[i];
            j++;
        }
        i++;
    }
    input[j] = 0;

    return 0;
}

/**
 * @brief Removes all non-alphabetic chars
 * @param input the string where the none alphabetic chars should be removed
 * @return int32_t: error code
 */
int32_t remove_non_alpha(char *input)
{
    if (input == NULL) return ERR_NULL_POINTER;
    if (strlen(input) == 0) return ERR_EMPTY_STRING;
    int32_t i = 0;
    int32_t j = 0;

    while (input[i] != 0)
    {
        if (isalpha(input[i]))
        {
            input[j] = input[i];
            j++;
        }
        i++;
    }
    input[j] = 0;

    return 0;
}

/**
 * @brief Checks if second is a permutation of first
 * @note Only works with uppercase alphabetic strings
 * @param first first str
 * @param second second str
 * @return bool: true or falsehood
 */
bool is_permutation(const char *first, const char *second)
{
    /*  I'll leave this here for the interested reader:
     *  Instead of using a boolean or int array to toggle or count the letters,
     *  I used a bitmask where each bit corresponds to an uppercase letter.
     *  After the subtraction of 'A' we leftshift the one by n places, which is equivalent to multiplying it by 2^n
     *  We XOR this bit with the mask, which is effectively toggling it.
     *  If first and second contain the same chars, the number must be 0 again,
     *  so the final check is a straightforward == 0.
     */

    if (first == NULL || second == NULL) return false;
    const size_t len1 = strlen(first);
    const size_t len2 = strlen(second);

    if (len1 != len2) return false;

    uint32_t mask = 0;

    for (size_t i = 0; i < len1; i++)
    {
        mask ^= 1 << (first[i] - 'A');
        mask ^= 1 << (second[i] - 'A');
    }

    return mask == 0;
}

/**
 * @brief Subtracts 'A' from all chars
 * @note All letters must be uppercase
 * @param str the original string
 * @return uint8_t*: to the processed array, NULL for error
 */
uint8_t* get_int_array_from_string(const char *str)
{
    if (str == NULL) return NULL;
    const size_t len = strlen(str);
    if (len == 0) return NULL;
    uint8_t *array = malloc(len * sizeof(uint8_t));
    assertmsg(array != NULL, "array == NULL");

    for (size_t i = 0; i < len; i++)
    {
        array[i] = (uint8_t) (str[i] - 'A');
    }

    return array;
}

/**
 * @brief Adds 'A' to all chars
 * @param array the int array to be converted
 * @param size the array size
 * @return char*: to the processed string, NULL for error
 */
char* get_string_from_int_array(const uint8_t *array, const size_t size)
{
    if (array == NULL) return NULL;
    if (size == 0) return NULL;
    char *str = malloc(size + 1);
    assertmsg(str != NULL, "str == NULL");

    for (size_t i = 0; i < size; i++)
    {
        str[i] = (char) (array[i] + 'A');
    }
    str[size] = 0;

    return str;
}

/**
 * @brief Checks if a string has duplicate chars
 * @note Ignores spaces
 * @note Only works with uppercase letters.
 * @param str the string to be checked
 * @return bool: true or falsehood
 */
bool has_duplicates(const char *str)
{
    if (str == NULL) return false;
    size_t len;
    if ((len = strlen(str)) == 0) return false;

    bool visited[ALPHABET_SIZE] = {false};

    for (size_t i = 0; i < len; ++i)
    {
        const char c = str[i];
        if(visited[c]) return true;
        visited[c] = true;
    }

    return false;
}

/**
 * @brief Checks if a string contains spaces
 * @param str the string to be checked
 * @return bool: true or falsehood
 */
bool contains_spaces(const char *str)
{
    if (str == NULL) return false;
    const size_t len = strlen(str);
    if (len == 0) return false;

    for (size_t i = 0; i < len; ++i)
    {
        if (isspace(str[i])) return true;
    }
    return false;
}

/**
 * @brief Counts alphabetic characters in a string
 * @param str the string where the alphas should be counted
 * @return size_t: num of alphas, SIZE_MAX for errors
 */
size_t count_alphas(const char *str)
{
    if (str == NULL) return SIZE_MAX;
    const size_t len = strlen(str);
    size_t counter   = 0;
    for (size_t i = 0; i < len; ++i)
    {
        if (isalpha(str[i])) counter++;
    }

    return counter;
}

/**
 * @brief Counts the number of occurrences of c in str
 * @param str string where the chars should be counted
 * @param c the char which is to be counted
 * @return size_t: the number of chars, SIZE_MAX for error
 */
size_t count_c(const char *str, const char c)
{
    if (str == NULL) return SIZE_MAX;
    const size_t len = strlen(str);
    if (len == 0) return SIZE_MAX;

    size_t occ = 0;
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == c) occ++;
    }
    return occ;
}

/**
 * @brief Tests if str2 is a substring of str1
 * @param str1 first string
 * @param str2 second string
 * @return bool: true or falsehood
 */
bool is_substring(const char *str1, const char *str2)
{
    return strstr(str1, str2) != NULL;
}

/**
 * @brief Calculates the index of coincidence
 * @param arr int array from which the IC should be calculated
 * @param len size of the array
 * @return double: the IC, NaN for error
 */
double calc_index_of_coincidence(const uint8_t *arr, const size_t len)
{
    if (arr == NULL) return NaN;
    if (len == 0) return 0;

    double numerator = 0;
    double denominator;
    int32_t occ[ALPHABET_SIZE] = {0};

    if (len < 2) return 0;

    for (size_t i = 0; i < len; ++i)
    {
        occ[toupper(arr[i])]++;
    }

    for (uint16_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        const int32_t c = occ[i];
        numerator += c * (c - 1);
    }
    const double len_d = (double) len;

    denominator = len_d * (len_d - 1);
    if (denominator == 0) return NaN;

    return numerator / denominator;
}

/**
 * @brief Reads a line from the console
 * @note Ignores \n
 * @param str the string in which the line should be stored
 * @param lim upper limit of how much characters are to be read
 * @return size_t: number of characters read
 */
size_t my_getline(char *str, const size_t lim)
{
    if (str == NULL) return 0;

    if(fgets(str, lim, stdin) == NULL) return 0;

    size_t len = strlen(str);

    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = 0;
        len--;
    }

    return len;
}
