#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#include "array.h"



typedef ARRAY_OF(char) string_t;

#define FREAD_CHUNK_SIZE 1024

string_t read_entire_file(FILE* file) {
    string_t contents = {0};

    while (0 == feof(file) && 0 == ferror(file)) {
        array_expand(&contents, FREAD_CHUNK_SIZE);
        char*  write_location = contents.elements + contents.count;
        size_t bytes_read     = fread(write_location, sizeof(char), FREAD_CHUNK_SIZE, file);
        contents.count += bytes_read;
    }

    return contents;
}

char* cstring_from_buffer(const char* buffer, size_t buffer_size) {
    char* cstring = malloc(buffer_size + 1);
    if (NULL == cstring) {
            (void)fputs(
                 "Error: Unable to allocate memory for C-string conversion; buy more RAM lol",
                 stderr
            );
            exit(1);
    }

    (void)memcpy(cstring, buffer, buffer_size);
    cstring[buffer_size + 1] = '\0';

    return cstring;
}



typedef enum {
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_ATOM,
    TOKEN_NEWLINE,
    TOKEN_PARENTHESIS
} TokenType;

const char* token_type_name(TokenType type) {
    switch (type) {
    case TOKEN_STRING:      return "TOKEN_STRING";
    case TOKEN_INTEGER:     return "TOKEN_INTEGER";
    case TOKEN_FLOAT:       return "TOKEN_FLOAT";
    case TOKEN_ATOM:        return "TOKEN_ATOM";
    case TOKEN_NEWLINE:     return "TOKEN_NEWLINE";
    case TOKEN_PARENTHESIS: return "TOKEN_PARENTHESIS";
    default:                assert(false && "Unhandled token type");
    }
}

#define MAX_TOKEN_SIZE 256

typedef union {
    char*   as_string;
    int64_t as_integer;
    double  as_float;
    char*   as_atom;
    char    as_newline;
    char    as_parenthesis;
} Token;

typedef struct {
    TokenType type;
    Token     token;
    size_t    line;
    size_t    column;
} Lexeme;

typedef ARRAY_OF(Lexeme) LexemeArray;

void lexeme_free(Lexeme* lexeme) {
    switch (lexeme->type) {

    case TOKEN_STRING:      free(lexeme->token.as_string); break;
    case TOKEN_INTEGER:     break;
    case TOKEN_FLOAT:       break;
    case TOKEN_ATOM:        free(lexeme->token.as_atom); break;
    case TOKEN_NEWLINE:     break;
    case TOKEN_PARENTHESIS: break;
    default:                break;
    }
}

typedef struct {
    const string_t* program;
    size_t          program_index;
    const char*     program_name;
    char*           token_buffer;
    size_t          token_buffer_size;
    LexemeArray     lexemes;
    size_t          line;
    size_t          column;
    size_t          multibyte_line;
    size_t          multibyte_column;
} LexerContext;

void try_append_multibyte_lexeme(LexerContext* context) {
    if (0 == context->token_buffer_size) return;

    char* token = cstring_from_buffer(context->token_buffer, context->token_buffer_size);

    size_t token_size          = context->token_buffer_size;
    context->token_buffer_size = 0;

    Lexeme lexeme;
    lexeme.line   = context->multibyte_line;
    lexeme.column = context->multibyte_column;

    char*   end_pointer = token;
    int64_t as_integer  = strtol(token, &end_pointer, 10);
    if (token_size == (size_t)(end_pointer - token)) {
        lexeme.type             = TOKEN_INTEGER;
        lexeme.token.as_integer = as_integer;
        goto lend;
    }

    end_pointer     = token;
    double as_float = strtod(token, &end_pointer);
    if (token_size == (size_t)(end_pointer - token)) {
        lexeme.type           = TOKEN_FLOAT;
        lexeme.token.as_float = as_float;
        goto lend;
    }

    lexeme.type          = TOKEN_ATOM;
    lexeme.token.as_atom = token;
    goto lend;

 lend:
    array_append(&context->lexemes, lexeme);
    context->multibyte_line   = SIZE_MAX;
    context->multibyte_column = SIZE_MAX;
}

