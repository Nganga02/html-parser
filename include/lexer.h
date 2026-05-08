#ifndef H_LEXER_H
#define H_LEXER_H

#include <stdlib.h>

#include "utils.h"

#define NULLTERMINATOR 0X00
#define EXCLAMATION 0X21
#define QUOTES 0X22
#define POUND 0X23
#define SINGLEQUOTES 0X27
#define DASH 0x2D
#define LESSTHAN 0X3C
#define EQUAL 0X3D
#define GREATERTHAN 0X3E
#define FOWARDSLASH 0X2F
#define AMPERSAND 0X26
#define SPACE 0X20
#define SEMICOLON 0X3B

typedef enum LexerState
{
    TAG,
    DATA,
}lexer_state;

struct Lexer
{
    struct Lexeme *lexemes;
    lexer_state state;
    int lexemes_len;
    int capacity;
};


void lexer_parse(char *str_stream, int str_len, struct Lexer *lexer);

#endif /*H_LEXER_H*/
