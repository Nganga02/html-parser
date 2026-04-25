#include "lexer.h"
#include "utils.h"
#include "tokens.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define JUMP 1
#define BUF_SIZE 128
#define INITLENGTH 32

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

// This function peeks to see if a character exists `x` jumps
static int peek(char *str, int index, int jumps, int length)
{
    if (!str)
        return 0;
    while ((index + jumps) < length + 1)
    {
        if (!(&str[index + jumps]) || str[index + jumps] == NULLTERMINATOR)
            return 0;
        else
            return 1;
    }
}

void flush_buffer(struct String *buf)
{
    if (buf->length > 0)
    {
#ifdef INFO
        printf("---- %s ----\n", buf->value);
#endif
        memset(buf->value, 0, buf->capacity * sizeof(char));
        if (buf->capacity > INITLENGTH)
        {
            char *tmp = realloc(buf->value, INITLENGTH * sizeof(char));

            if (tmp)
            {
                buf->value = tmp;
                buf->capacity = INITLENGTH;
            }
        }
        buf->length = 0;
    }
}

struct Token *emit_token(char *buf, int *i, struct Lexer *lexer, token_type type, bool has_attr, struct Attributes *attributes)
{
    struct Token *new_token = malloc(sizeof(*new_token));
    if (*i > 0)
    {
        buf[*i] = '\0';
#ifdef INFO
        printf("---- %s ----\n", buf);
#endif
    }
    if (!new_token)
        return NULL;

    // Creating a the struct we first initialize the string literal
    struct String *token_name = str_create();

    if (!token_name)
    {
        free(new_token);
        return NULL;
    }

    if (!append(token_name, buf, *i))
    {
        free_string(token_name);
        token_name = NULL;
        free(new_token);
        return NULL;
    }
    else
    {
        new_token->type = type;
        new_token->name = token_name;
        token_name = NULL; /*Making sure the pointer does not point to our data so we can destroy it*/
        new_token->has_attr = has_attr;

        if (has_attr)
        {
            new_token->attributes = attributes;
            attributes = NULL;
        }
        else
        {
            new_token->attributes = NULL;
        }
        return new_token;
    }
}

// Function to determine if a character c exist after n steps
static int check_character(char *str, int index, int jumps, int length, char c)
{
    if (peek(str, index, jumps, length) && str[index + jumps] == c)
        return 1;
    else
        return 0;
}

// This function returns a character and increaments the index
static char consume(char *str, int *index)
{
    return str[(*index)++];
}

static void consume_n_characters(char *str, int *index, int n)
{
    *index += n;
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
    char c;
    c = str_stream[0];

    int index = 1, i = 0, j = 0;
    lexer->tokens = token_init();
    lexer->tokens_len = 0;
    lexer->capacity = 4;

    while (peek(str_stream, index, JUMP, str_len))
    {
        // This function only reads open tags
        if (c == LESSTHAN || (c = consume(str_stream, &index)) == LESSTHAN)
        {
            lexer->state = tag;

            if ((c = consume(str_stream, &index)) == EXCLAMATION)
            {
                lexer->state = markup_dec_open;
                /*In markup declaration open state*/
                if ((c = consume(str_stream, &index)) == DASH)
                {
                    lexer->state = comment;
                    process_comment(str_stream, str_len, &index, lexer, &c, buf);
                }
                // Processing doctype
                else if (isalpha(c))
                {
                    index -= 1;
                    lexer->state = doctype;
                    process_doctype(str_stream, str_len, &index, lexer, &c, buf);
                }
                else
                {
                    // We are supposed to throw an error here.
                }
            }
            else if (isalpha(c))
            {
                // processing start tag
                index -= 1;
                process_start_tag(str_stream, str_len, &index, lexer, &c, buf);
            }
            else if (c = FOWARDSLASH)
            {
                // processing end tag
                process_end_tag(str_stream, str_len, &index, lexer, &c, buf);
            }
        }
        else
        {
            /**
             * We are supposed to process the text, the following are the rules for processing the text for html5
             * - When we encounter an `ampersand (&)` we are supposed to process the text up to the semicolon(;)
             *   and emit the token as a character referenceee
             */
            if (lexer->state != data)
                lexer->state = data;

            process_text(str_stream, str_len, &index, lexer, JUMP, &c, buf);
        }
    }
    free_string(buf);
}

