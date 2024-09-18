#include <string.h>
#include <ctype.h>

#include "helper.h"

/**
 * @brief Gets the number literal from a string
 * @param str the string containing the number literal
 * @param number the number to be extracted
 * @return int: error code
 */
int32_t get_number_from_string(const char *str, int32_t *number)
{
    if (strlen(str) == 0) return 1;
    char *endptr;

    *number = (int32_t) strtol(str, &endptr, 10);
    if (*endptr != '\0')
    {
        printf("Invalid input %s", str);
    }

    return 0;
}

/**
 * @brief Capitalizes the string
 * @param input the string to be capitalized
 * @return int: error code
 */
int32_t to_upper_case(char *input)
{
    if (input == NULL) return 1;
    const size_t length = strlen(input);
    if (length == 0) return 1;


    for (size_t i = 0; i < length; ++i)
    {
        input[i] = (char) toupper(input[i]);
    }

    return 0;
}

/**
 * @brief Removes all none alphabetic and numeric chars from a string
 * @param input the string where the alnums chars should be removed
 * @return int: error code
 */
int32_t remove_none_alnum(char *input)
{
    if (input == NULL) return 1;
    if (strlen(input) == 0) return 1;
    int32_t i = 0;
    int32_t j = 0;

    while (input[i] != '\0')
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
 * @brief Removes all none alphabetic chars
 * @param input the string where the none alphabetic chars should be removed
 * @return int: error code
 */
int32_t remove_none_alpha(char *input)
{
    if (input == NULL) return 1;
    if (strlen(input) == 0) return 1;
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
 * @param first first str
 * @param second second str
 * @return bool: true or falsehood
 */
bool is_permutation(const char *first, const char *second)
{
    if (first == NULL || second == NULL) return false;
    const size_t len1 = strlen(first);
    const size_t len2 = strlen(second);

    if (len1 != len2) return false;

    int32_t count[ASCII_SIZE] = {0};

    for (size_t i = 0; i < len1; i++)
    {
        count[(unsigned char) first[i]]++;
        count[(unsigned char) second[i]]--;
    }

    for (size_t i = 0; i < ASCII_SIZE; i++)
    {
        if (count[i] != 0) return false;
    }

    return true;
}

/**
 * @brief Subtracts 'A' from all chars
 * @param str the original string
 * @return int32_t*: to the processed array
 */
uint8_t* get_int_array_from_string(const char *str)
{
    if (str == NULL) return NULL;
    const size_t len = strlen(str);
    uint8_t *array   = malloc(len * sizeof(uint8_t));
    assertmsg(array != NULL, "array == NULL");

    for (size_t i = 0; i < len; i++)
    {
        array[i] = str[i] - 'A';
    }

    return array;
}

/**
 * @brief Adds 'A' to all chars
 * @param array the int array to be converted
 * @param size the array size
 * @return char*: to the processed string
 */
char* get_string_from_int_array(const uint8_t *array, const size_t size)
{
    if (array == NULL) return NULL;
    char *str = malloc(size + 1);
    assertmsg(str != NULL, "str == NULL");

    for (size_t i = 0; i < size; i++)
    {
        str[i] = array[i] + 'A';
    }
    str[size] = 0;

    return str;
}

/**
 * @brief Checks if a string has duplicate chars
 * @note Ignores spaces
 * @param str the string to be checked
 * @return bool: true of falsehood
 */
bool has_duplicates(const char *str)
{
    const size_t len = strlen(str);
    if (str == NULL) return false;
    if (len == 0) return false;

    bool seen[ASCII_SIZE] = {false};
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = str[i];
        if (seen[c]) return true;
        if (!isspace(c)) seen[c] = true;
    }

    return false;
}

/**
 * @brief Checks if a string contains spaces
 * @param str the string to be checked
 * @return bool: true of falsehood
 */
bool contains_spaces(const char *str)
{
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
 * @return int: num of alphas
 */
int32_t count_alphas(const char *str)
{
    int32_t counter  = 0;
    const size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        if (isalpha(str[i])) counter++;
    }

    return counter;
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

int32_t count(const char *str, const char c)
{
    int32_t occ      = 0;
    const size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == c) occ++;
    }
    return occ;
}

/**
 * @brief Calculates the index of coincidence
 * @param str int array from which the IC should be calculated
 * @param len size of the array
 * @return double: the IC
 */
double calc_index_of_coincidence(const uint8_t *str, const size_t len)
{
    double numerator = 0;
    double denominator;
    int32_t occ[ALPHABET_SIZE] = {0};

    if (len < 2) return 0;

    for (size_t i = 0; i < len; ++i)
    {
        occ[toupper(str[i])]++;
    }

    for (uint16_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        const int32_t c = occ[i];
        numerator += c * (c - 1);
    }
    const double len_d = (double) len;

    denominator = len_d * (len_d - 1);

    return numerator / denominator;
}

/**
 * @brief Reads a line from the console, ignores \n
 * @param str the string in which the line should be stored
 * @param lim upper limit of how much characters are to be read
 * @return size_t: number of characters read
 */
size_t mygetline(char *str, size_t lim)
{
    int32_t c;
    const char *temp = str;
    while (--lim > 0 && (c = getchar()) != EOF && c != '\n')
        *str++             = (char) c;

    *str = 0;
    return str - temp;
}
