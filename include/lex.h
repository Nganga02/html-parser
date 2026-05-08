#ifndef H_LEXEME_H
#define H_LEXEME_H

#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"
#include "utils.h"

typedef enum LexType
{
    TAG_OPEN = 0001,
    TAG_CLOSE = 0002, 
    IDENTITY = 0004,
    ASSIGN_OP = 0010,
    CHAR_REF_IDENT= 0020,
    CHAR_REF = 0040,
    STRING = 0100,
    SEMICOLON_LEX = 0200,
    TEXT = 0400, 
} lex_type;

struct Lexeme
{
    struct String *value;
    lex_type type;
};

/**
 * Focuses on the lexemes bit
 */
struct Lexeme *lexeme_init(void);
bool push_back_lexeme(struct Lexer *lexer, struct Lexeme *new_lexeme);
struct Lexeme *emit_lexeme(struct String *buf, lex_type type);
struct Lexeme *pop_lexeme(struct Lexer *lexer);
void free_lexeme(struct Lexeme *lexeme);
void free_lexemes(struct Lexer *lexer);
int update_lexeme(struct Lexeme *instance, void *updates, bool is_attributes);
#endif 
