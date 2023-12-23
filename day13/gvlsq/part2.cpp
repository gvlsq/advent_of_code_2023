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

enum Reflection_Type {
    REFLECTION_VERTICAL,
    REFLECTION_HORIZONTAL,
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

static void get_vertical_reflections(Pattern *pattern,
                                     int *reflection_count, int *left_column_counts) {
    for (int x = 0; x < pattern->width - 1; x++) {
        int x0 = x;
        int x1 = x + 1;
        
        while (vertical_columns_match(pattern, x0, x1)) {
            x0--;
            x1++;
            
            if (x0 < 0 || x1 == pattern->width) { // If either X hits a boundary, success
                assert(*reflection_count < 64);
                
                left_column_counts[*reflection_count] = x + 1;
                (*reflection_count)++;
                
                break;
            }
        }
    }
}

static bool horizontal_columns_match(Pattern *pattern, int y0, int y1) {
    for (int x = 0; x < pattern->width; x++) {
        char a = pattern->memory[get_tile_index(x, y0)];
        char b = pattern->memory[get_tile_index(x, y1)];
        
        if (a != b) return false;
    }
    
    return true;
}

static void get_horizontal_reflections(Pattern *pattern,
                                       int *reflection_count, int *above_column_counts) {
    for (int y = 0; y < pattern->height - 1; y++) {
        int y0 = y;
        int y1 = y + 1;
        
        while (horizontal_columns_match(pattern, y0, y1)) {
            y0--;
            y1++;
            
            if (y0 < 0 || y1 == pattern->height) { // If either Y hits a boundary, success
                assert(*reflection_count < 64);
                
                above_column_counts[*reflection_count] = y + 1;
                (*reflection_count)++;
                
                break;
            }
        }
    }
}

static inline char opposite(char c) {
    assert(c == '#' || c == '.');
    
    if (c == '#') return '.';
    else if (c == '.') return '#';
}

static void summarize_and_accept_first(Pattern *pattern,
                                       Reflection_Type *reflection_type, int *value) {
    int vertical_reflection_count = 0;
    int left_column_counts[1 << 6];
    get_vertical_reflections(pattern,
                             &vertical_reflection_count,
                             left_column_counts);
    
    if (vertical_reflection_count) {
        *reflection_type = REFLECTION_VERTICAL;
        *value = left_column_counts[0];
        return;
    }
    
    int horizontal_reflection_count = 0;
    int above_column_counts[1 << 6];
    get_horizontal_reflections(pattern,
                               &horizontal_reflection_count,
                               above_column_counts);
    if (horizontal_reflection_count) {
        *reflection_type = REFLECTION_HORIZONTAL;
        *value = above_column_counts[0];
        return;
    }
}

static bool summarize_and_reject(Pattern *pattern,
                                 Reflection_Type old_reflection_type, int old_value,
                                 Reflection_Type *new_reflection_type, int *new_value) {
    int vertical_reflection_count = 0;
    int left_column_counts[1 << 6];
    get_vertical_reflections(pattern,
                             &vertical_reflection_count,
                             left_column_counts);
    
    int horizontal_reflection_count = 0;
    int above_column_counts[1 << 6];
    get_horizontal_reflections(pattern,
                               &horizontal_reflection_count,
                               above_column_counts);
    
    //
    // Seeing if there are any reflections that don't match the old one
    //
    
    // Vertical
    for (int i = 0; i < vertical_reflection_count; i++) {
        int left_column_count = left_column_counts[i];
        
        if (old_reflection_type == REFLECTION_HORIZONTAL) {
            *new_reflection_type = REFLECTION_VERTICAL;
            *new_value = left_column_count;
            
            return true;
        }
        
        if ((old_reflection_type == REFLECTION_VERTICAL) && 
            (old_value != left_column_count)) {
            *new_reflection_type = REFLECTION_VERTICAL;
            *new_value = left_column_count;
            
            return true;
        }
    }
    
    // Horizontal
    for (int i = 0; i < horizontal_reflection_count; i++) {
        int above_column_count = above_column_counts[i];
        
        if (old_reflection_type == REFLECTION_VERTICAL) {
            *new_reflection_type = REFLECTION_HORIZONTAL;
            *new_value = above_column_count;
            
            return true;
        }
        
        if ((old_reflection_type == REFLECTION_HORIZONTAL) && 
            (old_value != above_column_count)) {
            *new_reflection_type = REFLECTION_HORIZONTAL;
            *new_value = above_column_count;
            
            return true;
        }
    }
    
    return false;
}

static int summarize_after_fixing_smudge(Pattern *pattern) {
    for (int y = 0; y < pattern->height; y++) {
        for (int x = 0; x < pattern->width; x++) {
            int index = get_tile_index(x, y);
            
            Reflection_Type old_reflection_type;
            int old_value;
            summarize_and_accept_first(pattern, &old_reflection_type, &old_value);
            
            pattern->memory[index] = opposite(pattern->memory[index]);
            {
                Reflection_Type new_reflection_type;
                int new_value;
                if (summarize_and_reject(pattern,
                                         old_reflection_type, old_value,
                                         &new_reflection_type, &new_value)) {
                    if (new_reflection_type == REFLECTION_HORIZONTAL) {
                        new_value *= 100;
                    }
                    
                    return new_value;
                }
            }
            pattern->memory[index] = opposite(pattern->memory[index]);
        }
    }
    
    return 0;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);
    
    preprocess_input(input);
    
    //
    int total = 0;
    
    for (int i = 0; i < pattern_count; i++) {
        total += summarize_after_fixing_smudge(&patterns[i]);
    }
    
    printf("The total is %d\n", total);
    
    return 0;
}
