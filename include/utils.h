#ifndef H_UTILS_H
#define H_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct DocumentNode{
    int number;
    struct ElementNode *elements;
};

struct AttributesNode{
    int height;
    int width;
};

struct ElementNode{
    struct ElementNode *child;
    struct AttributesNode attributes;
};

struct TextNodes{
    char *text;
};

struct CommentNode{

};

struct ProcessingInstructionNode{

};

struct String
{
    char *value;
    size_t length;
    size_t capacity;
};

struct String *read_file(char *filename);
struct String *str_create(void);
void flush_buffer(struct String *buf);
int append(struct String *instance, char *buf, int read_size);
void free_string(struct String *str);


#endif /*H_UTILS_H*/