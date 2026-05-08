#include "lexer.h"
#include "utils.h"
#include "lex.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define JUMP 1
#define SIZEOF_CHAR 1
#define BUF_SIZE 128
#define PREVIOUS_CHAR -2

// Called when we encounter ! after < open tag
static void process_doctype(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf);

static void process_comment(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf);

// Called when we encounter an alphanumeric characters after < open tag
static void process_start_tag(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf);

static void process_end_tag(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf);

// Called when we encounter the > of an open tag
static void process_text(char *stream, int stream_len, int *index, struct Lexer *lexer, const int jump, char *c, struct String *buf);

// Function called when we want to process attributes
void process_tag_attributes(char *stream, int stream_len, int *index, struct Lexer *lexer, const int jump, char *c);

// This function peeks to see if a character exists `x` offset
static int peek(char *str, int index, int length)
{
    if (!str)
        return 0;
    while ((index) < length + 1)
    {
        if (!(&str[index]) || str[index] == NULLTERMINATOR)
            return 0;
        else
            return 1;
    }
}

// Function to determine if a character c exist after n steps
char check_character(char *str, int index, int offset, int length)
{
    if (peek(str, index, length) && &str[index + offset] != NULL)
        return str[index + offset];
    else if (&str[index + offset] != NULL && (index + offset) <= length)
        return str[index + offset];
    else
        return '\0';
}

// This function returns a character and increaments the index
static char consume(char *str, int *index)
{
    return str[(*index)++];
}

static inline void consume_comments(char *str, int str_len, int *index_ptr, char *c)
{
    while (peek(str, *index_ptr, str_len))
    {
        *c = consume(str, index_ptr);
        if (*c == DASH)
        {
            *c = consume(str, index_ptr);
            if (*c == DASH)
            {
                *c = consume(str, index_ptr);
                if (*c == GREATERTHAN)
                {
                    *c = consume(str, index_ptr);
                    break;
                }
            }
        }
    }
}

static inline void check_comment(char *str, int *index, int str_len, char *c, struct String *buf)
{
    if (check_character(str, *index, 0, str_len) != EXCLAMATION)
        return;
    if (check_character(str, *index, 1, str_len) != DASH)
        return;
    if (check_character(str, *index, 2, str_len) != DASH)
        return;
    flush_buffer(buf);
    consume_comments(str, str_len, index, c);
}
/**
 * Main function that is responsible for lexical analysis
 *
 * Takes in as arguments:
 *  @param char* str_stream which is the string containing the html format
 *  @param int This is the length of string
 *  @param struct Lexer* which is a instance of a lexer.
 */
void lexer_parse(char *str_stream, int str_len, struct Lexer *lexer)
{
    struct String *buf = str_create();
    struct Lexeme *lxm;
    char *sub = malloc(sizeof(char));
    if (!sub)
        *sub = SPACE;
    char c = '\0';

    int index = 0;
    lexer->lexemes = lexeme_init();
    lexer->lexemes_len = 0;
    lexer->capacity = 4;

    while (peek(str_stream, index, str_len))
    {
        c = consume(str_stream, &index);
        /**
         * PSEUDO-CODE
         *
         *
         * For tags we are supposed to get rid of spaces and only output lexemes.
         * For characters we are supposed to output the spaces too.
         *
         */

        switch (c)
        {
        case LESSTHAN:
            if (buf->length > 0)
            {
                if (lexer->state == DATA)
                    lxm = emit_lexeme(buf, TEXT);
                else
                    lxm = emit_lexeme(buf, IDENTITY);

                push_back_lexeme(lexer, lxm);
            }
            append(buf, &c, SIZEOF_CHAR);
            if (!isalpha((unsigned char)check_character(str_stream, index, 0, str_len)))
            {
                check_comment(str_stream, &index, str_len, &c, buf);
                c = consume(str_stream, &index);
                append(buf, &c, SIZEOF_CHAR);
            }
            lxm = emit_lexeme(buf, TAG_OPEN);
            push_back_lexeme(lexer, lxm);
            lexer->state = TAG;
            break;
        case FOWARDSLASH:
            if (buf->length > 0)
            {
                lxm = emit_lexeme(buf, IDENTITY);
                push_back_lexeme(lexer, lxm);
            }
            append(buf, &c, SIZEOF_CHAR);
            if (check_character(str_stream, index, 0, str_len) == GREATERTHAN)
            {
                c = consume(str_stream, &index);
                append(buf, &c, SIZEOF_CHAR);
                lexer->state = DATA;
            }
            lxm = emit_lexeme(buf, TAG_CLOSE);
            push_back_lexeme(lexer, lxm);

            break;
        case GREATERTHAN:
            if (buf->length > 0)
            {
                lxm = emit_lexeme(buf, IDENTITY);
                push_back_lexeme(lexer, lxm);
            }
            append(buf, &c, SIZEOF_CHAR);
            lxm = emit_lexeme(buf, TAG_CLOSE);
            push_back_lexeme(lexer, lxm);
            lexer->state = DATA;
            break;
        case EQUAL:
            if (buf->length > 0)
            {
                lxm = emit_lexeme(buf, IDENTITY);
                push_back_lexeme(lexer, lxm);
            }
            append(buf, &c, SIZEOF_CHAR);
            lxm = emit_lexeme(buf, ASSIGN_OP);
            push_back_lexeme(lexer, lxm);

            break;
        case QUOTES:
            while (peek(str_stream, index, str_len))
            {
                c = consume(str_stream, &index);
                if (c == QUOTES)
                    break;
                append(buf, &c, SIZEOF_CHAR);
            }
            lxm = emit_lexeme(buf, STRING);
            push_back_lexeme(lexer, lxm);

            break;
        case SINGLEQUOTES:
            while (peek(str_stream, index, str_len))
            {
                c = consume(str_stream, &index);
                if (c == SINGLEQUOTES)
                    break;
                append(buf, &c, SIZEOF_CHAR);
            }
            lxm = emit_lexeme(buf, STRING);
            push_back_lexeme(lexer, lxm);

            break;
        case AMPERSAND:
            if (buf->length > 0)
            {
                lxm = emit_lexeme(buf, IDENTITY);
                push_back_lexeme(lexer, lxm);
            }
            append(buf, &c, SIZEOF_CHAR);
            if (check_character(str_stream, index, 0, str_len) == POUND)
            {
                c = consume(str_stream, &index);
                append(buf, &c, SIZEOF_CHAR);
            }
            lxm = emit_lexeme(buf, CHAR_REF_IDENT);
            push_back_lexeme(lexer, lxm);

            break;
        case SEMICOLON:
            if (buf->length > 0)
            {
                lxm = emit_lexeme(buf, IDENTITY);
                push_back_lexeme(lexer, lxm);
            }
            append(buf, &c, SIZEOF_CHAR);
            lxm = emit_lexeme(buf, SEMICOLON_LEX);
            push_back_lexeme(lexer, lxm);

            break;
        case SPACE:
            if (lexer->state == DATA)
                append(buf, &c, SIZEOF_CHAR);
            if (lexer->state == TAG)
            {
                if (buf->length > 0)
                {
                    lxm = emit_lexeme(buf, IDENTITY);
                    push_back_lexeme(lexer, lxm);
                }
            }
            break;
        default:

            if (lexer->state == DATA)
            {
                if (isspace(c) && !check_character(str_stream, index, 0, str_len) == LESSTHAN)
                {
                    append(buf, sub, SIZEOF_CHAR);
                    break;
                }
            }
            append(buf, &c, SIZEOF_CHAR);
            break;
        }
    }
    free(sub);
}
