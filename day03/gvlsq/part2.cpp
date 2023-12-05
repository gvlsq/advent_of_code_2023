#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Vector2i {
    int x;
    int y;
};

struct Part_Number {
    int value;
    Vector2i position;
};

struct Symbol {
    char c;

    Vector2i position;

    int child_count;
    Part_Number *children[1 << 8];
};

static int symbol_count;
static Symbol symbols[1 << 11];

static int part_number_count;
static Part_Number part_numbers[1 << 11];

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

static inline bool is_numeric(char c) {
    bool result = '0' <= c && c <= '9';

    return result;
}

static inline bool is_eol(char c) {
    bool result = c == '\r' || c == '\n';

    return result;
}

static inline int string_length(char *s) {
    int result = strlen(s);

    return result;
}

static inline Vector2i vector2i(int x, int y) {
    Vector2i result;
    result.x = x;
    result.y = y;

    return result;
}

static inline int get_length(int i) {
    // This function is cheesy!

    int result = 0;

    if (i < 10) {
        result = 1;
    } else if (i < 100) {
        result = 2;
    } else if (i < 1000) {
        result = 3;
    }

    return result;
}

static inline bool is_adjacent(Vector2i a, Vector2i b) {
    bool north_south = a.x == b.x && (a.y == b.y - 1 || a.y == b.y + 1);
    bool east_west = a.y == b.y && (a.x == b.x - 1 || a.x == b.x + 1);

    bool northwest = a.x == b.x - 1 && a.y == b.y - 1;
    bool northeast = a.x == b.x + 1 && a.y == b.y - 1;
    bool southwest = a.x == b.x - 1 && a.y == b.y + 1;
    bool southeast = a.x == b.x + 1 && a.y == b.y + 1;

    return (north_south || east_west) || (northwest || northeast || southwest || southeast);
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    int line_index = 0;

    // Extracting symbol positions and part numbers from input
    char *at = input;
    while (*at) {
        char line[128];
        int matched = sscanf(at, "%s\n", &line);

        int line_length = string_length(line);

        for (int i = 0; i < line_length; i++) {
            if (line[i] != '.' && !is_numeric(line[i])) {
                assert(symbol_count < array_count(symbols));

                Symbol *symbol = &symbols[symbol_count++];
                symbol->c = line[i];
                symbol->position = vector2i(i, line_index);
                symbol->child_count = 0;
            }
        }

        char tmp[128];
        for (int i = 0; i < line_length; i++) {
            if (is_numeric(line[i])) {
                int cursor = i;

                while (is_numeric(line[cursor])) {
                    tmp[cursor - i] = line[cursor];
                    cursor++;
                }
                tmp[cursor - i] = '\0';

                assert(part_number_count < array_count(part_numbers));

                Part_Number *part_number = &part_numbers[part_number_count++];
                part_number->value = atoi(tmp);
                part_number->position = vector2i(i, line_index);

                i = cursor - 1;
            }
        }

        at += line_length + 1; // +1 for the newline
        line_index++;
    }

    // Filling out adjacencies for each symbol
    for (int i = 0; i < part_number_count; i++) {
        Part_Number *part_number = &part_numbers[i];

        int x = part_number->position.x;
        int y = part_number->position.y;

        bool valid = false;

        int value_length = get_length(part_number->value);
        for (int j = 0; (j < value_length) && !valid; j++) {
            for (int k = 0; k < symbol_count; k++) {
                Symbol *symbol = &symbols[k];

                if (is_adjacent(vector2i(x + j, y), symbol->position)) {
                    assert(symbol->child_count < array_count(symbol->children));
                    symbol->children[symbol->child_count++] = part_number;

                    valid = true;
                    break;
                }
            }
        }
    }

    //
    int sum = 0;

    for (int i = 0; i < symbol_count; i++) {
        Symbol *symbol = &symbols[i];

        if (symbol->c == '*' && symbol->child_count == 2) { // Definition of a gear
            sum += symbol->children[0]->value*symbol->children[1]->value;
        }
    }

    printf("The sum is %d\n", sum);

    return 0;
}
