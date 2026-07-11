#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <v220_decoder.h>
#include <lun_file.h>

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
    printf("  " COLOR_YELLOW "-o <output_file>\t" COLOR_RESET "Specify the output .lun file path.\n");
    printf("  " COLOR_YELLOW "--help, -h\t\t" COLOR_RESET "Show this help message and exit.\n\n");

    printf(COLOR_BOLD "Example:\n" COLOR_RESET);
    printf("  lv220c " COLOR_GREEN "maps/level_01.map\n" COLOR_RESET);
    printf("  lv220c " COLOR_YELLOW "-o " COLOR_GREEN "maps/level_01.lun maps/level_01.map\n\n" COLOR_RESET);
}

char* get_default_output_filename(const char* input_path) {
    size_t len = strlen(input_path);
    char *out_path = malloc(len + 5); // Room for ".lun\0"
    if (!out_path) return NULL;

    strcpy(out_path, input_path);
    char *dot = strrchr(out_path, '.');
    char *slash = strrchr(out_path, '/');
    if (dot && (!slash || dot > slash)) {
        strcpy(dot, ".lun");
    } else {
        strcat(out_path, ".lun");
    }
    return out_path;
}

int main(int argc, char **argv) {
    const char *input_file = NULL;
    const char *output_file = NULL;
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER start, finish;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
#else
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            show_help();
            return 0;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, COLOR_RED "Error: Missing argument for option -o\n" COLOR_RESET);
                return 1;
            }
        } else {
            if (input_file == NULL) {
                input_file = argv[i];
            } else {
                fprintf(stderr, COLOR_RED "Error: Multiple input files specified: '%s' and '%s'\n" COLOR_RESET, input_file, argv[i]);
                return 1;
            }
        }
    }

    if (input_file == NULL) {
        fprintf(stderr, COLOR_RED "Error: No input map file specified\n" COLOR_RESET);
        show_help();
        return 1;
    }

    char *allocated_output = NULL;
    if (output_file == NULL) {
        allocated_output = get_default_output_filename(input_file);
        if (allocated_output == NULL) {
            fprintf(stderr, COLOR_RED "Error: Memory allocation failed\n" COLOR_RESET);
            return 1;
        }
        output_file = allocated_output;
    }

    printf(COLOR_CYAN "Opening map file: %s...\n" COLOR_RESET, input_file);
    FILE *map_file = fopen(input_file, "r");
    if (!map_file) {
        fprintf(stderr, COLOR_RED "Error: Could not open file %s\n" COLOR_RESET, input_file);
        free(allocated_output);
        return 1;
    }

    Map map;
    printf(COLOR_CYAN "Decoding map file...\n" COLOR_RESET);
    decode_map_file(map_file, &map);
    fclose(map_file);

    printf(COLOR_CYAN "Compiling map and exporting to %s...\n" COLOR_RESET, output_file);
    if (!export_map_to_lun(output_file, &map)) {
        fprintf(stderr, COLOR_RED "Error: Failed to export map to %s\n" COLOR_RESET, output_file);
        free_map(&map);
        free(allocated_output);
        return 1;
    }

#ifdef _WIN32
    QueryPerformanceCounter(&finish);
    double time_ms = (double)(finish.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
#else
    clock_gettime(CLOCK_MONOTONIC, &finish);
    double time_ms = (finish.tv_sec - start.tv_sec) * 1000.0 + (finish.tv_nsec - start.tv_nsec) / 1000000.0;
#endif

    printf(COLOR_GREEN COLOR_BOLD "Successfully compiled %s to %s in %.2f ms!\n" COLOR_RESET, input_file, output_file, time_ms);

    free_map(&map);
    free(allocated_output);

    return 0;
}
