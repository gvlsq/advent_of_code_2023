#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Scratchcard {
    int id;
    int winning_numbers[10];
    int numbers_i_have[25];
};

static int scratchcard_count;
static Scratchcard scratchcards[1 << 8];

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

static inline bool is_eol(char c) {
    bool result = c == '\r' || c == '\n';

    return result;
}

static bool is_winning_number(Scratchcard *scratchcard, int number) {
    for (int i = 0; i < array_count(scratchcard->winning_numbers); i++) {
        if (scratchcard->winning_numbers[i] == number) {
            return true;
        }
    }

    return false;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    // Processing scratchcards
    char *at = input;
    while (*at) {
        assert(scratchcard_count < array_count(scratchcards));
        Scratchcard *scratchard = &scratchcards[scratchcard_count++];

        int matched = sscanf(at, "Card %d: %d %d %d %d %d %d %d %d %d %d | %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            &scratchard->id,
            &scratchard->winning_numbers[0], &scratchard->winning_numbers[1], &scratchard->winning_numbers[2], &scratchard->winning_numbers[3], &scratchard->winning_numbers[4],
            &scratchard->winning_numbers[5], &scratchard->winning_numbers[6], &scratchard->winning_numbers[7], &scratchard->winning_numbers[8], &scratchard->winning_numbers[9],
            &scratchard->numbers_i_have[0], &scratchard->numbers_i_have[1], &scratchard->numbers_i_have[2], &scratchard->numbers_i_have[3], &scratchard->numbers_i_have[4],
            &scratchard->numbers_i_have[5], &scratchard->numbers_i_have[6], &scratchard->numbers_i_have[7], &scratchard->numbers_i_have[8], &scratchard->numbers_i_have[9],
            &scratchard->numbers_i_have[10], &scratchard->numbers_i_have[11], &scratchard->numbers_i_have[12], &scratchard->numbers_i_have[13], &scratchard->numbers_i_have[14],
            &scratchard->numbers_i_have[15], &scratchard->numbers_i_have[16], &scratchard->numbers_i_have[17], &scratchard->numbers_i_have[18], &scratchard->numbers_i_have[19],
            &scratchard->numbers_i_have[20], &scratchard->numbers_i_have[21], &scratchard->numbers_i_have[22], &scratchard->numbers_i_have[23], &scratchard->numbers_i_have[24]);

        // Skipping the line
        while (!is_eol(at[0])) at++;
        while (is_eol(at[0])) at++;
    }

    //
    int sum = 0;

    for (int i = 0; i < scratchcard_count; i++) {
        Scratchcard *scratchcard = &scratchcards[i];

        int points = 0;

        for (int j = 0; j < array_count(scratchcard->numbers_i_have); j++) {
            if (is_winning_number(scratchcard, scratchcard->numbers_i_have[j])) {
                if (points == 0) {
                    points = 1;
                } else {
                    points *= 2;
                }
            }
        }

        sum += points;
    }

    printf("The sum is %d\n", sum);

    return 0;
}
