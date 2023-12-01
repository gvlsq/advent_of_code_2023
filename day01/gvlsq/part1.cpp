#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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
        fprintf(stderr, "Failed to allocate memory when reading text file\n");
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

static inline int to_int(char c) {
    assert(is_numeric(c));

    int result = c - '0';

    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    int sum = 0;

    char *at = input;
    while (*at) {
        bool first_digit_seen = false;
        bool last_digit_seen = false;

        int first_digit = 0;
        int last_digit = 0;

        while (*at != '\n') {
            if (is_numeric(*at)) {
                int digit = to_int(*at);

                if (!first_digit_seen) {
                    first_digit = digit;
                    first_digit_seen = true;
                } else {
                    last_digit = digit;
                    last_digit_seen = true;
                }
            }

            at++;
        }
        at++; // Skipping the newline

        int calibration_value;
        if (!last_digit_seen) {
            calibration_value = first_digit*10 + first_digit;
        } else {
            calibration_value = first_digit*10 + last_digit;
        }

        sum += calibration_value;
    }

    printf("The sum is %d\n", sum);

    return 0;
}
