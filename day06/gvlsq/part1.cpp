#include <stdio.h>
#include <stdlib.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Stream {
    char *at;
};

struct Race {
    int duration;
    int record_distance;
};

static Race races[4];

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
    int matched = sscanf(stream.at, "Time: %d %d %d %d\n%n", &races[0].duration, &races[1].duration, &races[2].duration, &races[3].duration, &chars_read);

    stream.at += chars_read;

    matched = sscanf(stream.at, "Distance: %d %d %d %d\n", &races[0].record_distance, &races[1].record_distance, &races[2].record_distance, &races[3].record_distance);
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    //
    int tota1 = 1;

    for (int i = 0; i < array_count(races); i++) {
        Race *race = &races[i];

        int ways_to_beat_record = 0;

        for (int j = 0; j <= race->duration; j++) {
            int distance_traveled = j*(race->duration - j);
            if (distance_traveled > race->record_distance) {
                ways_to_beat_record++;
            }
        }

        tota1 *= ways_to_beat_record;
    }

    printf("The total is %d\n", tota1);

    return 0;
}
