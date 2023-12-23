#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

#define get_tile_index(x, y) ((x) + (y)*(pattern->width))

struct Pattern {
    int width;
    int height;
    
    char *memory;
};

static int pattern_count;
static Pattern patterns[1 << 8];

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

static void preprocess_input(char *input) {
    char *at = input;
    while (*at) {
        assert(pattern_count < array_count(patterns));
        Pattern *pattern = &patterns[pattern_count++];
        
        size_t memory_size = 1 << 16;
        size_t memory_cursor = 0;
        
        pattern->memory = (char *)malloc(memory_size);
        
        int chars_read = 0;
        int lines_read = 0;
        
        char tmp0[1 << 7];
        while (sscanf(at, "%[.#]%n", &tmp0, &chars_read) == 1) {
            assert(memory_cursor < memory_size);
            
            memcpy(pattern->memory + memory_cursor, tmp0, chars_read);
            memory_cursor += chars_read;
            
            at += chars_read;
            if (*at == '\n') {
                pattern->width = chars_read;
                at++;
            }
            
            lines_read++;
        }
        
        if (!(*at) || *at == '\n') {
            pattern->height = lines_read;
            
            if (*at == '\n') at++; // Double newline only!
        }
    }
}

static bool vertical_columns_match(Pattern *pattern, int x0, int x1) {
    for (int y = 0; y < pattern->height; y++) {
        char a = pattern->memory[get_tile_index(x0, y)];
        char b = pattern->memory[get_tile_index(x1, y)];
        
        if (a != b) return false;
    }
    
    return true;
}

static bool has_vertical_reflection(Pattern *pattern, int *left_column_count) {
    for (int x = 0; x < pattern->width - 1; x++) {
        int x0 = x;
        int x1 = x + 1;
        
        while (vertical_columns_match(pattern, x0, x1)) {
            x0--;
            x1++;
            
            if (x0 < 0 || x1 == pattern->width) { // If either X hits a boundary, success
                *left_column_count = x + 1;
                return true;
            }
        }
    }
    
    return false;
}

static bool horizontal_columns_match(Pattern *pattern, int y0, int y1) {
    for (int x = 0; x < pattern->width; x++) {
        char a = pattern->memory[get_tile_index(x, y0)];
        char b = pattern->memory[get_tile_index(x, y1)];
        
        if (a != b) return false;
    }
    
    return true;
}

static bool has_horizontal_reflection(Pattern *pattern, int *above_column_count) {
    for (int y = 0; y < pattern->height - 1; y++) {
        int y0 = y;
        int y1 = y + 1;
        
        while (horizontal_columns_match(pattern, y0, y1)) {
            y0--;
            y1++;
            
            if (y0 < 0 || y1 == pattern->height) { // If either Y hits a boundary, success
                *above_column_count = y + 1;
                return true;
            }
        }
    }
    
    return false;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);
    
    preprocess_input(input);
    
    //
    int total = 0;
    
    for (int i = 0; i < pattern_count; i++) {
        Pattern *pattern = &patterns[i];
        
        int left_column_count;
        if (has_vertical_reflection(pattern, &left_column_count)) {
            total += left_column_count;
        }
    }
    
    for (int i = 0; i < pattern_count; i++) {
        Pattern *pattern = &patterns[i];
        
        int above_column_count;
        if (has_horizontal_reflection(pattern, &above_column_count)) {
            total += above_column_count*100;
        }
    }
    
    printf("The total is %d\n", total);
    
    return 0;
}
