#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INITLENGTH 32


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
            free_string(instance);
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
void free_string(struct String *str)
{
    free(str->value);
    free(str);

    str = NULL;//Preventing dangling pointers
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
            free_string(src_code);
            return NULL;
        }
    }
    return src_code;
}
