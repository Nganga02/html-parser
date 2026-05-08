```C

/**
 * This is an ancillary function that performs analysis on the DOCTYPE tag
 *
 * It is triggered when the lexer state changes form markup declaration open to
 * It outputs tokens based on the state of the lexer since this tag does not have
 * the same syntax as othet tags
 */

static void process_doctype(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{
    __uint8_t count = 0;
    while (peek(stream, *index, JUMP, stream_len))
    {

        *c = consume(stream, index);

        if (*c == GREATERTHAN)
        {
            switch (lexer->state)
            {
            case doctype_name:
            case aft_doctype_name:
            case aft_doctype_PI:
            case aft_doctype_SI:
                flush_buffer(buf);
                break;
            case bef_doctype_PI:
            case doc_PI_dq:
            case doc_PI_sq:
            case bef_doctype_SI:
            case doc_SI_dq:
            case doc_SI_sq:
                flush_buffer(buf);
            case bef_doctype_name:
                /// TODO: Generate a token with a force_quirks flag set.
                break;
            default:
                break;
            }
            lexer->state = data;
            break;
        }
        if (*c == QUOTES)
        {

            switch (lexer->state)
            {
            case bef_doctype_PI:
                lexer->state = doc_PI_dq;
                break;
            case doc_PI_dq:
                flush_buffer(buf);
                lexer->state = aft_doctype_PI;
                break;
            case doc_PI_sq:
                goto append_input;
            case aft_doctype_PI:
                lexer->state = doc_SI_dq;
                break;
            case bef_doctype_SI:
                lexer->state = doc_PI_dq;
                break;
            case doc_SI_dq:
                flush_buffer(buf);
                lexer->state = aft_doctype_SI;
                break;
            case doc_SI_sq:
                goto append_input;
            case aft_doctype_SI:
                /// TODO: set the force quirk's flag to true
                fprintf(stderr, "setting the force quirks flag on\n");
                break;
            default:
                goto append_input;
                break;
            }
            continue;
        }

        if (*c == SINGLEQUOTES)
        {

            switch (lexer->state)
            {
            case bef_doctype_PI:
                lexer->state = doc_PI_sq;
                break;
            case doc_PI_sq:
                flush_buffer(buf);
                lexer->state = aft_doctype_PI;
                break;
            case doc_PI_dq:
                goto append_input;
            case aft_doctype_PI:
                lexer->state = doc_SI_sq;
                break;
            case bef_doctype_SI:
                lexer->state = doc_PI_sq;
                break;
            case doc_SI_sq:
                flush_buffer(buf);
                lexer->state = aft_doctype_SI;
                break;
            case doc_SI_dq:
                goto append_input;
            case aft_doctype_SI:
                /// TODO: set the force quirk's flag to true
                fprintf(stderr, "setting the force quirks flag on\n");
                break;
            default:
                goto append_input;
                break;
            }
            continue;
        }

        if (isspace((unsigned char)*c))
        {

            /* Making sure the buffer is not empty */
            switch (lexer->state)
            {
            case doctype:
                if ((strcmp(buf->value, "doctype")) == 0)
                {
                    flush_buffer(buf);
                    lexer->state = bef_doctype_name;
                }
                break;
            case doctype_name:
                flush_buffer(buf);
                lexer->state = aft_doctype_name;
                break;
            case doc_PI_dq:
            case doc_PI_sq:
            case doc_SI_dq:
            case doc_SI_sq:
                goto append_input;
            default:
                break;
            }
            continue;
        }

    /*The lexer only hits this point if it is a character*/
    append_input:
        if (lexer->state != doc_PI_dq && lexer->state != doc_PI_sq && lexer->state != doc_SI_dq && lexer->state != doc_SI_sq)
            if ((*c > 0x40) && (*c < 0x5b))
                *c += 0x20;
        append(buf, c, 1);
        switch (lexer->state)
        {
        case bef_doctype_name:
            lexer->state = doctype_name;
            break;
        case aft_doctype_name:
            count++;
            if (count == 6)
            {
                if ((strcmp(buf->value, "public")) == 0)
                {
                    flush_buffer(buf);
                    count = 0;
                    lexer->state = bef_doctype_PI;
                }
                else if ((strcmp(buf->value, "system")) == 0)
                {
                    flush_buffer(buf);
                    count = 0;
                    lexer->state = bef_doctype_SI;
                }
            }

            break;
        }
    }
}

static void process_comment(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{

    *c = consume(stream, index);
    while (peek(stream, *index, JUMP, stream_len) && !(
                                                         check_character(stream, *index, 0, stream_len, DASH) &&
                                                         check_character(stream, *index, 1, stream_len, DASH) &&
                                                         check_character(stream, *index, 2, stream_len, GREATERTHAN)))
    {
        *c = consume(stream, index);
        // Creating the buffer
        append(buf, c, 1);
    }
    flush_buffer(buf);
    consume_n_characters(stream, index, 3);
}

// Called when we encounter an alphanumeric characters after < open tag
static void process_start_tag(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{
    struct Token *tk = malloc(sizeof(*tk));
    if(!tk){
        perror("malloc error\n");
        return;
    }
    struct Attribute *at;
    bool from_double = false;

    while (peek(stream, *index, JUMP, stream_len))
    {
        *c = consume(stream, index);
        if (*c == GREATERTHAN)
        {
            switch (lexer->state)
            {
            case attr_name:
                at = init_attribute();
                append(at->name, buf, buf->length);
            case attr_value_unq:
                append(at->value, buf, buf->length);
            case bef_attr_name:
            case aft_attr_name:
            case bef_attr_value:
            case tag_name:
                tk = emit_token()
                push_back_token(lexer, tk);
                lexer->state = data;
                break;
            case attr_value_dq:
            case attr_value_sq:
                goto append_values;
            default:
                break;
            }
            lexer->state = data;
            break;
        }

        if (*c == FOWARDSLASH) // At this point we are processing the self closing tag
        {
            switch (lexer->state)
            {
            case tag_name:
            case attr_name:
            case attr_value_unq:
            case char_ref_attr_val:
                /// TODO: update the tag/attribute names.
            case bef_attr_name:
            case aft_attr_name:
            case bef_attr_value:
                lexer->state = self_closing_start_tag;
                break;
            case attr_value_dq:
            case attr_value_sq:
                goto append_values;
            default:
                break;
            }
            lexer->state = self_closing_start_tag;
            continue;
        }
        if (*c == AMPERSAND)
        {
            switch (lexer->state)
            {
            case bef_attr_name:
            case aft_attr_name:
                lexer->state = attr_name;
            case tag_name:
            case attr_name:
                goto append_values;
            case bef_attr_value:
                lexer->state = attr_value_unq;
                goto append_values;
            case attr_value_dq:
                from_double = true;
            case attr_value_sq:
                lexer->state = char_ref_attr_val;
                goto append_values;
            case self_closing_start_tag:
                lexer->state = attr_name;
                goto append_values;
            }
        }

        if (*c == QUOTES || *c == SINGLEQUOTES)
        {
            switch (lexer->state)
            {
            case tag_name:
                goto append_values;
            case bef_attr_value:
                lexer->state = *c == QUOTES ? attr_value_dq : attr_value_sq;
                break;
            case attr_value_dq:
                if (*c == QUOTES)
                {
                    /// TODO:Update token value
                    flush_buffer(buf);
                    lexer->state = aft_attr_quoted;
                    break;
                }
                else
                {
                    goto append_values;
                }
            case attr_value_sq:
                if (*c == SINGLEQUOTES)
                {
                    /// TODO:Update token value
                    flush_buffer(buf);
                    lexer->state = aft_attr_quoted;
                    break;
                }
                else
                {
                    goto append_values;
                }
            case char_ref_attr_val:
                if (from_double)
                {
                    if (*c == QUOTES)
                    {
                        flush_buffer(buf);
                        lexer->state = aft_attr_quoted;
                    }
                }
                else
                {
                    if (*c == SINGLEQUOTES)
                    {
                        flush_buffer(buf);
                        lexer->state = aft_attr_quoted;
                    }
                }
                break;
            default:
                break;
            }
            continue;
        }

        if (*c == EQUAL)
        {
            switch (lexer->state)
            {
            case tag_name:
                goto append_values;
            case attr_name:
                /// TODO: Update the token name
                flush_buffer(buf);
            case aft_attr_name:
                lexer->state = bef_attr_value;
                break;
            case attr_value_dq:
            case attr_value_sq:
            case char_ref_attr_val:
                goto append_values;
            default:
                break;
            }
            continue;
        }
        if (isspace((unsigned char)*c))
        {
            switch (lexer->state)
            {
            case tag_name: // we should flush the buffer
                flush_buffer(buf);
                lexer->state = bef_attr_name;
                break;
            case attr_name:
                flush_buffer(buf);
                lexer->state = aft_attr_name;
                break;
            case attr_value_unq:
                flush_buffer(buf);
                lexer->state = bef_attr_name;
                break;
            case self_closing_start_tag:
                flush_buffer(buf);
                lexer->state = bef_attr_name;
                break;
            case attr_value_dq:
            case attr_value_sq:
                goto append_values;
            case char_ref_attr_val:
                lexer->state = from_double ? attr_value_dq : attr_value_sq;
                goto append_values;
            default:
                break;
            }
            continue;
        }

    append_values:
        if (lexer->state != attr_value_dq && lexer->state != attr_value_sq)
        {
            if ((*c > 0x40) && (*c < 0x5b))
                *c += 0x20;
        }
        append(buf, c, 1);
        switch (lexer->state)
        {
        case bef_attr_name:
            lexer->state = attr_name;
            break;
        case tag:
            lexer->state = tag_name;
            break;
        case aft_attr_quoted:
            lexer->state = attr_name;
        default:
            break;
        }
    }
}

static void process_end_tag(char *stream, int stream_len, int *index, struct Lexer *lexer, char *c, struct String *buf)
{
    while (peek(stream, *index, JUMP, stream_len))
    {
        *c = consume(stream, index);
        if (*c == GREATERTHAN)
        {
            struct Token *tk = emit_token(buf, lexer, EndTag, false, NULL);
            flush_buffer(buf);
            break;
        }
        if (isspace(*c))
        {
            continue;
        }
        if (!isspace(*c))
        {
            append(buf, c, 1);
        }
    }
}

// Called when we encounter the > of an open tag
static void process_text(char *stream, int stream_len, int *index, struct Lexer *lexer, const int jump, char *c, struct String *buf)
{
    if (!isspace(*c))
    {
        if (*c == AMPERSAND)
        {
            while (peek(stream, *index, jump, stream_len) && *c != SPACE && *c != LESSTHAN)
            {
                append(buf, c, 1);
                if (*c == SEMICOLON)
                    break;
                *c = consume(stream, index);
            }
        }
        else
        {
            while (peek(stream, *index, jump, stream_len) && !check_character(stream, *index, 0, stream_len, AMPERSAND) && *c != LESSTHAN)
            {
                append(buf, c, 1);
                *c = consume(stream, index);
            }
        }
        struct Token *tk = emit_token(buf, lexer, Text, false, NULL);
        if (push_back_token(lexer->tokens, tk))
        {
            free_token(tk);
        }
        else
        {
            printf("Push back fail");
            free_token(tk);
        }
        flush_buffer(buf);
    }
}

void process_tag_attributes(char *stream, int stream_len, int *index, struct Lexer *lexer, const int jump, char *c)
{
}



```


