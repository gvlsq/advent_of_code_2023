#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define Invalid_Code_Path assert(!"Invalid code path")
#define Invalid_Default_Case default: Invalid_Code_Path; break;

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

#define get_tile_index(x, y) ((x) + grid_width*(y))

struct Vector2i {
	int x;
	int y;
};

struct Tile {
	char c;
	Vector2i position;
	int dist_to_S;
};

struct Tile_Queue {
	int read_cursor;
	int write_cursor;
	Tile *entries[1 << 10];
};

static int grid_width;
static int grid_height;
static Tile *grid;

static Tile_Queue tile_queue;

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

static inline Vector2i vector2i(int x, int y) {
	Vector2i result;
	result.x = x;
	result.y = y;

	return result;
}

static void preprocess_input(char *input) {
	int estimated_width = 256;
	int estimated_height = 256;

	grid = (Tile *)malloc(sizeof(Tile)*estimated_width*estimated_height);

	// Grid width
	char *at = input;
	while (!is_eol(*at)) {
		at++;
	}

	grid_width = at - input;

	// Grid tiles & height
	int index = 0;
	int lines_read = 0;

	at = input;
	while (*at) {
		assert(index < (estimated_width*estimated_height));

		Tile *tile = &grid[index];
		tile->c = *at;
		tile->position = vector2i(index % grid_width, index / grid_width);
		tile->dist_to_S = -1;

		index++;
		at++;

		if (is_eol(*at)) {
			lines_read++;
			at++;
		}
	}

	grid_height = lines_read;
}

static Tile *find_first_tile_with_char(char c) {
	Tile *result = 0;

	for (int y = 0; y < grid_height; y++) {
		for (int x = 0; x < grid_width; x++) {
			int index = get_tile_index(x, y);
			if (grid[index].c == c) {
				result = &grid[index];
				break;
			}
		}
	}

	return result;
}

static int advance_queue_cursor(int cursor) {
	int result = (cursor + 1) & (array_count(tile_queue.entries) - 1);

	return result;
}

static void enqueue_tile(Tile *tile) {
	assert(advance_queue_cursor(tile_queue.write_cursor) != tile_queue.read_cursor);

	tile_queue.entries[tile_queue.write_cursor] = tile;
	tile_queue.write_cursor = advance_queue_cursor(tile_queue.write_cursor);
}

static Tile *dequeue_tile(void) {
	assert(tile_queue.read_cursor != tile_queue.write_cursor);

	Tile *result = tile_queue.entries[tile_queue.read_cursor];
	tile_queue.read_cursor = advance_queue_cursor(tile_queue.read_cursor);

	return result;
}

static int count_queued_entries(void) {
	int result = 0;

	int read_cursor = tile_queue.read_cursor;
	int write_cursor = tile_queue.write_cursor;

	while (read_cursor != write_cursor) {
		read_cursor = advance_queue_cursor(read_cursor);
		result++;
	}

	return result;
}

static inline Tile *get_tile_at_position(int x, int y) {
	Tile *result = 0;

	if ((0 <= x && x < grid_width) && (0 <= y && y < grid_height)) {
		result = &grid[get_tile_index(x, y)];
	}

	return result;
}

