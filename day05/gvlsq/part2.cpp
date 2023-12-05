#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;

#define U64_MAX UINT64_MAX

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Stream {
    char *at;
};

struct Initial_Seed_Range {
    u64 start;
    u64 length;
};

struct Map_Line {
    u64 dest_range_start;
    u64 source_range_start;
    u64 range_length;
};
struct Map {
    int line_count;
    Map_Line lines[1 << 6];
};

static u64 initial_seed_range_count;
static Initial_Seed_Range initial_seed_ranges[1 << 5];

static Map seed_to_soil_map;
static Map soil_to_fertilizer_map;
static Map fertilizer_to_water_map;
static Map water_to_light_map;
static Map light_to_temperature_map;
static Map temperature_to_humidity_map;
static Map humidity_to_location_map;

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

static int string_length(const char *s) {
    int result = strlen(s);

    return result;
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

static void preprocess_initial_seeds(Stream *stream) {
    stream->at += string_length("seeds:") + 1; // title + whitespace

    u64 tmp[2];
    int chars_read;
    while (sscanf(stream->at, "%I64u %I64u%n", &tmp[0], &tmp[1], &chars_read) == 2) {
        assert(initial_seed_range_count < array_count(initial_seed_ranges));

        Initial_Seed_Range *range = &initial_seed_ranges[initial_seed_range_count++];
        range->start = tmp[0];
        range->length = tmp[1];

        stream->at += chars_read;
    }
}

static void preprocess_map(Stream *stream, Map *map, const char *title) {
    eat_eol(stream);

    stream->at += string_length(title) + 1 + 1; // title + colon + newline

    u64 tmp[3];
    int chars_read;
    while (sscanf(stream->at, "%I64u %I64u %I64u%n\n", &tmp[0], &tmp[1], &tmp[2], &chars_read) == 3) {
        assert(map->line_count < array_count(map->lines));

        Map_Line *map_line = &map->lines[map->line_count++];
        map_line->dest_range_start = tmp[0];
        map_line->source_range_start = tmp[1];
        map_line->range_length = tmp[2];

        stream->at += chars_read;
    }
}

static void preprocess_input(char *input) {
    Stream stream;
    stream.at = input;

    preprocess_initial_seeds(&stream);

    preprocess_map(&stream, &seed_to_soil_map, "seed-to-soil map");
    preprocess_map(&stream, &soil_to_fertilizer_map, "soil-to-fertilizer map");
    preprocess_map(&stream, &fertilizer_to_water_map, "fertilizer-to-water map");
    preprocess_map(&stream, &water_to_light_map, "water-to-light map");
    preprocess_map(&stream, &light_to_temperature_map, "light-to-temperature map");
    preprocess_map(&stream, &temperature_to_humidity_map, "temperature-to-humidity map");
    preprocess_map(&stream, &humidity_to_location_map, "humidity-to-location map");
}

static u64 map_source_value(Map *map, u64 source_value) {
    u64 result = source_value;

    for (int i = 0; i < map->line_count; i++) {
        Map_Line *map_line = &map->lines[i];

        u64 source_range_min = map_line->source_range_start;
        u64 source_range_max = map_line->source_range_start + map_line->range_length; // Exclusive

        if (source_range_min <= source_value && source_value < source_range_max) {
            u64 d = source_value - source_range_min;
            u64 new_value = map_line->dest_range_start + d;

            result = new_value;

            break;
        }
    }

    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    //
    u64 min_location = U64_MAX;

    for (int i = 0; i < initial_seed_range_count; i++) {
        Initial_Seed_Range *range = &initial_seed_ranges[i];

        for (u64 j = 0; j < range->length; j++) {
            u64 source_value = range->start + j;

            source_value = map_source_value(&seed_to_soil_map, source_value);
            source_value = map_source_value(&soil_to_fertilizer_map, source_value);
            source_value = map_source_value(&fertilizer_to_water_map, source_value);
            source_value = map_source_value(&water_to_light_map, source_value);
            source_value = map_source_value(&light_to_temperature_map, source_value);
            source_value = map_source_value(&temperature_to_humidity_map, source_value);
            source_value = map_source_value(&humidity_to_location_map, source_value);

            if (source_value < min_location) {
                min_location = source_value;
            }
        }
    }

    printf("The lowest location is %I64u\n", min_location);

    return 0;
}
