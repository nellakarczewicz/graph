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

/**
 * Funkcja sprawdzająca teoretyczną planarność grafu (warunek konieczny) w algorytmie Fruchtermana
 * Wykorzystuje twierdzenie Eulera dla grafów planarnych, sprawdzając zależność między liczbą krawędzi
 * a liczbą wierzchołków (E <= 3V - 6).
 * @param g wskaźnik na strukturę Graph zawierającą aktualne współrzędne wierzchołków i listę krawędzi
 * Zwraca: int (1 jeśli graf spełnia warunek i może być planarny, 0 jeśli liczba krawędzi 
 * wyklucza planarność bez względu na ułożenie wierzchołków)
 */
int is_potentially_planar(Graph *g) {
    // Grafy z 0, 1 lub 2 wierzchołkami są zawsze planarne
    if (g->node_count <= 2) return 1; 
    
    // Wzór Eulera dla grafów planarnych prostych: E <= 3V - 6
    int max_edges = 3 * g->node_count - 6;
    
    if (g->edge_count > max_edges) {
        return 0; // Za dużo krawędzi -> na pewno nie jest planarny
    }
    return 1; // Może być planarny
}

int read_graph(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    // Alokujemy pamięć na krawędzie
    g->edges = malloc(1000 * sizeof(Edge));
    // Alokujemy pamięć na wierzchołki (max 1000 unikalnych wierzchołków)
    // Używamy calloc, aby wyzerować dx, dy i inne pola
    g->nodes = calloc(1000, sizeof(Node));

    if (!g->edges || !g->nodes) {
        if (f) fclose(f);
        return -2;
    }
    
    // Tablica mapująca: id_map[ID_Z_PLIKU] = INDEKS_W_TABLICY_NODES
    // Inicjalizujemy -1, co oznacza "jeszcze nie widziałem tego ID"
    int id_map[10001]; 
    for(int i=0; i<10001; i++) id_map[i] = -1;

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
        char separator = detect_separator(line);
        if (separator == ',') {
            printf("  [Wskazówka] Linia %d: Wykryto przecinki zamiast średników. Próbuję przetworzyć...\n", line_num);
        }

        // 3. Czyszczenie (Sanitizing)
        sanitize(line);

        // 4. Parsowanie
        int u_id, v_id;
        double w;
        // Budujemy format parsowania w zależności od wykrytego separatora
        char format_str[50];
        // Dodanie ogranicznika 49 dla bezpieczeństwa
        sprintf(format_str, " %%49[^%c]%c%%d%c%%d%c%%lf", separator, separator, separator, separator);

        if (sscanf(line, format_str, name, &u_id, &v_id, &w) == 4) {
            // ZABEZPIECZENIE: sprawdzamy czy ID z pliku nie wykracza poza zakres mapy
            if (u_id < 0 || u_id >= 10000 || v_id < 0 || v_id >= 10000) continue;

            // Mapujemy wierzchołek U
            if (id_map[u_id] == -1) {
                if (g->node_count >= 1000) continue; // Osiągnięto limit 1000 wierzchołków
                id_map[u_id] = g->node_count;
                g->nodes[g->node_count].id = u_id;
                g->node_count++;
            }

            // Mapujemy wierzchołek V
            if (id_map[v_id] == -1) {
                if (g->node_count >= 1000) continue;
                id_map[v_id] = g->node_count;
                g->nodes[g->node_count].id = v_id;
                g->node_count++;
            }
            
            // Dodajemy krawędź używając ciągłych indeksów (0, 1, 2...)
            // Rezerwowe zabezpieczenie przed wyjściem poza 1000
            if (g->edge_count < 1000) {
                g->edges[g->edge_count].u_idx = id_map[u_id];
                g->edges[g->edge_count].v_idx = id_map[v_id];
                g->edges[g->edge_count].weight = w;
                g->edge_count++;
            
            } 
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