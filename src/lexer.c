#include "lexer.h"
#include <string.h>
#include <ctype.h>

#define JUMP 1
#define BUF_SIZE 128

// Called when we encounter ! after < open tag
static void process_doctype(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

static void process_comment(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

// Called when we encounter an alphanumeric characters after < open tag
static void process_start_tag(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

static void process_end_tag(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

static void process_self_closing_tag(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

// Called when we encounter the > of an open tag
static void process_text(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

// Function called when we want to process attributes
void process_tag_attributes(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

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

void flush_buffer( char *buf, int *i)
{
    if (*i > 0)
    {
        buf[*i] = '\0';
        printf("---- %s ----\n", buf);
        *i = 0;
    }
}

struct Token *emit_token( char *buf, int *i)
{
    if (*i > 0)
    {
        buf[*i] = '\0';
        printf("---- %s ----\n", buf);
        *i = 0;
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

void lexer_parse(char *str_stream, int str_len, struct Token *tokens)
{
    char str_buf[128] = {0};
    char c;
    c = str_stream[0];

    int index = 1, i = 0, j = 0;

    while (peek(str_stream, index, JUMP, str_len))
    {
        // This function only reads open tags
        if (c == LESSTHAN || (c = consume(str_stream, &index)) == LESSTHAN)
        {

            if ((c = consume(str_stream, &index)) == EXCLAMATION)
            {
                // Processing comments
                if ((c = consume(str_stream, &index)) == DASH)
                {
                    process_comment(str_stream, str_len, &index, tokens, JUMP, &c);
                }
                // Processing doctype
                else if (isalpha(c))
                {
                    index -= 1;
                    process_doctype(str_stream, str_len, &index, tokens, JUMP, &c);
                }
            }
            else if (isalpha(c))
            {
                // processing start tag
                index -= 1;
                process_start_tag(str_stream, str_len, &index, tokens, JUMP, &c);
            }
            else if (c = FOWARDSLASH)
            {
                // processing end tag
                process_end_tag(str_stream, str_len, &index, tokens, JUMP, &c);
            }
        }
        else
        {
            /**
             * We are supposed to process the text, the following are the rules for processing the text for html5
             * - When we encounter an `ampersand (&)` we are supposed to process the text up to the semicolon(;)
             *   and emit the token as a character referenceee
             */
            process_text(str_stream, str_len, &index, tokens, JUMP, &c);
        }
    }
}

/**
 * Called when we encouter a ! after an open tag <
 */

// For this we are processing the doctype
static void process_doctype(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c)
{
    char buf[BUF_SIZE] = {0};
    int i = 0;

    while (peek(stream, *index, JUMP, stream_len))
    {

        *c = consume(stream, index);

        if (*c == GREATERTHAN)
        {
            flush_buffer(buf, &i);
            break;
        }

        if (isspace((unsigned char)*c))
        {
            flush_buffer(buf, &i);
            continue;
        }

        // Safe append
        if (i < BUF_SIZE - 1)
        {
            buf[i++] = *c;
        }
        else
        {
            // Optional: handle overflow (truncate or error)
        }
    }
}

static void process_comment(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c)
{
    char buf[BUF_SIZE];
    int i = 0;
    *c = consume(stream, index);
    while (peek(stream, *index, JUMP, stream_len) && !(
                                                         check_character(stream, *index, 0, stream_len, DASH) &&
                                                         check_character(stream, *index, 1, stream_len, DASH) &&
                                                         check_character(stream, *index, 2, stream_len, GREATERTHAN)))
    {
        *c = consume(stream, index);
        if (!isspace(*c))
        {
            // Creating the buffer
            buf[i++] = *c;
        }
        else
        {
            // Processing the buffer

            buf[i] = '\0';
            printf("-- comments: %s --\n", buf);
            memset(buf, 0, i);
            i = 0;
        }
    }
    consume_n_characters(stream, index, 3);
}

// Called when we encounter an alphanumeric characters after < open tag
static void process_start_tag(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c)
{
    char buf[32] = {0};
    int i = 0;

    while (peek(stream, *index, JUMP, stream_len))
    {
        *c = consume(stream, index);
        if (*c == GREATERTHAN)
        {
            flush_buffer(buf, &i);
            break;
        }

        if (isspace(*c) && strlen(buf) > 0)
        {
            flush_buffer(buf, &i);
            continue;
        }
        if (!isspace(*c))
        {
            buf[i++] = *c;
        }
    }
}

static void process_end_tag(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c)
{
    char buf[32] = {0};
    int i = 0;

    while (peek(stream, *index, JUMP, stream_len))
    {
        *c = consume(stream, index);
        if (*c == GREATERTHAN)
        {
            flush_buffer(buf, &i);
            break;
        }
        if (isspace(*c))
        {
            flush_buffer(buf, &i);
            continue;
        }
        if (!isspace(*c))
        {
            buf[i++] = *c;
        }
    }
}

static void process_self_closing_tag(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c);

// Called when we encounter the > of an open tag
static void process_text(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c)
{
    char buf[128] = {0};
    int i = 0;
    if (!isspace(*c))
    {
        if (*c == AMPERSAND)
        {
            while (peek(stream, *index, jump, stream_len) && *c != SPACE && *c != SEMICOLON && *c != LESSTHAN)
            {
                buf[i++] = *c;
                *c = consume(stream, index);
            }
        }
        else
        {
            while (peek(stream, *index, jump, stream_len) && !check_character(stream, *index, 0, stream_len, AMPERSAND) && *c != LESSTHAN)
            {
                buf[i++] = *c;
                *c = consume(stream, index);
            }
        }
        flush_buffer(buf, &i);
    }
}

void process_tag_attributes(char *stream, int stream_len, int *index, struct Token *tokens, const int jump, char *c)
{
}
