#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_count(a) (sizeof((a)) / sizeof((a)[0]))

static char card_labels[] = {
    'A',
    'K',
    'Q',
    'T',
    '9',
    '8',
    '7',
    '6',
    '5',
    '4',
    '3',
    '2',
};

enum Hand_Type {
    HAND_TYPE_HIGH_CARD,
    HAND_TYPE_ONE_PAIR,
    HAND_TYPE_TWO_PAIR,
    HAND_TYPE_THREE_OF_A_KIND,
    HAND_TYPE_FULL_HOUSE,
    HAND_TYPE_FOUR_OF_A_KIND,
    HAND_TYPE_FIVE_OF_A_KIND,
    HAND_TYPE_COUNT,
};
struct Hand {
    char cards[5 + 1];
    int bid;
};

struct Card_Group {
    char label;
    int count;
};

struct Hand_Stack {
    int depth;
    Hand hands[1 << 8];
};

static int hand_count;
static Hand hands[1 << 10];

static Hand_Stack hand_stack;

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

static inline char *duplicate_string(char *s) {
    char *result = _strdup(s);

    return result;
}

static void preprocess_input(char *input) {
    char *at = input;

    int chars_read;

    char tmp0[5 + 1];
    int tmp1;
    while (sscanf(at, "%s %d\n%n", &tmp0, &tmp1, &chars_read) == 2) {
        assert(hand_count < array_count(hands));

        Hand *hand = &hands[hand_count++];
        memcpy(hand->cards, tmp0, 5);
        hand->bid = tmp1;

        at += chars_read;
    }
}

static inline int string_length(char *s) {
    int result = strlen(s);

    return result;
}

static inline int index_of(char *s, char c) {
    int result = -1;

    for (int i = 0; i < string_length(s); i++) {
        if (s[i] == c) {
            result = i;
            break;
        }
    }

    return result;
}

static void push_hand(Hand_Stack *stack, Hand hand) {
    assert(stack->depth < array_count(stack->hands));
    stack->hands[stack->depth++] = hand;
}

static Hand pop_hand(Hand_Stack *stack) {
    assert(stack->depth > 0);
    return stack->hands[--stack->depth];
}

static inline Card_Group *find_group(Card_Group *groups, int group_count, char label) {
    Card_Group *result = 0;

    for (int i = 0; i < group_count; i++) {
        if (groups[i].label == label) {
            result = &groups[i];
            break;
        }
    }

    return result;
}

static void group_by_card_label(Hand *hand, Card_Group *groups, int *group_count) {
    // Raw group by
    for (int i = 0; i < string_length(hand->cards); i++) {
        char label = hand->cards[i];

        Card_Group *group = find_group(groups, *group_count, label);
        if (!group) {
            assert(*group_count < 5);

            group = &groups[(*group_count)++];
            group->label = label;
            group->count = 0;
        }

        group->count++;
    }

    // Sort by count
    for (int i = 0; i < *group_count; i++) {
        bool swapped = false;

        for (int j = 0; j < (*group_count - 1); j++) {
            Card_Group *a = &groups[j];
            Card_Group *b = &groups[j + 1];

            if (b->count > a->count) {
                Card_Group swap = *a;
                *a = *b;
                *b = swap;

                swapped = true;
            }
        }

        if (!swapped) break;
    }
}

static inline bool is_five_of_a_kind(Card_Group *groups, int group_count) {
    bool result = group_count == 1 && groups[0].count == 5;

    return result;
}

static inline bool is_four_of_a_kind(Card_Group *groups, int group_count) {
    bool result = group_count == 2 && (groups[0].count == 4 && groups[1].count == 1);

    return result;
}

static inline bool is_full_house(Card_Group *groups, int group_count) {
    bool result = group_count == 2 && (groups[0].count == 3 && groups[1].count == 2);

    return result;
}

static inline bool is_three_of_a_kind(Card_Group *groups, int group_count) {
    bool result = group_count == 3 && (groups[0].count == 3 && groups[1].count == 1 && groups[2].count == 1);

    return result;
}

static inline bool is_two_pair(Card_Group *groups, int group_count) {
    bool result = group_count == 3 && (groups[0].count == 2 && groups[1].count == 2 && groups[2].count == 1);

    return result;
}