static bool is_pipe_move_valid(Tile *a, Tile *b) {
	assert(a->c != '.');

	if (b->c == '.') return false;
	if (b->c == 'S') return false;

	bool b_is_above = b->position.y < a->position.y;
	bool b_is_below = b->position.y > a->position.y;
	bool b_is_to_the_left = b->position.x < a->position.x;
	bool b_is_to_the_right = b->position.x > a->position.x;

	switch (a->c) {
		case '|':
			if (b->c == '|' && (b_is_above || b_is_below)) return true;
			if ((b->c == 'L' || b->c == 'J') && b_is_below) return true;
			if ((b->c == '7' || b->c == 'F') && b_is_above) return true;
			break;

		case '-':
			if (b->c == '-' && (b_is_to_the_left || b_is_to_the_right)) return true;
			if ((b->c == 'L' || b->c == 'F') && b_is_to_the_left) return true;
			if ((b->c == 'J' || b->c == '7') && b_is_to_the_right) return true;
			break;

		case 'L':
			if ((b->c == '|' || b->c == 'F' || b->c == '7') && b_is_above) return true;
			if ((b->c == '-' || b->c == 'J' || b->c == '7') && b_is_to_the_right) return true;
			break;

		case 'J':
			if ((b->c == '|' || b->c == '7' || b->c == 'F') && b_is_above) return true;
			if ((b->c == '-' || b->c == 'L' || b->c == 'F') && b_is_to_the_left) return true;
			break;

		case '7':
			if ((b->c == '|' || b->c == 'L' || b->c == 'J') && b_is_below) return true;
			if ((b->c == '-' || b->c == 'L' || b->c == 'F') && b_is_to_the_left) return true;
			break;

		case 'F':
			if ((b->c == '|' || b->c == 'L' || b->c == 'J') && b_is_below) return true;
			if ((b->c == '-' || b->c == 'J' || b->c == '7') && b_is_to_the_right) return true;
			break;

			Invalid_Default_Case;
	}

	return false;
}

static char pipe_types[] = {
	'|',
	'-',
	'L',
	'J',
	'7',
	'F',
};
static char infer_pipe_at_tile(Tile *S) {
	char result = S->c;

	bool found = false;

	Vector2i search_offsets[4] = {
		vector2i(0, -1),
		vector2i(0, +1),
		vector2i(-1, 0),
		vector2i(+1, 0),
	};
	for (int i = 0; (i < array_count(search_offsets) - 1) && !found; i++) {
		Vector2i oa = search_offsets[i];

		for (int j = i + 1; (j < array_count(search_offsets) && !found); j++) {
			Vector2i ob = search_offsets[j];

			int x = S->position.x;
			int y = S->position.y;

			Tile *a = get_tile_at_position(x + oa.x, y + oa.y);
			Tile *b = get_tile_at_position(x + ob.x, y + ob.y);

			for (int k = 0; k < array_count(pipe_types); k++) {
				char pipe = pipe_types[k];

				Tile tmp = *S;
				tmp.c = pipe;

				if (is_pipe_move_valid(&tmp, a) && is_pipe_move_valid(&tmp, b)) {
					result = pipe;

					found = true;
					break;
				}
			}
		}
	}

	assert(result != '.' && result != 'S');

	return result;
}

int main(int argc, char **argv) {
	char *input = read_text_file("input.txt");
	if (!input) exit(EXIT_FAILURE);

	preprocess_input(input);

	Tile *S = find_first_tile_with_char('S');
	S->c = infer_pipe_at_tile(S);
	S->dist_to_S = 0;

	enqueue_tile(S);

	Vector2i search_offsets[4] = {
		vector2i(0, -1),
		vector2i(0, +1),
		vector2i(-1, 0),
		vector2i(+1, 0),
	};
	while (count_queued_entries() > 0) {
		Tile *dequeued = dequeue_tile();

		for (int i = 0; i < array_count(search_offsets); i++) {
			Vector2i offset = search_offsets[i];

			int search_x = dequeued->position.x + offset.x;
			int search_y = dequeued->position.y + offset.y;

			Tile *search = get_tile_at_position(search_x, search_y);
			if (!search) continue;

			if ((search->dist_to_S == -1) && is_pipe_move_valid(dequeued, search)) {
				assert(dequeued->dist_to_S != -1);

				search->dist_to_S = dequeued->dist_to_S + 1;
				enqueue_tile(search);
			}
		}
	}

	//
	int max_dist = INT32_MIN;

	for (int y = 0; y < grid_height; y++) {
		for (int x = 0; x < grid_width; x++) {
			Tile *tile = &grid[get_tile_index(x, y)];
			if (tile->dist_to_S > max_dist) {
				max_dist = tile->dist_to_S;
			}
		}
	}

	printf("The max distance is %d\n", max_dist);

	return 0;
}