void lex_string(LexerContext* context) {
    context->multibyte_line   = context->line;
    context->multibyte_column = context->column;

    // Skips past first quote.
    ++context->column;
    ++context->program_index;

    string_t string          = {0};
    bool     found_end_quote = false;

    for (; context->program_index < context->program->count; ++context->program_index) {
        if (found_end_quote) break;

        char character = array_at(context->program, context->program_index);

        switch (character) {
        case '"': {
            found_end_quote = true;
            ++context->column;
        } break;

        case '\n': {
            ++context->line;
            context->column = 0;
        } break;

        case '\\': {
            ++context->program_index;
            if (context->program_index >= context->program->count) {
                goto lunterminated_string;
            }
            character = array_at(context->program, context->program_index);

            switch (character) {
            case '"':  array_append(&string, character); break;
            case '\\': array_append(&string, character); break;
            case 'n':  array_append(&string, '\n');      break;
            case 't':  array_append(&string, '\t');      break;

            default: {
                (void)fprintf(
                     stderr,
                     "%s(%zu:%zu): Error: Unknown escape sequence '\\%c'",
                     context->program_name,
                     context->line, context->column,
                     character
                );
                exit(1);
            } break;
            };

            context->column += 2;
        } break;

        default: {
            array_append(&string, character);
            ++context->column;
        } break;
        };
    }

    if (!found_end_quote) goto lunterminated_string;

    char* token = cstring_from_buffer(string.elements, string.count);
    array_free(&string);

    Lexeme lexeme = {
        .type   = TOKEN_STRING,
        .token  = { .as_string = token },
        .line   = context->line,
        .column = context->column
    };
    array_append(&context->lexemes, lexeme);

    context->multibyte_line   = SIZE_MAX;
    context->multibyte_column = SIZE_MAX;

    return;

 lunterminated_string:
    (void)fprintf(
         stderr,
         "%s(%zu:%zu): Error: Unterminated string",
         context->program_name,
         context->multibyte_line, context->multibyte_column
    );
    exit(1);
}

LexemeArray lex_program(const string_t *restrict program, const char *restrict program_name) {
    char         token_buffer[MAX_TOKEN_SIZE];
    LexerContext context = {
        .program           = program,
        .program_index     = 0,
        .program_name      = program_name,
        .token_buffer      = token_buffer,
        .token_buffer_size = 0,
        .lexemes           = {0},
        .line              = 1,
        .column            = 0,
        .multibyte_line    = SIZE_MAX,
        .multibyte_column  = SIZE_MAX
    };

    for (; context.program_index < context.program->count; ++context.program_index) {
        char character = array_at(context.program, context.program_index);

        switch (character) {
        case '\n': {
            try_append_multibyte_lexeme(&context);

            Lexeme lexeme = {
                .type   = TOKEN_NEWLINE,
                .token  = { .as_newline = character },
                .line   = context.line,
                .column = context.column
            };
            array_append(&context.lexemes, lexeme);

            ++context.line;
            context.column = 0;
        } break;

        case '(':
        case ')': {
            try_append_multibyte_lexeme(&context);

            Lexeme lexeme = {
                .type   = TOKEN_PARENTHESIS,
                .token  = { .as_parenthesis = character },
                .line   = context.line,
                .column = context.column
            };
            array_append(&context.lexemes, lexeme);

            ++context.column;
        } break;

        case '"': {
            try_append_multibyte_lexeme(&context);
            lex_string(&context);
        } break;

        case ' ':
        case '\t': {
            try_append_multibyte_lexeme(&context);
            ++context.column;
        } break;

        default: {
            if (MAX_TOKEN_SIZE <= context.token_buffer_size) {
                (void)fprintf(
                     stderr,
                     "%s(%zu:%zu): Error: Encountered token larger than the maximum allowed size %zu: %.*s",
                     context.program_name,
                     context.line, context.column,
                     (size_t)MAX_TOKEN_SIZE,
                     (int)context.token_buffer_size, context.token_buffer
                );
                exit(1);
            }

            if (SIZE_MAX == context.multibyte_line)
                context.multibyte_line = context.line;
            if (SIZE_MAX == context.multibyte_column)
                context.multibyte_column = context.column;
            context.token_buffer[context.token_buffer_size++] = character;

            ++context.column;
        } break;
        }
    }

    try_append_multibyte_lexeme(&context);

    return context.lexemes;
}