static inline bool is_one_pair(Card_Group *groups, int group_count) {
    bool result = group_count == 4 && (groups[0].count == 2 && groups[1].count == 1 && groups[2].count == 1 && groups[3].count == 1);

    return result;
}

static Hand_Type compute_hand_type(Hand *hand) {
    Hand_Type best_type = {};

    push_hand(&hand_stack, *hand);

    while (hand_stack.depth) {
        Hand popped = pop_hand(&hand_stack);

        int joker_index = index_of(popped.cards, 'J');
        if (joker_index != -1) {
            for (int i = 0; i < array_count(card_labels); i++) {
                popped.cards[joker_index] = card_labels[i];
                push_hand(&hand_stack, popped);
            }
        } else {
            int card_group_count = 0;
            Card_Group card_groups[5];
            group_by_card_label(&popped, card_groups, &card_group_count);

            Hand_Type hand_type;
            if (is_five_of_a_kind(card_groups, card_group_count)) {
                hand_type = HAND_TYPE_FIVE_OF_A_KIND;
            } else if (is_four_of_a_kind(card_groups, card_group_count)) {
                hand_type = HAND_TYPE_FOUR_OF_A_KIND;
            } else if (is_full_house(card_groups, card_group_count)) {
                hand_type = HAND_TYPE_FULL_HOUSE;
            } else if (is_three_of_a_kind(card_groups, card_group_count)) {
                hand_type = HAND_TYPE_THREE_OF_A_KIND;
            } else if (is_two_pair(card_groups, card_group_count)) {
                hand_type = HAND_TYPE_TWO_PAIR;
            } else if (is_one_pair(card_groups, card_group_count)) {
                hand_type = HAND_TYPE_ONE_PAIR;
            } else {
                hand_type = HAND_TYPE_HIGH_CARD;
            }

            if (hand_type > best_type) {
                best_type = hand_type;
            }
        }
    }

    return best_type;
}

static int get_card_strength(char c) {
    int result = 0;

    switch (c) {
        case 'A':
            result = 14;
            break;

        case 'K':
            result = 13;
            break;

        case 'Q':
            result = 12;
            break;

        case 'T':
            result = 10;
            break;

        case 'J':
            result = 1;
            break;

        default:
            result = c - '0';
            break;
    }

    assert(1 <= result && result <= 14);
    assert(result != 11);

    return result;
}

int main(int argc, char **argv) {
    char *input = read_text_file("input.txt");
    if (!input) exit(EXIT_FAILURE);

    preprocess_input(input);

    // Sort by hand type
    for (int i = 0; i < hand_count; i++) {
        bool swapped = false;

        for (int j = 0; j < hand_count - 1; j++) {
            Hand *a = &hands[j];
            Hand *b = &hands[j + 1];

            Hand_Type ta = compute_hand_type(a);
            Hand_Type tb = compute_hand_type(b);

            if (tb > ta) {
                Hand swap = *a;
                *a = *b;
                *b = swap;

                swapped = true;
            }
        }

        if (!swapped) break;
    }

    // Sort by first difference
    for (int i = 0; i < hand_count; i++) {
        bool swapped = false;

        for (int j = 0; j < hand_count - 1; j++) {
            Hand *a = &hands[j];
            Hand *b = &hands[j + 1];

            Hand_Type ta = compute_hand_type(a);
            Hand_Type tb = compute_hand_type(b);

            if (ta == tb) {
                assert(string_length(a->cards) == string_length(b->cards));

                for (int k = 0; k < string_length(a->cards); k++) {
                    int sa = get_card_strength(a->cards[k]);
                    int sb = get_card_strength(b->cards[k]);

                    if (sb > sa) {
                        Hand swap = *a;
                        *a = *b;
                        *b = swap;

                        swapped = true;

                        break;
                    } else if (sa > sb) { // A is already stronger than B, so we bail
                        break;
                    }
                }
            }
        }

        if (!swapped) break;
    }

    // 
    int total_winnings = 0;

    for (int i = 0; i < hand_count; i++) {
        int rank = hand_count - i;

        total_winnings += hands[i].bid*rank;
    }

    printf("The total winnings are %d\n", total_winnings);

    return 0;
}
