#include "tokens.h"


/**
 * Given the minimum value of a valid html file is
 * \<!DOCTYPE html> initializing a token array of 4 elements
 * for buffering before doubling the capacity.
 */
struct Token *token_init(void)
{
    struct Token *tokens_array = malloc(4 * sizeof(*tokens_array));
    if (!tokens_array)
        return NULL;

    return tokens_array;
}

/**
 * Appends a new token to the end of the lexer's dynamic token array
 *
 * This function ensures that the lexer has enough memory to store an
 * additional token. If the current capacity is insufficient, the internal
 * token buffer is resized by doubling the capacity of the buffer.
 *
 * On successful insertion, the token is copied by value into the array and
 * token length increamented.
 */

bool push_back_token(struct Lexer *lexer, struct Token *new_token)
{
    struct Token *tmp;
    if (lexer->tokens_len + 1 >= lexer->capacity)
    {
        while (lexer->tokens_len + 1 >= lexer->capacity)
        {
            lexer->capacity *= 2;
        }
        tmp = realloc(
            lexer->tokens,
            lexer->capacity * sizeof(struct Token));
    }

    if (!tmp)
    {
        fprintf(stderr, "Error reallocating\n Reassigning old tokens");
        return;
    }
    else
    {
        lexer->tokens = tmp;
        tmp = NULL;
        lexer->tokens[lexer->tokens_len++] = *new_token;
    }
}

/**
 * Popping and returning the last element of the array
 *
 * The following pops and returns the last element of the token array,
 * It also checks if the array elements are less than 3/4 of half the
 * token's array capacity, if true it reallocates half the token's.
 *
 * Upon successful ops, the function returns a pointer to the popped
 * token
 */
struct Token *pop_token(struct Lexer *lexer)
{
    struct Token *tokens = lexer->tokens;
    int length = lexer->tokens_len;

    if (!tokens || length == 0)
    {
        return NULL;
    }

    struct Token *popped_token = malloc(sizeof(*popped_token));
    if (!popped_token)
        return NULL;

    *popped_token = tokens[length - 1];
    length -= 1;
    if (length <= (int)(0.375 * lexer->capacity)) // We want the integer floor value
    {
        int new_capacity = 0.5 * lexer->capacity;
        struct Token *tmp = realloc(lexer->tokens, new_capacity * sizeof(struct Token));

        if (!tmp)
        {
#ifdef DEBUG
            fprintf(stderr, "REALLOC ERROR: [%s] sizing down the token array\n", __func__);
#endif
        }
        else
        {
            lexer->tokens = tmp;
            lexer->capacity = new_capacity;
        }
    }
    lexer->tokens_len = length;
    return popped_token;
}

/**
 * Destroying a single token
 *
 * The function takes as a parameter
 * - @param token: This is a pointer to a token element and performs and in-depth
 *      freeing.
 *
 */

void free_token(struct Token *token)
{
    if (token->attributes)
    {
        free_string(token->attributes->name);
        free_string(token->attributes->value);
        free(token->attributes);
        token->attributes = NULL;
    }

    free_string(token->name);
}

/**
 * Destroying the array of tokens
 *
 * The function takes as parameters:
 * - @param lexer this contains the length of the token array and loops throught the
 *      array freeing the memory allocated by malloc.
 */

void destroy_tokens(struct Lexer *lexer)
{
    if (!lexer->tokens || !lexer)
        return; // If the tokens point to NULL return

    for (int i = 0; i < lexer->tokens_len; i++)
    {
        free_token(&lexer->tokens[i]);
    }
    free(lexer->tokens);
    lexer->tokens = NULL;
    lexer->tokens_len = 0;
    lexer->capacity = 0;
#ifdef DEBUG
    fprintf(stderr, "NULL ASSIGNMENT: [%s] lexer->tokens in NULL\n"__func__);
#endif
}