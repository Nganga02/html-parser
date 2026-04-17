#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>

#include "lexer.h"

#define INITLENGTH 32

// Dynamic string DS
struct String
{
    char *value;
    size_t length;
    size_t capacity;
};

// Functions prototype
struct String *read_file(char *filename);
struct String *str_create(void);
void destroy(struct String *str);

int main(int argc, char *argv[])
{
    /*Loading the file into a string */
    if (argc == 1)
    {
        fprintf(stderr, "Usage: Requires a html document\n\t./lexer <inputfile>\n");
        exit(10);
    }
    if (strlen(argv[1]) > 28)
    {
        fprintf(stderr, "File too long\n");
        exit(11);
    }
    char *filename = argv[1];
    char extension[5] = {'.', 'h', 't', 'm', 'l'};

    struct String *read_string;

    if (strcmp(&filename[strlen(filename) - 5], extension) == 0)
    {
        read_string = read_file(filename);
    }
    else
    {
        fprintf(stderr, "Wrong file extension\n");
        exit(12);
    }

#if defined(DEBUG) && DEBUG == 1
    printf("%s", read_string->value);
#endif

    /*+-----------------------------------------------+*/
    /*|   Performing lexical analysis on the string   |*/
    /*+-----------------------------------------------+*/
    // Dynamic string DS
    struct Token tokens[20] = {0};

    printf("%p\n", read_string->value);

    lexer_parse(read_string->value, read_string->length, tokens);
    printf("%p\n", read_string->value);

    printf("%s\n", read_string->value);

    destroy(read_string);
    return 0;
}

// Creating a dynamic string
struct String *str_create(void)
{
    struct String *s = malloc(sizeof(*s));
    if (!s)
        return NULL;

    s->value = malloc(INITLENGTH);
    if (!s->value)
    {
        free(s);
        return NULL;
    }

    s->capacity = INITLENGTH;
    s->length = 0;
    return s;
}

// Function to append string to the end of the string
int append(struct String *instance, char *buf, int read_size)
{
    if (!instance || !buf)
    {
        return 0;
    }
    int index = instance->length;

    if ((instance->capacity - instance->length) < read_size)
    {
        size_t ncapacity = instance->capacity * 2;
        while (ncapacity < instance->length + read_size + 1)
        {
            ncapacity *= 2;
        }

        char *sptr = realloc(instance->value, ncapacity);
        if (!sptr)
        {
            destroy(instance);
            return 0;
        }

        instance->value = sptr;
        instance->capacity = ncapacity;
    }

    for (int i = 0; i < read_size; i++)
    {
        instance->value[index++] = buf[i];
    }
    instance->value[index] = '\0';
    instance->length += read_size;
    return 1;
}

// Destroying string after use
void destroy(struct String *str)
{
    free(str->value);
    free(str);
}

// Reading the file and returning a string
struct String *read_file(char *filename)
{

    int fd;
    size_t buf_size;
    if ((fd = open(filename, O_RDONLY)) == -1)
    {
        fprintf(stderr, "error opening");
        exit(12);
    }

    struct String *src_code = str_create();

    if (!src_code)
        return NULL;

    char buf[INITLENGTH];

    while ((buf_size = read(fd, buf, INITLENGTH)) > 0)
    {
        // For atomicity
        if (!append(src_code, buf, buf_size))
        {
            destroy(src_code);
            return NULL;
        }
    }
    return src_code;
}