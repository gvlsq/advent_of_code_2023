#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

static const int MAX_RED_CUBE_COUNT = 12;
static const int MAX_GREEN_CUBE_COUNT = 13;
static const int MAX_BLUE_CUBE_COUNT = 14;

enum Token_Type {
    TOKEN_TYPE_ERROR,

    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_INTEGER,

    TOKEN_TYPE_NULL_TERMINATOR,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_COLON,
    TOKEN_TYPE_SEMICOLON,

    TOKEN_TYPE_COUNT,
};
struct Token {
    Token_Type type;
    union {
        char *text;
        int integer_value;
    };
};

struct Tokenizer {
    char *at;
};

static char *read_text_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file '%s'\n", path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    int file_size = (int)ftell(file);
    fseek(file, 0, SEEK_SET);

    void *file_memory = malloc(file_size + 1);
    if (!file_memory) {
        fprintf(stderr, "Failed to allocate memory for text file\n");
        fclose(file);
        return 0;
    }

    fread(file_memory, file_size, 1, file);
    ((char *)file_memory)[file_size] = '\0';

    fclose(file);

    return (char *)file_memory;
}

static inline bool is_alphabetic(char c) {
    bool result = ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');

    return result;
}

static inline bool is_numeric(char c) {
    bool result = '0' <= c && c <= '9';

    return result;
}

static inline bool is_whitespace(char c) {
    bool result = c == ' ' || c == '\t';

    return result;
}

static inline bool is_eol(char c) {
    bool result = c == '\n' || c == '\r';

    return result;
}

static void eat_all_whitespace(Tokenizer *tokenizer) {
    while (*tokenizer->at) {
        if (is_whitespace(tokenizer->at[0]) || is_eol(tokenizer->at[0])) {
            tokenizer->at++;
        } else {
            break;
        }
    }
}

void skip_line(Tokenizer *tokenizer) {
    while (*tokenizer->at && !is_eol(tokenizer->at[0])) {
        tokenizer->at++;
    }

    eat_all_whitespace(tokenizer);
}

static void lex_integer(Tokenizer *tokenizer, Token *token) {
    char tmp[128];

    char *out = tmp;
    while (*tokenizer->at && is_numeric(*tokenizer->at)) {
        *out++ = *tokenizer->at++;
    }
    *out++ = '\0';

    int value = atoi(tmp);

    token->type = TOKEN_TYPE_INTEGER;
    token->integer_value = value;
}

static char *duplicate_string(char *s) {
    char *result = _strdup(s);

    return result;
}

static void lex_identifier(Tokenizer *tokenizer, Token *token) {
    char tmp[128];

    char *out = tmp;
    while (*tokenizer->at && is_alphabetic(*tokenizer->at)) {
        *out++ = *tokenizer->at++;
    }
    *out++ = '\0';

    token->type = TOKEN_TYPE_IDENTIFIER;
    token->text = duplicate_string(tmp);
}

static Token get_token(Tokenizer *tokenizer) {
    Token result = {};

    eat_all_whitespace(tokenizer);

    char c = tokenizer->at[0];
    switch (c) {
        case '\0':
            result.type = TOKEN_TYPE_NULL_TERMINATOR;
            tokenizer->at++;
            break;

        case ',':
            result.type = TOKEN_TYPE_COMMA;
            tokenizer->at++;
            break;

        case ':':
            result.type = TOKEN_TYPE_COLON;
            tokenizer->at++;
            break;

        case ';':
            result.type = TOKEN_TYPE_SEMICOLON;
            tokenizer->at++;
            break;

        default:
            if (is_numeric(c)) {
                lex_integer(tokenizer, &result);
            } else {
                lex_identifier(tokenizer, &result);
            }
            break;
    }

    return result;
}

static Token peek_token(Tokenizer *tokenizer) {
    Tokenizer tmp = *tokenizer;
    Token result = get_token(&tmp);

    return result;
}

static void eat(Tokenizer *tokenizer, Token_Type token_type) {
    Token token = get_token(tokenizer);

    assert(token.type == token_type);
}

static inline bool strings_match(char *a, const char *b) {
    bool result = strcmp(a, b) == 0;

    return result;
}

static bool parse_reveal(Tokenizer *tokenizer, int *red_count, int *green_count, int *blue_count) {
    bool valid = true;

    bool parsing = true;
    while (parsing) {
        Token peek = peek_token(tokenizer);
        switch (peek.type) {
            case TOKEN_TYPE_INTEGER: {
                Token count = get_token(tokenizer);
                Token color = get_token(tokenizer);

                peek = peek_token(tokenizer);
                if (peek.type == TOKEN_TYPE_COMMA) {
                    eat(tokenizer, TOKEN_TYPE_COMMA);
                }

                if (strings_match(color.text, "red")) {
                    *red_count = count.integer_value;
                } else if (strings_match(color.text, "green")) {
                    *green_count = count.integer_value;
                } else if (strings_match(color.text, "blue")) {
                    *blue_count = count.integer_value;
                }
            } break;

            case TOKEN_TYPE_SEMICOLON:
                eat(tokenizer, TOKEN_TYPE_SEMICOLON);

                parsing = false;
                break;

            default:
                parsing = false;
                break;
        }
    }

    return valid;
}

static int parse_game(Tokenizer *tokenizer) {
    Token game = get_token(tokenizer);
    Token id = get_token(tokenizer);

    eat(tokenizer, TOKEN_TYPE_COLON);

    int red_max = INT_MIN;
    int green_max = INT_MIN;
    int blue_max = INT_MIN;

    bool parsing = true;
    while (parsing) {
        Token token = peek_token(tokenizer);
        switch (token.type) {
            case TOKEN_TYPE_INTEGER: {
                int red_count, green_count, blue_count;
                parse_reveal(tokenizer, &red_count, &green_count, &blue_count);

                if (red_count > red_max)   red_max = red_count;
                if (green_count > green_max) green_max = green_count;
                if (blue_count > blue_max)  blue_max = blue_count;
            } break;

            default:
                parsing = false;
                break;
        }
    }

    int power = red_max*green_max*blue_max;

    return power;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    Tokenizer tokenizer;
    tokenizer.at = input;

    int sum = 0;

    bool parsing = true;
    while (parsing) {
        Token token = peek_token(&tokenizer);
        switch (token.type) {
            case TOKEN_TYPE_IDENTIFIER:
                sum += parse_game(&tokenizer);
                break;

            case TOKEN_TYPE_NULL_TERMINATOR:
                parsing = false;
                break;
        }
    }

    printf("The sum is %d\n", sum);

    return 0;
}
