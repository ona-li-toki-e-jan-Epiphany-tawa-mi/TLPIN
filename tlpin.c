#include "array.h"

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

typedef double float64_t;



/* typedef enum { */
/*     TOKEN_STRING, */
/*     TOKEN_CHARACTER, */
/*     TOKEN_FLOAT, */
/*     TOKEN_ATOM, */
/*     TOKEN_NEWLINE, */
/*     TOKEN_PARENTHESIS, */
/*     TOKEN_BRACKET */
/* } TokenType; */

/* const char* token_type_name(TokenType type) { */
/*     switch (type) { */
/*     case TOKEN_STRING:      return "TOKEN_STRING"; */
/*     case TOKEN_CHARACTER:   return "TOKEN_CHARACTER"; */
/*     case TOKEN_FLOAT:       return "TOKEN_FLOAT"; */
/*     case TOKEN_ATOM:        return "TOKEN_ATOM"; */
/*     case TOKEN_NEWLINE:     return "TOKEN_NEWLINE"; */
/*     case TOKEN_PARENTHESIS: return "TOKEN_PARENTHESIS"; */
/*     case TOKEN_BRACKET:     return "TOKEN_BRACKET"; */
/*     default:                assert(false && "Unhandled token type"); */
/*     } */
/* } */

/* #define MAX_TOKEN_SIZE 256 */

/* typedef union { */
/*     sstring_t as_string; */
/*     char      as_character; */
/*     double    as_float; */
/*     sstring_t as_atom; */
/*     char      as_newline; */
/*     char      as_parenthesis; */
/*     char      as_bracket; */
/* } Token; */

/* typedef struct { */
/*     TokenType type; */
/*     Token     token; */
/*     size_t    line; */
/*     size_t    column; */
/* } Lexeme; */

/* void lexeme_free(Lexeme* lexeme, array_free_t free) { */
/*     switch (lexeme->type) { */
/*     case TOKEN_STRING:      sstring_free(&lexeme->token.as_string, free); break; */
/*     case TOKEN_CHARACTER:   break; */
/*     case TOKEN_FLOAT:       break; */
/*     case TOKEN_ATOM:        sstring_free(&lexeme->token.as_atom, free); break; */
/*     case TOKEN_NEWLINE:     break; */
/*     case TOKEN_PARENTHESIS: break; */
/*     case TOKEN_BRACKET:     break; */
/*     default:                break; */
/*     } */
/* } */

/* typedef ARRAY_OF(Lexeme) LexemeArray; */

/* void lexeme_array_free(LexemeArray* lexemes, array_free_t free) { */
/*     for (size_t i = 0; i < lexemes->count; ++i) */
/*         lexeme_free(&array_at(lexemes, i), free); */
/*     array_free(lexemes, free); */
/* } */

/* typedef struct { */
/*     const sstring_t* program; */
/*     size_t           program_index; */
/*     const char*      program_name; */
/*     sstring_t        token_buffer; */
/*     LexemeArray      lexemes; */
/*     size_t           line; */
/*     size_t           column; */
/*     size_t           multibyte_line; */
/*     size_t           multibyte_column; */
/*     array_realloc_t  realloc; */
/*     bool             failed; */
/* } LexerContext; */

/* void try_append_multibyte_lexeme(LexerContext* context) { */
/*     if (0 == context->token_buffer.count) return; */

/*     Lexeme lexeme = {0}; */
/*     lexeme.line   = context->multibyte_line; */
/*     lexeme.column = context->multibyte_column; */

/*     SstringConvertResult result; */

/*     // We try to parse it as a float. */
/*     double as_float = sstring_to_double(&context->token_buffer, &result); */
/*     switch (result) { */
/*     case SSTRING_CONVERT_SUCCESS: { */
/*         lexeme.type           = TOKEN_FLOAT; */
/*         lexeme.token.as_float = as_float; */
/*         goto lend; */
/*     } break; */
/*     case SSTRING_CONVERT_UNDERFLOW: { */
/*         (void)fprintf( */
/*              stderr, */
/*              "%s(%zu:%zu): Error: Float conversion of '%" SSTRING_PRINT */
/*              "' results in underflow", */
/*              context->program_name, */
/*              lexeme.line, lexeme.column, */
/*              SSTRING_FORMAT(&context->token_buffer) */
/*         ); */
/*         context->failed = true; */
/*     } break; */
/*     case SSTRING_CONVERT_OVERFLOW: { */
/*         (void)fprintf( */
/*              stderr, */
/*              "%s(%zu:%zu): Error: Float conversion of '%" SSTRING_PRINT */
/*              "' results in overflow", */
/*              context->program_name, */
/*              lexeme.line, lexeme.column, */
/*              SSTRING_FORMAT(&context->token_buffer) */
/*         ); */
/*         context->failed = true; */
/*     } break; */
/*     case SSTRING_CONVERT_PARSE_FAIL: break; */
/*     default: assert(false && "Unhandled sstring conversion result"); */
/*     } */

