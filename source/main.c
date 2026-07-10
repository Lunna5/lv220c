#include <stdio.h>
#include <v220_decoder.h>

#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RED     "\x1b[31m"

void show_help() {
    printf(COLOR_BOLD COLOR_CYAN "Lunna Valve220 Map Compiler " COLOR_RESET);
    printf("for " COLOR_BOLD "LunnaEngine\n\n" COLOR_RESET);

    printf(COLOR_BOLD "Usage:\n" COLOR_RESET);
    printf("  lv220c [" COLOR_YELLOW "options" COLOR_RESET "] " COLOR_GREEN "<map_file>\n\n" COLOR_RESET);

    printf(COLOR_BOLD "Options:\n" COLOR_RESET);

    printf("  " COLOR_YELLOW "--help\t\t" COLOR_RESET "Show this help message and exit.\n");

    printf(COLOR_BOLD "Example:\n" COLOR_RESET);
    printf("  lv220c " COLOR_GREEN "maps/level_01.map\n\n" COLOR_RESET);
}
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, COLOR_RED "Bad number of arguments\n" COLOR_RESET);
        show_help();
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        show_help();
        return 0;
    }

    FILE *map_file = fopen(argv[1], "r");
    if (!map_file) {
        fprintf(stderr, COLOR_RED "Error: Could not open file %s\n" COLOR_RESET, argv[1]);
        return 1;
    }

    Map map;
    decode_map_file(map_file, &map);

    // Print map

    for (int i = 0; i < map.entity_count; i++) {
        Entity *entity = &map.entities[i];
        printf("Entity %d: %d properties %d, brushes\n", i, entity->property_count, entity->brush_count);
    }

    free_map(&map);
    fclose(map_file);

    return 0;
}
