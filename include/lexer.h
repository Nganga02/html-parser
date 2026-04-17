#ifndef H_LEXER_H
#define H_LEXER_H
#include "utils.h"

#define NULLTERMINATOR  0X00
#define EXCLAMATION     0X21
#define QUOTES          0X22
#define DASH            0x2D
#define LESSTHAN        0X3C
#define EQUAL           0X3D
#define GREATERTHAN     0X3E
#define FOWARDSLASH     0X2F
#define AMPERSAND       0X26 
#define SPACE           0X20
#define SEMICOLON       0X3B

typedef enum TokenType{
    DocType = 0001,
    StartTag = 0002,
    EndTag = 0004,
    Comment = 0010,
    Text = 0020,
}token_type;

typedef enum TagCategory{
    root,
    metadata,
    sectioning,
    text_content,
    input,
    iterations,
    inputs,
    media,
    scripting,
}tag_category;

struct Token{
    char * value;
    token_type type;
    tag_category category;
    bool has_attr;
    bool is_self_closing;
};

struct Attributes{
    char *name;
    char **value;//One attribute can have many strings
};

enum LexerState{
    text,
    tag,
    attribute,
    character_reference,
};

struct Lexer{
    int state;

};

void lexer_parse(char *str_stream, int str_len, struct Token *tokens);

#endif /*H_LEXER_H*/



/** According to W3 standards, the following are the lexer states
 * - RCDATA
 * - PCDATA - Initial state
 * - CDATA
 * - PLAINTEXT
 * 
 * 
 * when an end tag has attributes, that is a parser error
 */