/*     // If that fails, we assume it is an atom. */
/*     // TODO make this an array.h function. */
/*     lexeme.type = TOKEN_ATOM; */
/*     sstring_resize(&lexeme.token.as_atom, context->token_buffer.count, context->realloc); */
/*     (void)memcpy( */
/*          lexeme.token.as_atom.elements, */
/*          context->token_buffer.elements, */
/*          context->token_buffer.count */
/*     ); */
/*     lexeme.token.as_atom.count = context->token_buffer.count; */
/*     context->token_buffer.count = 0; */
/*     goto lend; */

/*  lend: */
/*     array_append(&context->lexemes, lexeme, context->realloc); */
/*     context->token_buffer.count = 0; */
/*     context->multibyte_line     = SIZE_MAX; */
/*     context->multibyte_column   = SIZE_MAX; */
/* } */

/* void lex_string(LexerContext* context) { */
/*     context->multibyte_line   = context->line; */
/*     context->multibyte_column = context->column; */

/*     // Skips past first quote. */
/*     ++context->column; */
/*     ++context->program_index; */

/*     sstring_t string          = {0}; */
/*     bool      found_end_quote = false; */

/*     for (; context->program_index < context->program->count; ++context->program_index) { */
/*         if (found_end_quote) break; */

/*         char character = sstring_at(context->program, context->program_index); */

/*         switch (character) { */
/*         case '"': { */
/*             found_end_quote = true; */
/*             ++context->column; */
/*         } break; */

/*         case '\n': { */
/*             ++context->line; */
/*             context->column = 0; */
/*         } break; */

/*         case '\\': { */
/*             if (++context->program_index >= context->program->count) { */
/*                 goto lunterminated_string; */
/*             } */
/*             character = sstring_at(context->program, context->program_index); */

/*             switch (character) { */
/*             case '"':  sstring_append(&string, character, context->realloc); break; */
/*             case '\\': sstring_append(&string, character, context->realloc); break; */
/*             case 'n':  sstring_append(&string, '\n',      context->realloc); break; */
/*             case 't':  sstring_append(&string, '\t',      context->realloc); break; */

/*             default: { */
/*                 (void)fprintf( */
/*                      stderr, */
/*                      "%s(%zu:%zu): Error: Unknown escape sequence '\\%c'", */
/*                      context->program_name, */
/*                      context->line, context->column, */
/*                      character */
/*                 ); */
/*                 context->failed = true; */
/*             } break; */
/*             }; */

/*             context->column += 2; */
/*         } break; */

/*         default: { */
/*             sstring_append(&string, character, context->realloc); */
/*             ++context->column; */
/*         } break; */
/*         }; */
/*     } */

/*     if (!found_end_quote) goto lunterminated_string; */

/*     Lexeme lexeme = { */
/*         .type   = TOKEN_STRING, */
/*         .token  = { .as_string = string }, */
/*         .line   = context->multibyte_line, */
/*         .column = context->multibyte_column */
/*     }; */
/*     array_append(&context->lexemes, lexeme, context->realloc); */

/*     context->multibyte_line   = SIZE_MAX; */
/*     context->multibyte_column = SIZE_MAX; */

/*     return; */

/*  lunterminated_string: */
/*     (void)fprintf( */
/*          stderr, */
/*          "%s(%zu:%zu): Error: Unterminated string", */
/*          context->program_name, */
/*          context->multibyte_line, context->multibyte_column */
/*     ); */
/*     exit(1); */
/* } */

/* void lex_character_literal(LexerContext* context) { */
/*     context->multibyte_line   = context->line; */
/*     context->multibyte_column = context->column; */

/*     // Skips past first quote. */
/*     ++context->column; ++context->program_index; */
/*     if (context->program_index >= context->program->count) { */
/*         goto lunterminated_character_literal; */
/*     } */

/*     char character = sstring_at(context->program, context->program_index); */

/*     switch (character) { */
/*     case '\\': { */
/*         ++context->column; ++context->program_index; */
/*         if (context->program_index >= context->program->count) { */
/*             goto lunterminated_character_literal; */
/*         } */

