#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Record_Line {
    char text[1 << 5];

    int group_count;
    int groups[1 << 3];
};

struct Line_Stack {
    int depth;
    Record_Line lines[1 << 5];
};

static int line_count;
static Record_Line lines[1 << 10];

static Line_Stack line_stack;

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

static char *duplicate_string(char *s) {
    char *result = _strdup(s);

    return result;
}

static int string_length(char *s) {
    int result = strlen(s);

    return result;
}

static inline bool is_eol(char c) {
    bool result = c == '\n';

    return result;
}

static void preprocess_input(char *input) {
    char *at = input;

    int chars_read;

    char tmp0[128];
    while (sscanf(at, "%[.#?]%n", &tmp0, &chars_read) == 1) {
        assert(line_count < array_count(lines));

        Record_Line *line = &lines[line_count++];
        strncpy(line->text, tmp0, string_length(tmp0));

        at += chars_read;

        int tmp1;
        while (sscanf(at, "%d%n", &tmp1, &chars_read) == 1) {
            assert(line->group_count < array_count(line->groups));

            line->groups[line->group_count++] = tmp1;

            at += chars_read;

            if (*at == ',') at++;

            if (is_eol(*at)) {
                at++;
                break;
            }
        }
    }
}

static void push_line(Record_Line line) {
    assert(line_stack.depth < array_count(line_stack.lines));
    line_stack.lines[line_stack.depth++] = line;
}

static Record_Line pop_line(void) {
    assert(line_stack.depth > 0);

    Record_Line result = line_stack.lines[--line_stack.depth];

    return result;
}

static bool arrangement_is_valid(Record_Line *line, Record_Line *arranged) {
    int valid_count_group = 0;

    int ai = 0;
    for (int i = 0; i < line->group_count; i++) {
        int group = line->groups[i];

        for (; ai < string_length(arranged->text); ai++) {
            if (arranged->text[ai] == '#') {
                int counter = 0;
                while ((ai < string_length(arranged->text)) && (arranged->text[ai] == '#')) {
                    counter++;
                    ai++;
                }

                bool valid = counter == group;
                if (!valid) return false; // We bail after the first invalid group we see

                valid_count_group++;
                break;
            }
        }
    }

    int straggler_count = 0; // Checking for any #s after the valid groups we found
    for (; ai < string_length(arranged->text); ai++) {
        if (arranged->text[ai] == '#') straggler_count++;
    }

    return (straggler_count == 0) && (line->group_count == valid_count_group);
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    //
    int sum = 0;

    for (int i = 0; i < line_count; i++) {
        Record_Line *line = &lines[i];

        push_line(*line);

        while (line_stack.depth) {
            Record_Line popped = pop_line();

            if (arrangement_is_valid(line, &popped)) {
                sum++;
                continue;
            }

            for (int i = 0; i < string_length(popped.text); i++) {
                if (popped.text[i] == '?') {
                    Record_Line tmp0; // ? => #
                    memcpy(&tmp0, &popped, sizeof(popped));

                    Record_Line tmp1; // ? => .
                    memcpy(&tmp1, &popped, sizeof(popped));

                    tmp0.text[i] = '#';
                    tmp1.text[i] = '.';

                    push_line(tmp0);
                    push_line(tmp1);

                    break;
                }
            }
        }
    }

    printf("The sum is %d\n", sum);

    return 0;
}
