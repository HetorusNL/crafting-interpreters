#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <src/common.h>
#include <src/scanner.h>

typedef struct Scanner {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

Token skip_whitespace_error_token;

void init_scanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

static char peek() { return *scanner.current; }

static bool is_at_end() { return peek() == '\0'; }

static char peek_next() {
    if (is_at_end())
        return '\0';
    return scanner.current[1];
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static bool match(char expected) {
    if (is_at_end())
        return false;
    if (peek() != expected)
        return false;
    scanner.current++;
    return true;
}

static Token make_token(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token error_token(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

// returns true when it fills the `skip_whitespace_error_token`
static bool skip_whitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            scanner.line++;
            advance();
            break;
        case '/':
            if (peek_next() == '/') {
                // a comment continues till the end of the line
                while (peek() != '\n' && !is_at_end())
                    advance();
            } else if (peek_next() == '*') {
                // advance the '/' and /*/
                advance();
                advance();
                // a block comment continues until a terminating '*/'
                while ((peek() != '*' || peek_next() != '/') && !is_at_end()) {
                    if (advance() == '\n') {
                        scanner.line++;
                    }
                    if (is_at_end()) {
                        // use the error token to pass the error to the scan_token function
                        skip_whitespace_error_token = error_token("Unterminated block comment.");
                        return true;
                    }
                }
                // advance the '*' and '/'
                advance();
                advance();
            } else {
                return false;
            }
            break;
        default:
            return false;
        }
    }
}

static TokenType check_keyword(int start, int length, const char* rest, TokenType type) {
    if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, (size_t)length) == 0)
        return type;

    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type() {
    switch (*scanner.start) {
    case 'a':
        return check_keyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return check_keyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return check_keyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
            case 'a':
                return check_keyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return check_keyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return check_keyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return check_keyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return check_keyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return check_keyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return check_keyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return check_keyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return check_keyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
            case 'h':
                return check_keyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return check_keyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return check_keyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (is_alpha(peek()) || is_digit(peek()))
        advance();
    return make_token(identifier_type());
}

static Token number() {
    while (is_digit(peek()))
        advance();

    // look for an optional fractional part
    if (peek() == '.' && is_digit(peek_next())) {
        // consume the '.'
        advance();

        while (is_digit(peek()))
            advance();
    }

    return make_token(TOKEN_NUMBER);
}

static Token string() {
    while (peek() != '"' && !is_at_end()) {
        if (advance() == '\n')
            scanner.line++;
    }

    if (is_at_end())
        return error_token("Unterminated string.");

    // the closing quote
    advance();
    return make_token(TOKEN_STRING);
}

Token scan_token() {
    bool has_error = skip_whitespace();
    if (has_error)
        return skip_whitespace_error_token;

    scanner.start = scanner.current;

    if (is_at_end())
        return make_token(TOKEN_EOF);

    char c = advance();
    if (is_alpha(c))
        return identifier();
    if (is_digit(c))
        return number();

    switch (c) {
    // single-character tokens
    case '(':
        return make_token(TOKEN_LEFT_PAREN);
    case ')':
        return make_token(TOKEN_RIGHT_PAREN);
    case '{':
        return make_token(TOKEN_LEFT_BRACE);
    case '}':
        return make_token(TOKEN_RIGHT_BRACE);
    case ',':
        return make_token(TOKEN_COMMA);
    case '.':
        return make_token(TOKEN_DOT);
    case '-':
        return make_token(TOKEN_MINUS);
    case '+':
        return make_token(TOKEN_PLUS);
    case ';':
        return make_token(TOKEN_SEMICOLON);
    case '/':
        return make_token(TOKEN_SLASH);
    case '*':
        return make_token(TOKEN_STAR);
    // one or more character tokens
    case '!':
        return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '>':
        return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '<':
        return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '"':
        return string();
    }

    return error_token("Unexpected character.");
}