/*         character = sstring_at(context->program, context->program_index); */

/*         switch (character) { */
/*         case '\'':  character = '\''; break; */
/*         case '\\': character = '\\';  break; */
/*         case 'n':  character = '\n';  break; */
/*         case 't':  character = '\t';  break; */

/*         default: { */
/*             (void)fprintf( */
/*                  stderr, */
/*                  "%s(%zu:%zu): Error: Unknown escape sequence '\\%c'", */
/*                  context->program_name, */
/*                  context->line, context->column, */
/*                  character */
/*             ); */
/*             context->failed = true; */
/*         } break; */
/*         } */
/*     } break; */

/*     default: break; */
/*     }; */

/*     ++context->column; ++context->program_index; */
/*     if (context->program_index >= context->program->count) { */
/*         goto lunterminated_character_literal; */
/*     } */
/*     if ('\'' != sstring_at(context->program, context->program_index)) { */
/*         goto lunterminated_character_literal; */
/*     }; */

/*     ++context->column; ++context->program_index; */

/*     Lexeme lexeme = { */
/*         .type   = TOKEN_CHARACTER, */
/*         .token  = { .as_character = character }, */
/*         .line   = context->multibyte_line, */
/*         .column = context->multibyte_column */
/*     }; */
/*     array_append(&context->lexemes, lexeme, context->realloc); */

/*     context->multibyte_line   = SIZE_MAX; */
/*     context->multibyte_column = SIZE_MAX; */

/*     return; */

/*  lunterminated_character_literal: */
/*     (void)fprintf( */
/*          stderr, */
/*          "%s(%zu:%zu): Error: Unterminated character literal", */
/*          context->program_name, */
/*          context->multibyte_line, context->multibyte_column */
/*     ); */
/*     exit(1); */
/* } */

/* LexemeArray lex_program( const sstring_t *restrict program */
/*                        , const char *restrict program_name */
/*                        , array_free_t free */
/*                        , array_realloc_t realloc) { */
/*     LexerContext context = { */
/*         .program           = program, */
/*         .program_index     = 0, */
/*         .program_name      = program_name, */
/*         .token_buffer      = {0}, */
/*         .lexemes           = {0}, */
/*         .line              = 1, */
/*         .column            = 0, */
/*         .multibyte_line    = SIZE_MAX, */
/*         .multibyte_column  = SIZE_MAX, */
/*         .realloc           = realloc, */
/*         .failed            = false */
/*     }; */
/*     // Since we have a limit on how large tokens can be we can preallocate the */
/*     // buffer used to construct them. */
/*     sstring_resize(&context.token_buffer, MAX_TOKEN_SIZE, context.realloc); */

/*     for (; context.program_index < context.program->count; ++context.program_index) { */
/*         char character = sstring_at(context.program, context.program_index); */

/*         switch (character) { */
/*         case '\n': { */
/*             try_append_multibyte_lexeme(&context); */

/*             Lexeme lexeme = { */
/*                 .type   = TOKEN_NEWLINE, */
/*                 .token  = { .as_newline = character }, */
/*                 .line   = context.line, */
/*                 .column = context.column */
/*             }; */
/*             array_append(&context.lexemes, lexeme, context.realloc); */

/*             ++context.line; */
/*             context.column = 0; */
/*         } break; */

/*         case '(': */
/*         case ')': { */
/*             try_append_multibyte_lexeme(&context); */

/*             Lexeme lexeme = { */
/*                 .type   = TOKEN_PARENTHESIS, */
/*                 .token  = { .as_parenthesis = character }, */
/*                 .line   = context.line, */
/*                 .column = context.column */
/*             }; */
/*             array_append(&context.lexemes, lexeme, context.realloc); */

/*             ++context.column; */
/*         } break; */

/*         case '{': */
/*         case '}': { */
/*             try_append_multibyte_lexeme(&context); */

/*             Lexeme lexeme = { */
/*                 .type   = TOKEN_BRACKET, */
/*                 .token  = { .as_bracket = character }, */
/*                 .line   = context.line, */
/*                 .column = context.column */
/*             }; */
/*             array_append(&context.lexemes, lexeme, context.realloc); */

/*             ++context.column; */
/*         } break; */

/*         case '"': { */
/*             try_append_multibyte_lexeme(&context); */
/*             lex_string(&context); */
/*         } break; */

/*         case '\'': { */
/*             try_append_multibyte_lexeme(&context); */
/*             lex_character_literal(&context); */
/*         } break; */