```C

// This function only reads open tags
        if (c == LESSTHAN || (c = consume(str_stream, &index)) == LESSTHAN)
        {
            lexer->state = tag;

            if ((c = consume(str_stream, &index)) == EXCLAMATION)
            {
                lexer->state = markup_dec_open;
                /*In markup declaration open state*/
                if ((c = consume(str_stream, &index)) == DASH)
                {
                    lexer->state = comment;
                    process_comment(str_stream, str_len, &index, lexer, &c, buf);
                }
                // Processing doctype
                else if (isalpha(c))
                {
                    index -= 1;
                    lexer->state = doctype;
                    process_doctype(str_stream, str_len, &index, lexer, &c, buf);
                }
                else
                {
                    // We are supposed to throw an error here.
                }
            }
            else if (isalpha(c))
            {
                // processing start tag
                index -= 1;
                lexer->state = tag;
                process_start_tag(str_stream, str_len, &index, lexer, &c, buf);
            }
            else if (c = FOWARDSLASH)
            {
                // processing end tag
                process_end_tag(str_stream, str_len, &index, lexer, &c, buf);
            }
        }
        else
        {
            /**
             * We are supposed to process the text, the following are the rules for processing the text for html5
             * - When we encounter an `ampersand (&)` we are supposed to process the text up to the semicolon(;)
             *   and emit the token as a character referenceee
             */
            if (lexer->state != data)
                lexer->state = data;

            process_text(str_stream, str_len, &index, lexer, JUMP, &c, buf);
        }
    }
    free_string(buf);

```