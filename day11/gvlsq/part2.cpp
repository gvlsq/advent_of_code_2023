#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint64_t u64;

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

#define get_image_index(x, y) ((x) + image_width*(y))

static const int EXPANSION_INCREMENT = 1000000 - 1;

struct Vector2u {
    u64 x, y;
};

struct Galaxy {
    Vector2u position;
};

static int galaxy_count;
static Galaxy galaxies[1 << 9];

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
    bool result = c == '\n';

    return result;
}

static inline Vector2u vector2u(u64 x, u64 y) {
    Vector2u result;
    result.x = x;
    result.y = y;

    return result;
}

static void preprocess_input(char *input) {
    //
    // Original image
    //

    int image_height = 0;
    int image_width = 0;
    char *image = (char *)malloc(sizeof(char)*512*512);

    // Image width
    char *at = input;
    while (!is_eol(*at)) {
        image_width++;
        at++;
    }

    // Image data & height
    int index = 0;

    at = input;
    while (*at) {
        assert(index < 512*512);

        image[index] = *at;

        index++;
        at++;

        if (is_eol(*at)) {
            image_height++;
            at++;
        }
    }

    // 
    // Expanded image
    //

    // Gatherin' galaxies
    for (int y = 0; y < image_height; y++) {
        for (int x = 0; x < image_width; x++) {
            assert(galaxy_count < array_count(galaxies));

            if (image[get_image_index(x, y)] == '#') {
                Galaxy *galaxy = &galaxies[galaxy_count++];
                galaxy->position = vector2u(x, y);
            }
        }
    }

    // Rows
    u64 yshift = 0;

    for (u64 y = 0; y < image_height; y++) {
        bool found_galaxy = false;

        for (int x = 0; x < image_width; x++) {
            if (image[get_image_index(x, y)] == '#') {
                found_galaxy = true;
                break;
            }
        }

        if (!found_galaxy) {
            for (int i = 0; i < galaxy_count; i++) {
                Galaxy *galaxy = &galaxies[i];
                if (galaxy->position.y > (y + yshift)) {
                    galaxy->position.y += EXPANSION_INCREMENT;
                }
            }

            yshift += EXPANSION_INCREMENT;
        }
    }

    // Columns
    u64 xshift = 0;

    for (u64 x = 0; x < image_width; x++) {
        bool found_galaxy = false;

        for (int y = 0; y < image_height; y++) {
            if (image[get_image_index(x, y)] == '#') {
                found_galaxy = true;
                break;
            }
        }

        if (!found_galaxy) {
            for (int i = 0; i < galaxy_count; i++) {
                Galaxy *galaxy = &galaxies[i];
                if (galaxy->position.x > (x + xshift)) {
                    galaxy->position.x += EXPANSION_INCREMENT;
                }
            }

            xshift += EXPANSION_INCREMENT;
        }
    }
}

static inline u64 manhattan_distance(Vector2u a, Vector2u b) {
    u64 dx = a.x - b.x;
    u64 dy = a.y - b.y;

    u64 result = abs((int)dx) + abs((int)dy);

    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    //
    u64 sum = 0;

    for (int i = 0; i < galaxy_count - 1; i++) {
        Galaxy *a = &galaxies[i];
        for (int j = i + 1; j < galaxy_count; j++) {
            Galaxy *b = &galaxies[j];

            sum += manhattan_distance(a->position, b->position);
        }
    }

    printf("The sum is %I64u\n", sum);

    return 0;
}
