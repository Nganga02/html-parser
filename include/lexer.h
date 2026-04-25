#ifndef H_LEXER_H
#define H_LEXER_H
#include "utils.h"

#define NULLTERMINATOR  0X00
#define EXCLAMATION     0X21
#define QUOTES          0X22
#define SINGLEQUOTES    0X27
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
    Character_ref = 0040
}token_type;

struct Token{
    struct String *name;
    struct Attributes *attributes; //This field can be null
    token_type type;
    bool has_attr;
};

struct Attributes{
    struct String *name;
    struct String *value;//One attribute can have many strings
};

/*we will use the following while generating tokens*/
enum LexerState{
    /*tracks the state of text between tags*/
    data,                           // Data state
    character_reference,            // Character reference


    /*Tracks the state of doctype declaration*/
    markup_dec_open,                // Markup declaration open
    doctype,                        // Doctype state
    bef_doctype_name,               // Before Doctype name
    aft_doctype_name,               // After Doctype name
    bef_doctype_PI,                 // Before Doctype public Identifier
    bef_doctype_SI,                 // Before Doctype system Identifier
    doc_PI_dq,                      // Doctype PI(double quotes)
    doc_PI_sq,                      // Doctype PI(single quotes)
    aft_doctype_PI,                 // After Doctype PI
    aft_doctype_SI,                 // After Doctype SI
    doc_SI_dq,                      // Doctype SI(double quotes)
    doc_SI_sq,                      // Doctype SI(single quotes)


    /* Tracks the state of start/self-closing tag */
    tag,
    self_closing_start_tag,
    bef_attr_name,
    attr_name_state,
    aft_attr_name,
    bef_attr_value,
    attr_value_dq,
    attr_value_unq,
    attr_value_sq,
    char_ref_attr_val,

    /*Tracks comments in the file*/
    comment
};

struct Lexer{
    struct Token *tokens;
    enum LexerState state;
    int tokens_len;
    int capacity;
};

void lexer_parse(char *str_stream, int str_len, struct Lexer *lexer);

#endif /*H_LEXER_H*/
