#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include <ctype.h>
#include "io.h"



// Funkcja pomocnicza: zamienia przecinki na kropki i usuwa spacje 
int sanitize(char *s) {
    int changed = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == ',') {
            s[i] = '.'; // Zamiana separatora dziesiętnego
            changed = 1;
        }
    }
    return changed;
}

// Pomocnicza funkcja do wykrywania separatora (średnik lub przecinek)
static char detect_separator(const char *line) {
    return (strchr(line, ';') == NULL && strchr(line, ',') != NULL) ? ',' : ';';
}

int validate_and_process(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Błąd: Nie można otworzyć pliku %s\n", filename);
        return -1;
    }

    char line[256], name[50];
    int line_num = 0, critical_errors = 0, total_fixes = 0;
    int u, v;
    double w;

    printf("\n============================================================\n");
    printf("   RAPORT ANALIZY DANYCH WEJSCIOWYCH    \n");
    printf("============================================================\n");
    printf("%-7s | %-30s | %-15s\n", "Linia", "Zinterpretowane dane", "Status");
    printf("------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        if (strlen(line) <= 1 || line[0] == '\n') continue;

        char sep = detect_separator(line);
        int fixed = sanitize(line);
        if (fixed) total_fixes++;

        char format[32];
        sprintf(format, " %%49[^%c]%c%%d%c%%d%c%%lf", sep, sep, sep, sep);

        if (sscanf(line, format, name, &u, &v, &w) == 4) {
            printf("%-7d | Krawedz %s: %d -> %d (w: %.2f) ", line_num, name, u, v, w);
            printf("| [%s]\n", fixed ? "Poprawiono ," : (sep == ',' ? "Separator ," : "OK"));
        } else {
            printf("%-7d | [!!! BŁĄD KRYTYCZNY - ZŁY FORMAT !!!]\n", line_num);
            critical_errors++;
        }
    }
    fclose(f);

    printf("============================================================\n");

    // SCENARIUSZ 1: Błędy uniemożliwiające start
    if (critical_errors > 0) {
        printf("\nSTOP! Znaleziono %d bledow krytycznych w strukturze pliku.\n", critical_errors);
        return -1;
    }

    // SCENARIUSZ 2: Dane są poprawne technicznie - czekamy na decyzję użytkownika
    if (total_fixes > 0) {
        printf("INFO: Dane poprawne technicznie (dokonano %d autokorekt).\n", total_fixes);
    } else {
        printf("INFO: Dane poprawne technicznie.\n");
    }

    printf("Uruchamiam przetwarzanie...\n");
    return 0; // Można kontynuować
}

int read_graph(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    // Używamy calloc, aby wyzerować dx, dy i inne pola
    g->edges = malloc(1000 * sizeof(Edge));
    g->nodes = calloc(1000, sizeof(Node));
    
    if (!g->edges || !g->nodes) return -2;

    g->edge_count = 0;
    g->node_count = 0;

    char line[256], name[50];
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
            printf("  [Wskazówka] Linia %d: Wykryto przecinki zamiast średników. Próbuję przetworzyć...\n", line_num);
        }

        // 3. Czyszczenie (Sanitizing)
        sanitize(line);

        // 4. Parsowanie
        int u, v;
        double w;
        // Budujemy format parsowania w zależności od wykrytego separatora
        char format_str[50];
        sprintf(format_str, " %%[^%c]%c%%d%c%%d%c%%lf", separator, separator, separator, separator);

        if (sscanf(line, format_str, name, &u, &v, &w) == 4) {
            // Rezerwowe zabezpieczenie przed wyjściem poza 1000
            if (g->edge_count < 1000) {
                g->edges[g->edge_count].u_idx = u - 1;
                g->edges[g->edge_count].v_idx = v - 1;
                g->edges[g->edge_count].weight = w;
                g->edge_count++;
            
                if (u > g->node_count) g->node_count = u;
                if (v > g->node_count) g->node_count = v;
            } 
        } else {
            printf("  [BŁĄD] Linia %d: Niepoprawny format danych. Pominięto.\n", line_num);
        }
    }

    for (int i = 0; i < g->node_count; i++) g->nodes[i].id = i + 1;

    printf("\nPodsumowanie:\n - Wczytano poprawnie: %d krawędzi.\n", g->edge_count);
    printf(" - Przeanalizowano linii: %d.\n", line_num);
    printf("---------------------------\n");

    fclose(f);
    return 0;
}


int save_graph(Graph *g, const char *filename, const char *format) {
    if (strcmp(format, "bin") == 0) {
        // --- ZAPIS BINARNY ---
        FILE *f = fopen(filename, "wb");
        if (!f) return -1;

        // Zapisujemy najpierw liczbę wierzchołków, żeby wiedzieć ile czytać
        fwrite(&(g->node_count), sizeof(int), 1, f);
        
        // Zapisujemy całą tablicę struktur Node za jednym zamachem
        fwrite(g->nodes, sizeof(Node), g->node_count, f);

        fclose(f);
        printf("Graf zapisany binarnie do: %s\n", filename);
    } 
    else {
        // --- ZAPIS TEKSTOWY (Domyślny) ---
        FILE *f = fopen(filename, "w");
        if (!f) return -1;

        // Zapisujemy zgodnie ze specyfikacją projektu: <wierzchołek> <x> <y>
        for (int i = 0; i < g->node_count; i++) {
            fprintf(f, "%d %.2f %.2f\n", g->nodes[i].id, g->nodes[i].x, g->nodes[i].y);
        }
        fclose(f);
        printf("Graf zapisany tekstowo do: %s\n", filename);
    }
    return 0;
}

void free_graph(Graph *g) {
    if(g->nodes) free(g->nodes);
    if(g->edges) free(g->edges);
    g->nodes = NULL;
    g->edges = NULL;
}