/*         case ' ': */
/*         case '\t': { */
/*             try_append_multibyte_lexeme(&context); */
/*             ++context.column; */
/*         } break; */

/*         default: { */
/*             if (MAX_TOKEN_SIZE <= context.token_buffer.count) { */
/*                 (void)fprintf( */
/*                      stderr, */
/*                      "%s(%zu:%zu): Error: Encountered token larger than the" */
/*                      "maximum allowed size %zu: %" SSTRING_PRINT, */
/*                      context.program_name, */
/*                      context.line, context.column, */
/*                      (size_t)MAX_TOKEN_SIZE, */
/*                      SSTRING_FORMAT(&context.token_buffer) */
/*                 ); */
/*                 exit(1); */
/*             } */

/*             if (SIZE_MAX == context.multibyte_line) { */
/*                 context.multibyte_line = context.line; */
/*             } */
/*             if (SIZE_MAX == context.multibyte_column) { */
/*                 context.multibyte_column = context.column; */
/*             } */
/*             sstring_append(&context.token_buffer, character, context.realloc); */

/*             ++context.column; */
/*         } break; */
/*         } */
/*     } */

/*     try_append_multibyte_lexeme(&context); */

/*     if (context.failed) exit(1); */

/*     sstring_free(&context.token_buffer, free); */
/*     return context.lexemes; */
/* } */

/* void dump_lexemes( FILE *restrict stream */
/*                  , const LexemeArray *restrict lexemes */
/*                  , const char *restrict program_name) { */
/*     for (size_t i = 0; i < lexemes->count; ++i) { */
/*         const Lexeme* lexeme = &array_at(lexemes, i); */

/*         switch (lexeme->type) { */
/*         case TOKEN_STRING: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s: \"", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type) */
/*             ); */

/*             for (size_t k = 0; k < lexeme->token.as_string.count; ++k) { */
/*                 char character = sstring_at(&lexeme->token.as_string, k); */
/*                 switch (character) { */
/*                 case '"': */
/*                 case '\\': { */
/*                     (void)fputc('\\', stream); */
/*                     (void)fputc(character, stream); */
/*                 } break; */

/*                 case '\n': { */
/*                     (void)fputs("\\n", stream); */
/*                 } break; */

/*                 case '\t': { */
/*                     (void)fputs("\\t", stream); */
/*                 } break; */

/*                 default: { */
/*                     (void)fputc(character, stream); */
/*                 } break; */
/*                 } */
/*             } */

/*             (void)fputs("\"\n", stream); */
/*         } break; */

/*         case TOKEN_CHARACTER: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s: '", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type) */
/*             ); */

/*             char character = lexeme->token.as_character; */

/*             switch (character) { */
/*             case '\'': */
/*             case '\\': { */
/*                 (void)fputc('\\', stream); */
/*                 (void)fputc(character, stream); */
/*             } break; */

/*             case '\n': { */
/*                 (void)fputs("\\n", stream); */
/*             } break; */

/*             case '\t': { */
/*                 (void)fputs("\\t", stream); */
/*             } break; */

/*             default: { */
/*                 (void)fputc(character, stream); */
/*             } break; */
/*             } */

/*             (void)fputs("'\n", stream); */
/*         } break; */

/*         case TOKEN_FLOAT: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s: %f\n", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type), */
/*                  lexeme->token.as_float */
/*             ); */
/*         } break; */

/*         case TOKEN_ATOM: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s: %" SSTRING_PRINT "\n", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type), */
/*                  SSTRING_FORMAT(&lexeme->token.as_atom) */
/*             ); */
/*         } break; */

/*         case TOKEN_NEWLINE: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s\n", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type) */
/*             ); */
/*         } break; */

/*         case TOKEN_PARENTHESIS: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s: %c\n", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type), */
/*                  lexeme->token.as_parenthesis */
/*             ); */
/*         } break; */

/*         case TOKEN_BRACKET: { */
/*             (void)fprintf( */
/*                  stream, */
/*                  "%s(%zu:%zu): %s: %c\n", */
/*                  program_name, lexeme->line, lexeme->column, */
/*                  token_type_name(lexeme->type), */
/*                  lexeme->token.as_bracket */
/*             ); */
/*         } break; */

/*         default: assert(false && "Unhandled token type"); */
/*         }; */
/*     } */
/* } */



typedef enum {
    VALUE_NUMBER,
    VALUE_CHARACTER,
    VALUE_ARRAY
} ValueType;

typedef struct Value Value;

