#include "parser_internal.h"
#include "cTooling.h"
#include "ctype.h"

// Buffer functions
static bool is_at_end(Lexer *lexer) {
    return lexer->buffer[lexer->current] == '\0';
}

static char next(Lexer *lexer) {
    lexer->col.current++;
    return lexer->buffer[lexer->current++];
}

static char peek(Lexer *lexer) {
    return lexer->buffer[lexer->current];
}

static bool match(Lexer *lexer, char c) {
    if(peek(lexer) == c) {
        next(lexer);
        return true;
    }

    return false;
}

static char prev(Lexer *lexer) {
    assert(lexer->current > 0);
    return lexer->buffer[lexer->current - 1];
}

// Tokens functions

// prepares the lexer to add the next token
static void begin_new_tkn(Lexer *lexer) {
    lexer->col.start = lexer->col.current;
    lexer->start = lexer->current;
}

static void add_tkn(Lexer *lexer, TokenType type, char *lexeme) {
    Token *token = arena_alloc(&lexer->arena, sizeof(Token));
    token->type = type;
    token->lexeme = lexeme;
    token->col = (TokenCol){lexer->col.start, lexer->col.current};
    token->line = lexer->line;
    token->prev = NULL;
    token->next = NULL;

    if(lexer->tokens.count > 0) {
        lexer->tokens.tail->next = token;
        token->prev = lexer->tokens.tail;
    } else {
        lexer->tokens.head = token;
    }

    lexer->tokens.tail = token;
    lexer->tokens.count++;
}

static void add_basic_tkn(Lexer *lexer, TokenType type) {
    add_tkn(lexer, type, NULL);
}

// Error functions
// prints error at the lexer's line and marks the error as a range using lexer's col
static void range_error(Lexer *lexer, const char *msg) {
    print_range_error(msg, (ErrorSrc){
        .line = lexer->line,
        .col = {lexer->col.start, lexer->col.current},
        .src = lexer->buffer,
    });
    lexer->had_errors = true;
}

static void error_at_current(Lexer *lexer, const char *msg) {
    print_range_error(msg, (ErrorSrc){
        .line = lexer->line,
        .col = {lexer->col.current - 1, lexer->col.current},
        .src = lexer->buffer,
    });
    lexer->had_errors = true;
}

// Other
static char *allocate_lexeme(Lexer *lexer, int start, int end) {
    int len = end - start + 1;
    char *lexeme = arena_alloc(&lexer->arena, len);

    memcpy(
        lexeme,
        lexer->buffer + start,
        len
    );
    lexeme[len - 1] = '\0';
    return lexeme;
}

// Lexing functions

static void string(Lexer *lexer) {
    while(!is_at_end(lexer)) {
        char c = peek(lexer);
        if(c == '\n' || c == '"') break;
        next(lexer);
    }

    if(!match(lexer, '"')) {
        range_error(lexer, "Expected terminating \"");
        return;
    }

    char *lexeme = allocate_lexeme(
        lexer,
        // +1 to remove the preceding "
        lexer->start + 1,
        // -1 to remove the ending "
        lexer->current - 1
    );

    add_tkn(lexer, STRING_TKN, lexeme);
}

static void number(Lexer *lexer) {
    if(prev(lexer) == '-' && !isdigit(peek(lexer))) {
        error_at_current(lexer, "Expected digit after \"-\"");
        return;
    }

    while(isdigit(peek(lexer))) next(lexer);

    if(match(lexer, '.')) {
        if(!isdigit(peek(lexer))) {
            error_at_current(lexer, "Expected digit after \".\"");
            return;
        }

        while(isdigit(peek(lexer))) next(lexer);
    }

    if(match(lexer, 'e')) {
        if(!isdigit(peek(lexer))) {
            error_at_current(lexer, "Expected digit after \"e\"");
            return;
        }

        while(isdigit(peek(lexer))) next(lexer);
    }

    char *lexeme = allocate_lexeme(lexer, lexer->start, lexer->current);

    add_tkn(lexer, NUMBER_TKN, lexeme);
}

static void keyword(Lexer *lexer) {
    while(isalpha(peek(lexer))) next(lexer);

    char *keyword = strndup(lexer->buffer + lexer->start, lexer->current - lexer->start);

    if(strcmp(keyword, "false") == 0) {
        add_basic_tkn(lexer, FALSE_TKN);
    } else if(strcmp(keyword, "true") == 0) {
        add_basic_tkn(lexer, TRUE_TKN);
    } else if(strcmp(keyword, "null") == 0) {
        add_basic_tkn(lexer, NULL_TKN);
    } else {
        const char *msg = text_format("Unknown keyword \"%s\"", keyword);
        range_error(lexer, msg);
    }

    free(keyword);
}

// Public functions

bool lexer_scan(Lexer *lexer) {
    while(!is_at_end(lexer)) {
        begin_new_tkn(lexer);

        char c = next(lexer);

        switch(c) {
            case '{': add_basic_tkn(lexer, OPEN_BRACE_TKN); break;
            case '}': add_basic_tkn(lexer, CLOSE_BRACE_TKN); break;
            case ':': add_basic_tkn(lexer, COLON_TKN); break;
            case '[': add_basic_tkn(lexer, OPEN_BRACKET_TKN); break;
            case ']': add_basic_tkn(lexer, CLOSE_BRACKET_TKN); break;
            case ',': add_basic_tkn(lexer, COMMA_TKN); break;
            case '"': string(lexer); break;
            case '\n':
                lexer->line++;
                lexer->col.start = lexer->col.current = 0;
                break;
            case '\t':
            case ' ':
            case '\r':
                break;
            default: {
                if(isalpha(c)) {
                    keyword(lexer);
                } else if(isdigit(c) || c == '-') {
                    number(lexer);
                } else {
                    const char *msg = text_format("Unexpected character \"%c\"", c);
                    error_at_current(lexer, msg);
                }
            }
        }
    }

    return true;
}

void lexer_init(Lexer *lexer) {
    lexer->line = 1;
}

void lexer_free(Lexer *lexer) {
    arena_free(&lexer->arena);
}
