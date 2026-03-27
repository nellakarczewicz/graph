#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include <ctype.h>



// Funkcja pomocnicza: zamienia przecinki na kropki i usuwa spacje 
void sanitize_line(char *s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] == ',') s[i] = '.'; // Zamiana separatora dziesiętnego
    }
}

int read_graph(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    g->edges = malloc(1000 * sizeof(Edge));
    g->nodes = malloc(1000 * sizeof(Node));
    g->edge_count = 0;
    g->node_count = 0;

    char line[256];
    int line_num = 0;
    int auto_fixed = 0;

    printf("\n--- STATUS WCZYTYWANIA ---\n");

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        
        // 1. Ignorowanie pustych linii
        if (strlen(line) <= 1 || line[0] == '\n') continue;

        // 2. Wykrywanie separatora (średnik vs przecinek)
        char separator = ';';
        if (strchr(line, ';') == NULL && strchr(line, ',') != NULL) {
            separator = ',';
            printf("  [Wskazówka] Linia %d: Wykryto przecinki zamiast średników. Próbuję przetworzyć...\n", line_num);
        }

        // 3. Czyszczenie (Sanitizing)
        sanitize_line(line);

        // 4. Parsowanie
        char name[50];
        int u, v;
        double w;
        
        // Budujemy format parsowania w zależności od wykrytego separatora
        char format_str[50];
        sprintf(format_str, " %%[^%c]%c%%d%c%%d%c%%lf", separator, separator, separator, separator);

        if (sscanf(line, format_str, name, &u, &v, &w) == 4) {
            g->edges[g->edge_count].u_idx = u - 1;
            g->edges[g->edge_count].v_idx = v - 1;
            g->edges[g->edge_count].weight = w;
            g->edge_count++;
            
            if (u > g->node_count) g->node_count = u;
            if (v > g->node_count) g->node_count = v;
        } else {
            printf("  [BŁĄD] Linia %d: Niepoprawny format danych. Pominięto.\n", line_num);
        }
    }

    printf("\nPodsumowanie:\n - Wczytano poprawnie: %d krawędzi.\n", g->edge_count);
    printf(" - Przeanalizowano linii: %d.\n", line_num);
    printf("---------------------------\n");

    fclose(f);
    return 0;
}


int save_graph(Graph *g, const char *filename, const char *format) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    // Zapisujemy zgodnie ze specyfikacją projektu: <wierzchołek> <x> <y>
    for (int i = 0; i < g->node_count; i++) {
        fprintf(f, "%d %.2f %.2f\n", g->nodes[i].id, g->nodes[i].x, g->nodes[i].y);
    }

    fclose(f);
    return 0;
}

void free_graph(Graph *g) {
    if(g->nodes) free(g->nodes);
    if(g->edges) free(g->edges);
}