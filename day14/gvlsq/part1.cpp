#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

#define Invalid_Code_Path assert(!"Invalid code path")
#define Invalid_Default_Case default: Invalid_Code_Path; break;

struct Vector2i {
    int x, y;
};

enum Rock_Type {
    ROCK_TYPE_ROUNDED,
    ROCK_TYPE_CUBE,
};
struct Rock {
    Rock_Type type;
    Vector2i position;
};

static int rock_count;
static Rock rocks[1 << 12];

static int platform_width;
static int platform_height;

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

static inline Vector2i vector2i(int x, int y) {
    Vector2i result;
    result.x = x;
    result.y = y;
    
    return result;
}

static void preprocess_input(char *input) {
    int x = 0;
    int y = 0;
    
    char *at = input;
    while (*at) {
        char c = *at;
        switch (c) {
            case 'O': {
                assert(rock_count < array_count(rocks));
                
                Rock *rock = &rocks[rock_count++];
                rock->type = ROCK_TYPE_ROUNDED;
                rock->position = vector2i(x, y);
            } break;
            
            case '#': {
                assert(rock_count < array_count(rocks));
                
                Rock *rock = &rocks[rock_count++];
                rock->type = ROCK_TYPE_CUBE;
                rock->position = vector2i(x, y);
            } break;
            
            case '.': {
                
            } break;
            
            Invalid_Default_Case;
        }
        
        x++;
        
        at++;
        if (*at == '\n') {
            platform_width = x;
            
            x = 0;
            y++;
            
            at++;
        }
    }
    
    platform_height = y;
}

static void shift_rounded_rocks_up(int y) {
    for (int i = rock_count - 1; i >= 0; i--) {
        Rock *rock = &rocks[i];
        if (rock->type == ROCK_TYPE_ROUNDED && rock->position.y == y) {
            int xx = rock->position.x;
            int yy = y - 1;
            
            bool found_obstruction = false;
            {
                for (int j = 0; j < rock_count; j++) {
                    if (i == j) continue;
                    
                    Rock *other = &rocks[j];
                    if (other->position.x == xx && other->position.y == yy) {
                        found_obstruction = true;
                    }
                }
            }
            
            if (!found_obstruction) {
                rock->position = vector2i(xx, yy);
            }
        }
    }
}

static int count_rounded_rocks_with_y(int y) {
    int result = 0;
    
    for (int i = 0; i < rock_count; i++) {
        Rock *rock = &rocks[i];
        if (rock->type == ROCK_TYPE_ROUNDED && rock->position.y == y) {
            result++;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);
    
    preprocess_input(input);
    
    //
    int total = 0;
    
    for (int i = 0; i < platform_height - 1; i++) {
        for (int y = 1; y < platform_height; y++) {
            shift_rounded_rocks_up(y);
        }
    }
    
    for (int y = platform_height - 1; y >= 0; y--) {
        int count = count_rounded_rocks_with_y(y);
        
        total += (platform_height - y)*count;
    }
    
    printf("The total is %d\n", total);
    
    return 0;
}
