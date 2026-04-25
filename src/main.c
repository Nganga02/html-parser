#include "lexer.h"
#include "utils.h"


// Dynamic string DS


// Functions prototype


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

    // Initialization
    struct Lexer Lx = {
        .state = data,
    };

    lexer_parse(read_string->value, read_string->length, &Lx);

    free_string(read_string);
    return 0;
}

// Creating a dynamic string