void dump_lexemes( FILE *restrict stream
                 , const LexemeArray *restrict lexemes
                 , const char *restrict program_name) {
    for (size_t i = 0; i < lexemes->count; ++i) {
        Lexeme lexeme = array_at(lexemes, i);

        switch (lexeme.type) {
        case TOKEN_STRING: {
            (void)fprintf(
                 stream,
                 "%s(%zu:%zu): %s: \"",
                 program_name, lexeme.line, lexeme.column,
                 token_type_name(lexeme.type)
            );

            for ( const char* character = lexeme.token.as_string
                ; '\0' != *character
                ; ++character) {
                switch (*character) {
                case '"':
                case '\\': {
                    (void)fputc('\\', stream);
                    (void)fputc(*character, stream);
                } break;

                case '\n': {
                    (void)fputs("\\n", stream);
                } break;

                case '\t': {
                    (void)fputs("\\t", stream);
                } break;

                default: {
                    (void)fputc(*character, stream);
                } break;
                }
            }

            (void)fputs("\"\n", stream);
        } break;

        case TOKEN_INTEGER: {
            (void)fprintf(
                 stream,
                 "%s(%zu:%zu): %s: %ld\n",
                 program_name, lexeme.line, lexeme.column,
                 token_type_name(lexeme.type),
                 lexeme.token.as_integer
            );
        } break;

        case TOKEN_FLOAT: {
            (void)fprintf(
                 stream,
                 "%s(%zu:%zu): %s: %f\n",
                 program_name, lexeme.line, lexeme.column,
                 token_type_name(lexeme.type),
                 lexeme.token.as_float
            );
        } break;

        case TOKEN_ATOM: {
            (void)fprintf(
                 stream,
                 "%s(%zu:%zu): %s: %s\n",
                 program_name, lexeme.line, lexeme.column,
                 token_type_name(lexeme.type),
                 lexeme.token.as_atom
            );
        } break;

        case TOKEN_NEWLINE: {
            (void)fprintf(
                 stream,
                 "%s(%zu:%zu): %s\n",
                 program_name, lexeme.line, lexeme.column,
                 token_type_name(lexeme.type)
            );
        } break;

        case TOKEN_PARENTHESIS: {
            (void)fprintf(
                 stream,
                 "%s(%zu:%zu): %s: %c\n",
                 program_name, lexeme.line, lexeme.column,
                 token_type_name(lexeme.type),
                 lexeme.token.as_parenthesis
            );
        } break;

        default: assert(false && "Unhandled token type");
        };
    }
}



#define SOURCE_FILE "test.tlpin"

int main(void) {
    FILE* source = fopen(SOURCE_FILE, "r");
    if (NULL == source) {
        perror("Error: Unable to open file '" SOURCE_FILE "'");
        return 1;
    };

    string_t contents = read_entire_file(source);
    (void)fclose(source);

    LexemeArray lexemes = lex_program(&contents, SOURCE_FILE);
    array_free(&contents);

    dump_lexemes(stdout, &lexemes, SOURCE_FILE);

    for (size_t i = 0; i < lexemes.count; ++i)
        lexeme_free(&array_at(&lexemes, i));
    array_free(&lexemes);

    return 0;
}
