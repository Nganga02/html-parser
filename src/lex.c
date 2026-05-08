#include "lex.h"
#include "utils.h"
#include <stdio.h>


/**
 * Given the minimum value of a valid html file is
 * \<!DOCTYPE html> initializing a lexeme array of 4 elements
 * for buffering before doubling the capacity.
 */
struct Lexeme *lexeme_init(void)
{
    struct Lexeme *lexemes_array = malloc(4 * sizeof(*lexemes_array));
    if (!lexemes_array)
        return NULL;

    return lexemes_array;
}

/**
 * Appends a new lexeme to the end of the lexer's dynamic lexeme array
 *
 * This function ensures that the lexer has enough memory to store an
 * additional lexeme. If the current capacity is insufficient, the internal
 * lexeme buffer is resized by doubling the capacity of the buffer.
 *
 * On successful insertion, the lexeme is copied by value into the array and
 * lexeme length increamented.
 */

bool push_back_lexeme(struct Lexer *lexer, struct Lexeme *new_lexeme)
{
    struct Lexeme *tmp;
    if (lexer->lexemes_len + 1 >= lexer->capacity)
    {
        while (lexer->lexemes_len + 1 >= lexer->capacity)
        {
            lexer->capacity *= 2;
        }
        tmp = realloc(
            lexer->lexemes,
            lexer->capacity * sizeof(struct Lexeme));
        if (tmp)
        {
            lexer->lexemes = tmp;
            tmp = NULL;
        }
        else
        {
            fprintf(stderr, "Error reallocating\n Reassigning old lexemes");
            return false;
        }
    }

        lexer->lexemes[lexer->lexemes_len++] = *new_lexeme;
        return true;
}

/**
 * Popping and returning the last element of the array
 *
 * The following pops and returns the last element of the lexeme array,
 * It also checks if the array elements are less than 3/4 of half the
 * lexeme's array capacity, if true it reallocates half the lexeme's.
 *
 * Upon successful ops, the function returns a pointer to the popped
 * lexeme
 */
struct Lexeme *pop_lexeme(struct Lexer *lexer)
{
    struct Lexeme *lexemes = lexer->lexemes;
    int length = lexer->lexemes_len;

    if (!lexemes || length == 0)
    {
        return NULL;
    }

    struct Lexeme *popped_lexeme = malloc(sizeof(*popped_lexeme));
    if (!popped_lexeme)
        return NULL;

    *popped_lexeme = lexemes[length - 1];
    length -= 1;
    if (length <= (int)(0.375 * lexer->capacity)) // We want the integer floor value
    {
        int new_capacity = 0.5 * lexer->capacity;
        struct Lexeme *tmp = realloc(lexer->lexemes, new_capacity * sizeof(struct Lexeme));

        if (!tmp)
        {
#ifdef DEBUG
            fprintf(stderr, "REALLOC ERROR: [%s] sizing down the lexeme array\n", __func__);
#endif
        }
        else
        {
            lexer->lexemes = tmp;
            lexer->capacity = new_capacity;
        }
    }
    lexer->lexemes_len = length;
    return popped_lexeme;
}

/**
 * Emitting a lexeme to append to the array of lexemes to be processed by the tree walker
 */

struct Lexeme *emit_lexeme(struct String *buf, lex_type type)
{
    const char *LEX_NAMES[] = {
        [TAG_OPEN] = "TAG_OPEN",
        [TAG_CLOSE] = "TAG_CLOSE",
        [IDENTITY] = "IDENTITY",
        [ASSIGN_OP] = "ASSIGN_OP",
        [CHAR_REF_IDENT] = "CHAR_REF_IDENT",
        [CHAR_REF] = "CHAR_REF",
        [STRING] = "STRING",
        [SEMICOLON_LEX] = "SEMICOLON",
        [TEXT] = "TEXT"
    };
    struct Lexeme *new_lexeme = malloc(sizeof(*new_lexeme));
    if (buf->length > 0)
    {
#ifdef INFO
        printf("---- %s ----\n", buf->value);
#endif
    }
    if (!new_lexeme)
        return NULL;

    // Creating the struct we first initialize the string literal
    struct String *lexeme_value = str_create();

    if (!lexeme_value)
    {
        free(new_lexeme);
        return NULL;
    }

    if (!append(lexeme_value, buf->value, buf->length))
    {
        free_string(lexeme_value);
        lexeme_value = NULL;
        free(new_lexeme);
        return NULL;
    }
    else
    {
        flush_buffer(buf);
        new_lexeme->type = type;
        new_lexeme->value = lexeme_value;
#ifdef DEBUG
        printf("lex type: %s lex value:---- %s -----\n", LEX_NAMES[new_lexeme->type], new_lexeme->value->value);
#endif
        lexeme_value = NULL;
        return new_lexeme;
    }
}

/**
 * Destroying a single lexeme
 *
 * The function takes as a parameter
 * - @param lexeme: This is a pointer to a lexeme element and performs and in-depth
 *      freeing.
 *
 */

void free_lexeme(struct Lexeme *lexeme)
{
    if (!lexeme)
    {
#ifdef DEBUG
        fprintf(stderr, "[WARNING]: NULL POINTER PASSED TO %s", __func__);
#endif
        return;
    }

    free_string(lexeme->value);
    lexeme->value = NULL;
}

/**
 * Destroying the array of lexemes
 *
 * The function takes as parameters:
 * - @param lexer this contains the length of the lexeme array and loops throught the
 *      array freeing the memory allocated by malloc.
 */

void free_lexemes(struct Lexer *lexer)
{
    if (!lexer->lexemes || !lexer)
        return; // If the lexemes point to NULL return

    for (int i = 0; i < lexer->lexemes_len; i++)
    {
        free_lexeme(&lexer->lexemes[i]);
    }
    free(lexer->lexemes);
    lexer->lexemes = NULL;
    lexer->lexemes_len = 0;
    lexer->capacity = 0;
}
