#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

struct History {
	int count;
	int values[1 << 10];
};

struct History_Stack {
	int depth;
	History histories[1 << 8];
};

static int initial_history_count;
static History initial_histories[1 << 9];

static History_Stack history_stack;

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
		assert(initial_history_count < array_count(initial_histories));

		History *initial_history = &initial_histories[initial_history_count++];

		int chars_read;
		int tmp;
		while (sscanf(at, "%d%n", &tmp, &chars_read) == 1) {
			assert(initial_history->count < array_count(initial_history->values));
			initial_history->values[initial_history->count++] = tmp;

			at += chars_read;

			if (*at == '\n') {
				at++;
				break;
			}
		}
	}
}

static void clear_histories(void) {
	history_stack.depth = 0;
}

static void push_history(History history) {
	assert(history_stack.depth < array_count(history_stack.histories));
	history_stack.histories[history_stack.depth++] = history;
}

static History pop_history(void) {
	assert(history_stack.depth);
	return history_stack.histories[--history_stack.depth];
}

static History peek_history(void) {
	assert(history_stack.depth);
	return history_stack.histories[history_stack.depth - 1];
}

static inline bool is_all_zeros(History history) {
	for (int i = 0; i < history.count; i++) {
		if (history.values[i] != 0) return false;
	}

	return true;
}

static int get_next_value_for_history(History *history) {
	clear_histories();

	push_history(*history);

	// Buffering up new histories based on old ones
	while (history_stack.depth) {
		History peek = peek_history();

		History new_history = {};
		{
			for (int i = 0; i < peek.count - 1; i++) {
				assert(new_history.count < array_count(new_history.values));
				new_history.values[new_history.count++] = peek.values[i + 1] - peek.values[i];
			}
		}
		push_history(new_history);

		if (is_all_zeros(new_history)) break;
	}

	// Filling out the "zero" history at the top of the stack
	History *zero_history = &history_stack.histories[history_stack.depth - 1];
	assert(zero_history->count < array_count(zero_history->values));
	zero_history->values[zero_history->count++] = 0;

	// Computing new ending values for other histories
	for (int i = history_stack.depth - 2; i >= 0; i--) {
		History *current_history = &history_stack.histories[i];
		History *prior_history = &history_stack.histories[i + 1];

		int a = current_history->values[current_history->count - 1];
		int b = prior_history->values[prior_history->count - 1];

		assert(current_history->count < array_count(current_history->values));
		current_history->values[current_history->count++] = a + b;
	}

	//
	History *first_history = &history_stack.histories[0];

	return first_history->values[first_history->count - 1];
}

int main(int argc, char **argv) {
	char *input = read_text_file("input.txt");
	if (!input) exit(EXIT_FAILURE);

	preprocess_input(input);

	//
	int total = 0;

	for (int i = 0; i < initial_history_count; i++) {
		History *initial_history = &initial_histories[i];

		total += get_next_value_for_history(initial_history);
	}

	printf("The total is %d\n", total);

	return 0;
}