typedef ARRAY_OF(Value) ValueArray;

struct Value {
    ValueType type;
    union {
        float64_t  as_number;
        uint8_t    as_character;
        ValueArray as_array;
    };
};



typedef enum {
    FUNCTION_NATIVE,
    FUNCTION_DEFINED,
    FUNCTION_LITERAL
} FunctionType;

typedef struct Function Function;

typedef ARRAY_OF(Function) FunctionArray;

struct Function {
    FunctionType type;
    union {
        void(*as_native)(ValueArray*);
        FunctionArray as_defined;
        Value         as_literal;
    };
};



// Adds values.
void native_pona(ValueArray* stack) {
    // TODO: make proper run time error.
    assert(stack->count >= 2 && "Stack Underflow");

    Value* a = &stack->elements[stack->count - 2];
    assert(a->type == VALUE_NUMBER && "TODO: handle non-number types");
    Value* b = &stack->elements[stack->count - 1];
    assert(b->type == VALUE_NUMBER && "TODO: handle non-number types");

    a->as_number = a->as_number + b->as_number;
    --stack->count;
}

// Subtracts values.
void native_ike(ValueArray* stack) {
    // TODO: make proper run time error.
    assert(stack->count >= 2 && "Stack Underflow");

    Value* a = &stack->elements[stack->count - 2];
    assert(a->type == VALUE_NUMBER && "TODO: handle non-number types");
    Value* b = &stack->elements[stack->count - 1];
    assert(b->type == VALUE_NUMBER && "TODO: handle non-number types");

    a->as_number = a->as_number - b->as_number;
    --stack->count;
}

void execute_functions(const FunctionArray* functions, ValueArray* stack) {
    for (size_t i = 0; i < functions->count; ++i) {
        const Function* function = &functions->elements[i];

        switch (function->type) {
        case FUNCTION_DEFINED: execute_functions(&function->as_defined, stack);     break;
        case FUNCTION_NATIVE:  function->as_native(stack);                          break;
        case FUNCTION_LITERAL: array_append(stack, function->as_literal, &realloc); break;
        default: assert(0 && "Encountered unexpected function type");
        };
    }
}



/* #define SOURCE_FILE     "test.tlpin" */
/* #define READ_CHUNK_SIZE 1024 */

void dump_stack(const ValueArray* stack) {
    for (size_t i = 0; i < stack->count; ++i) {
        const Value* value = &stack->elements[i];

        switch (value->type) {
        case VALUE_NUMBER:    (void)printf("%lf ", value->as_number);    break;
        case VALUE_CHARACTER: (void)printf("%c ",  value->as_character); break;

        case VALUE_ARRAY: {
            (void)printf("{ ");
            dump_stack(&value->as_array);
            (void)printf("} ");
        } break;

        default: assert(0 && "Encountered unexpected value type");
        };
    }
}

#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])

const Function initial_program[] = {
    {
        .type = FUNCTION_LITERAL,
        .as_literal = {
            .type = VALUE_NUMBER,
            .as_number = 30
        }
    },
    {
        .type = FUNCTION_LITERAL,
        .as_literal = {
            .type = VALUE_NUMBER,
            .as_number = 10
        }
    },
    {
        .type = FUNCTION_NATIVE,
        .as_native = &native_pona
    },
    {
        .type = FUNCTION_LITERAL,
        .as_literal = {
            .type = VALUE_NUMBER,
            .as_number = 20
        }
    },
    {
        .type = FUNCTION_NATIVE,
        .as_native = &native_ike
    }
};

int main(void) {
    /* FILE* source = fopen(SOURCE_FILE, "r"); */
    /* if (NULL == source) { */
    /*     perror("Error: Unable to open file '" SOURCE_FILE "'"); */
    /*     return 1; */
    /* }; */
    /* sstring_t contents = sstring_read_file(source, READ_CHUNK_SIZE, &realloc); */
    /* (void)fclose(source); */

    /* LexemeArray lexemes = lex_program(&contents, SOURCE_FILE, &free, &realloc); */
    /* sstring_free(&contents, &free); */

    /* dump_lexemes(stdout, &lexemes, SOURCE_FILE); */
    /* //cparse_program(&lexemes); */
    /* lexeme_array_free(&lexemes, &free); */

    ValueArray stack = {0};

    FunctionArray program = {0};
    array_append_many(&program, initial_program, ARRAY_SIZE(initial_program), &realloc);

    execute_functions(&program, &stack);

    (void)printf("Stack dump: ");
    dump_stack(&stack);

    return 0;
}
