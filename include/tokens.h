#ifndef H_TOKEN_H
#define H_TOKEN_H

#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"
#include "utils.h"

struct Token *token_init(void);
bool push_back_token(struct Lexer *lexer, struct Token *new_token);
struct Token *pop_token(struct Lexer *lexer);
void free_token(struct Token *token);
void free_tokens(struct Lexer *lexer);

#endif /*H_TOKEN_H*/
