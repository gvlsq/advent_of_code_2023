#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint64_t u64;

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Stream {
    char *at;
};

struct Race {
    u64 duration;
    u64 record_distance;
};

static Race race;

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
    bool result = c == '\n' || c == '\r';

    return result;
}

static void eat_eol(Stream *stream) {
    while (*stream->at) {
        if (is_eol(stream->at[0])) {
            stream->at++;
        } else {
            break;
        }
    }
}

static void preprocess_input(char *input) {
    Stream stream;
    stream.at = input;
    
    int chars_read;

    int tmp0[4];
    int matched = sscanf(stream.at, "Time: %d %d %d %d\n%n", &tmp0[0], &tmp0[1], &tmp0[2], &tmp0[3], &chars_read);

    stream.at += chars_read;

    int tmp1[4];
    matched = sscanf(stream.at, "Distance: %d %d %d %d\n", &tmp1[0], &tmp1[1], &tmp1[2], &tmp1[3]);

    // 

    u64 duration = 0;
    for (int i = 0; i < array_count(tmp0); i++) {
        duration *= 100;
        duration += tmp0[i];
    }

    u64 record_distance = 0;
    for (int i = 0; i < array_count(tmp1); i++) {
        record_distance *= 10000;
        record_distance += tmp1[i];
    }

    race.duration = duration;
    race.record_distance = record_distance;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    //
    int total = 0;

    for (u64 i = 0; i <= race.duration; i++) {
        u64 distance_traveled = i*(race.duration - i);
        if (distance_traveled > race.record_distance) {
            total++;
        }
    }

    printf("The total is %d\n", total);

    return 0;
}