/**
 * This is an ancillary function that performs analysis on the DOCTYPE tag
 * 
 * It is triggered when the lexer state changes form markup declaration open to 
 * It outputs tokens based on the state of the lexer since this tag does not have
 * the same syntax as othet tags
 */

static void process_doctype(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{

    while (peek(stream, *index, JUMP, stream_len))
    {

        *c = consume(stream, index);

        if (*c == GREATERTHAN)
        {
            if(lexer->state == bef_doctype_PI || lexer->state == bef_doctype_SI)
            flush_buffer(buf);
            lexer->state = data;
            break;
        }
        if(*c ==  QUOTES)
        {
            if(lexer->state == bef_doctype_PI || lexer->state == bef_doctype_SI)
            {
                lexer->state = lexer->state == bef_doctype_PI ? doc_PI_dq : doc_SI_dq;
            }else if(lexer->state == doc_PI_dq || lexer->state == doc_PI_dq)
            {
                flush_buffer(buf);
            }
            continue;
        }

        if (isspace((unsigned char)*c))
        {

            /* Making sure the buffer is not empty */
            if (buf->length > 0)
            {
                if (lexer->state == markup_dec_open && (strcmp(buf->value, "doctype")) == 0)
                {
                    flush_buffer(buf);
                    lexer->state = bef_doctype_name;
                }else if(lexer->state == bef_doctype_name)
                {
                    flush_buffer(buf);
                    lexer->state = aft_doctype_name;
                }else if(lexer->state == bef_doctype_PI)
                {
                    if((strcmp(buf->value, "public")) == 0)
                    {
                        flush_buffer(buf);
                        lexer->state = bef_doctype_PI;

                    }else if((strcmp(buf->value, "system")) == 0)
                    {
                        flush_buffer(buf);
                        lexer->state = bef_doctype_SI;
                    }
                }
            }
            continue;
        }

        /*The lexer only hits this point if it is a character*/
        if ((*c > 0x40) && (*c < 0x5b))
            *c += 0x20;
        append(buf, c, 1);
    }
}

static void process_comment(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{

    *c = consume(stream, index);
    while (peek(stream, *index, JUMP, stream_len) && !(
                                                         check_character(stream, *index, 0, stream_len, DASH) &&
                                                         check_character(stream, *index, 1, stream_len, DASH) &&
                                                         check_character(stream, *index, 2, stream_len, GREATERTHAN)))
    {
        *c = consume(stream, index);
        // Creating the buffer
        append(buf, c, 1);
    }
    flush_buffer(buf);
    consume_n_characters(stream, index, 3);
}

// Called when we encounter an alphanumeric characters after < open tag
static void process_start_tag(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{

    while (peek(stream, *index, JUMP, stream_len))
    {
        *c = consume(stream, index);
        if (*c == GREATERTHAN)
        {
            flush_buffer(buf);
            break;
        }

        if (*c == FOWARDSLASH && check_character(stream, *index, 0, stream_len, GREATERTHAN)) // At this point we are processing the self closing tag
        {
            lexer->state = self_closing_start_tag;
            continue;
        }

        if (isspace(*c) && buf->length > 0)
        {
            flush_buffer(buf);
            continue;
        }
        if (!isspace(*c))
        {
            append(buf, c, 1);
        }
    }
}

static void process_end_tag(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{

    while (peek(stream, *index, JUMP, stream_len))
    {
        *c = consume(stream, index);
        if (*c == GREATERTHAN)
        {
            flush_buffer(buf);
            break;
        }
        if (isspace(*c))
        {
            flush_buffer(buf);
            continue;
        }
        if (!isspace(*c))
        {
            append(buf, c, 1);
        }
    }
}

// Called when we encounter the > of an open tag
static void process_text(char *stream, int stream_len, int *index, struct Lexer *lexer, const int jump, char *c, struct String *buf)
{
    if (!isspace(*c))
    {
        if (*c == AMPERSAND)
        {
            while (peek(stream, *index, jump, stream_len) && *c != SPACE && *c != LESSTHAN)
            {
                append(buf, c, 1);
                if (*c == SEMICOLON)
                    break;
                *c = consume(stream, index);
            }
        }
        else
        {
            while (peek(stream, *index, jump, stream_len) && !check_character(stream, *index, 0, stream_len, AMPERSAND) && *c != LESSTHAN)
            {
                append(buf, c, 1);
                *c = consume(stream, index);
            }
        }
        flush_buffer(buf);
    }
}

void process_tag_attributes(char *stream, int stream_len, int *index, struct Lexer *lexer, const int jump, char *c)
{
}
