#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "graph.h"
#include "layout_tutte.h"
#include "fruchterman.h"
#include "io.h"                 

void print_usage(char *prog_name) {
    printf("Użycie: %s -i <wejście.txt> -o <wyjście> -a <tutte|fruchterman> [-f <txt|bin>]\n", prog_name);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int opt;
    int success = 0;
    char *input_path = NULL;
    char *output_path = NULL;
    char *algorithm = "tutte";
    char *format = "txt";

    while ((opt = getopt(argc, argv, "i:o:a:f:h")) != -1) {
        switch (opt) {
            case 'i': input_path = optarg; break;
            case 'o': output_path = optarg; break;
            case 'a': algorithm = optarg; break;
            case 'f': format = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default: return 1;
        }
    }

    if (input_path == NULL || output_path == NULL) {
        fprintf(stderr, "Błąd: Brak podanego pliku wejściowego lub wyjściowego!\n");
        return 1;
    }

    Graph g = { .nodes = NULL, .edges = NULL, .node_count = 0, .edge_count = 0, .width = 1000.0, .height = 1000.0 };

    if (validate_and_process(input_path) == -1) return 10;

    if (read_graph(&g, input_path) != 0) return 2;

    if (g.edge_count == 0) {
        fprintf(stderr, "Błąd: Plik pusty.\n");
        free_graph(&g);
        return 5;
    }

    // Sprawdzenie spójności (Kluczowy moment)
    if (!is_graph_connected(&g)) {
        printf("\n============================================================\n");
        printf("   BŁĄD KRYTYCZNY: GRAF NIESPÓJNY    \n");
        printf("============================================================\n");
        printf("Dane z pliku wejściowego [%s] są błędne:\n", input_path);
        printf("-> Graf składa się z kilku niepołączonych ze sobą części.\n");
        printf("-> Algorytm wymaga grafu spójnego, aby poprawnie wyznaczyć pozycje.\n");
        printf("============================================================\n");
        
        free_graph(&g);
        return 9; // Zwracamy kod błędu 9 dla braku spójności
    }

    success = 0; // Flaga sukcesu dla całego procesu

    // --- LOGIKA WYBORU ALGORYTMU ---
    if (strcmp(algorithm, "tutte") == 0) {
        if (g.node_count < 3) {
            fprintf(stderr, "Błąd: Tutte wymaga >= 3 wierzchołków!\n");
            free_graph(&g);
            return 6;
        }
        compute_tutte_layout(&g); 
        if (is_tutte_layout_planar(&g)) {
            printf("Sukces! Graf planarny (Tutte).\n");
            success = 1;
        } else {
            printf("[Ostrzeżenie] Graf Tutte'a ma przecięcia.\n");
            success = 1; 
        }
    } 
    else if (strcmp(algorithm, "fruchterman") == 0) {
        if (!is_potentially_planar(&g)) {
            free_graph(&g);
            return 7;
        }
        if (!is_graph_connected(&g)) {
            free_graph(&g);
            return 9;
        }

        int attempts = 0;
        while (!success && attempts < 3000) {
            attempts++;
            init_random_positions(&g);
            run_fruchterman(&g, 1000, 30.0);
            if (is_layout_planar(&g)) success = 1;
        }
    }

    // --- ZAPIS ---
    if (success) {
        save_graph(&g, output_path, format);
    }

    free_graph(&g);
    return success ? 0 : 8;
}