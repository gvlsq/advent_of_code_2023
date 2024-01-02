#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct Step {
    char data[10];
};

static int step_count;
static Step steps[1 << 12];

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

static void preprocess_input(char *input) {
    char *at = input;
    while (*at) {
        assert(step_count < array_count(steps));
        Step *step = &steps[step_count++];
        
        int cursor = 0;
        while (*at && (*at != ',')) {
            if (is_eol(*at)) at++;
            
            assert(cursor < array_count(step->data));
            step->data[cursor++] = *at++;
        }
        
        assert(cursor < array_count(step->data));
        step->data[cursor++] = '\0';
        
        if (*at == ',') at++;
    }
}

static int string_length(char *s) {
    int result = strlen(s);
    
    return result;
}

static int run_hash_algorithm(Step *step) {
    int result = 0;
    
    for (int i = 0; i < string_length(step->data); i++) {
        result += (int)step->data[i];
        result *= 17;
        result %= 256;
    }
    
    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);
    
    preprocess_input(input);
    
    //
    int sum = 0;
    
    for (int i = 0; i < step_count; i++) {
        sum += run_hash_algorithm(&steps[i]);
    }
    
    printf("The sum is %d\n", sum);
    
    return 0;
}
