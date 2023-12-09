#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

#define maximum(a, b) ((a) > (b) ? (a) : (b))
#define minimum(a, b) ((a) < (b) ? (a) : (b))

struct Node {
    char *name;

    char *left_name;
    char *right_name;

    int steps_to_z;
};

static char left_right_instruction[1 << 9];

static int node_count;
static Node nodes[1 << 10];

static int starting_node_count;
static Node *starting_nodes[1 << 3];

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

static char *duplicate_string(char *s) {
    char *result = _strdup(s);

    return result;
}

static void preprocess_input(char *input) {
    char *at = input;

    int matched;
    int chars_read;

    // Left-right instructions
    matched = sscanf(at, "%s\n\n%n", &left_right_instruction, &chars_read);

    at += chars_read;

    // Network
    char tmp0[3 + 1];
    char tmp1[3 + 1];
    char tmp2[3 + 1];
    while (sscanf(at, "%s = (%[^,], %[^)])\n%n", &tmp0, &tmp1, &tmp2, &chars_read) == 3) {
        assert(node_count < array_count(nodes));

        Node *node = &nodes[node_count++];
        node->name = duplicate_string(tmp0);
        node->left_name = duplicate_string(tmp1);
        node->right_name = duplicate_string(tmp2);

        at += chars_read;
    }
}

static inline bool strings_match(char *a, const char *b) {
    bool result = strcmp(a, b) == 0;

    return result;
}

static Node *find_node(const char *name) {
    Node *result = 0;

    for (int i = 0; i < node_count; i++) {
        if (strings_match(nodes[i].name, name)) {
            result = &nodes[i];
            break;
        }
    }

    assert(result);

    return result;
}

static u64 gcd(u64 a, u64 b) {
    u64 result = minimum(a, b);

    while (result) {
        if ((a % result == 0) && (b % result == 0)) {
            break;
        }

        result--;
    }

    return result;
}

static u64 lcm(u64 a, u64 b) {
    u64 result = (a*b) / gcd(a, b);

    return result;
}

static int count_steps_to_z(Node *node) {
    int result = 0;

    Node *at_node = node;

    char *instruction_ptr = left_right_instruction;
    for (;;) {
        if (!(*instruction_ptr)) instruction_ptr = left_right_instruction; // Re-spool instructions if we wrap

        char instruction = *instruction_ptr++;
        if (instruction == 'L') {
            at_node = find_node(at_node->left_name);
        } else if (instruction == 'R') {
            at_node = find_node(at_node->right_name);
        }

        result++;

        if (at_node->name[2] == 'Z') break;
    }

    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    // Initialize starting nodes
    for (int i = 0; i < node_count; i++) {
        Node *node = &nodes[i];
        if (node->name[2] == 'A') {
            node->steps_to_z = count_steps_to_z(node);
            
            assert(starting_node_count < array_count(starting_nodes));
            starting_nodes[starting_node_count++] = node;
        }
    }

    // Find LCM of steps-to-**Z for all starting nodes
    u64 total = starting_nodes[0]->steps_to_z;
    for (int i = 1; i < starting_node_count; i++) {
        total = lcm(total, (u64)starting_nodes[i]->steps_to_z);
    }

    printf("The total is %I64u\n", total);

    return 0;
}
