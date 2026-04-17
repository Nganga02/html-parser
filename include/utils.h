#ifndef H_UTILS_H
#define H_UTILS_H
#include <stdio.h>
#include <stdbool.h>

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



#endif /*H_UTILS_H*/