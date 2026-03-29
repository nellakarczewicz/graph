#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"
#include "io.h"

/**
 * Funkcja sprawdzająca spójność grafu za pomocą algorytmu przeszukiwania (DFS).
 *
 */
int is_graph_connected(Graph *g) {
    if (g->node_count <= 1) return 1;

    int *visited = calloc(g->node_count, sizeof(int));
    int *stack = malloc(g->node_count * sizeof(int));
    int top = -1;

    stack[++top] = 0;
    visited[0] = 1;
    int visited_count = 1;

    while (top >= 0) {
        int u = stack[top--];

        for (int i = 0; i < g->edge_count; i++) {
            int v = -1;
            if (g->edges[i].u_idx == u) v = g->edges[i].v_idx;
            else if (g->edges[i].v_idx == u) v = g->edges[i].u_idx;

            if (v != -1 && !visited[v]) {
                visited[v] = 1;
                stack[++top] = v;
                visited_count++;
            }
        }
    }

    free(visited);
    free(stack);
    return (visited_count == g->node_count);
}

int sanitize(char *s) {
    int changed = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == ',') {
            s[i] = '.';
            changed = 1;
        }
    }
    return changed;
}

static char detect_separator(const char *line) {
    return (strchr(line, ';') == NULL && strchr(line, ',') != NULL) ? ',' : ';';
}

/**
 * Raportuje stan pliku przed wczytaniem.
 * Uwzględnia teraz krawędzie bez nazw.
 */
int validate_and_process(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[256], name[50];
    int line_num = 0, critical_errors = 0, total_fixes = 0;
    int u, v;
    double w;

    printf("\n============================================================\n");
    printf("   RAPORT ANALIZY DANYCH WEJSCIOWYCH    \n");
    printf("============================================================\n");

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        if (strlen(line) <= 1 || line[0] == '\n') continue;

        char sep = detect_separator(line);
        int fixed = sanitize(line);
        if (fixed) total_fixes++;

        char format[64];
        int res;
        if (line[0] == sep) {
            sprintf(format, "%c%%d%c%%d%c%%lf", sep, sep, sep);
            res = sscanf(line, format, &u, &v, &w);
            if (res == 3) printf("%-7d | Krawedz (auto): %d -> %d | OK (brak nazwy)\n", line_num, u, v);
        } else {
            sprintf(format, " %%49[^%c]%c%%d%c%%d%c%%lf", sep, sep, sep, sep);
            res = sscanf(line, format, name, &u, &v, &w);
            if (res == 4) printf("%-7d | Krawedz %s: %d -> %d | OK\n", line_num, name, u, v);
        }

        if (res < 3) {
            printf("%-7d | [!!! BŁĄD KRYTYCZNY !!!]\n", line_num);
            critical_errors++;
        }
    }
    fclose(f);

    if (critical_errors > 0) return -1;
    return 0;
}

int is_potentially_planar(Graph *g) {
    if (g->node_count <= 2) return 1;
    return (g->edge_count <= (3 * g->node_count - 6));
}

/**
 * Czyta graf z pliku, obsługuje brak nazw i eliminuje duplikaty.
 */
int read_graph(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    g->edges = malloc(1000 * sizeof(Edge));
    g->nodes = calloc(1000, sizeof(Node));
    int id_map[10001]; 
    for(int i=0; i<10001; i++) id_map[i] = -1;

    g->edge_count = 0;
    g->node_count = 0;
    char line[256], name[50];
    int line_num = 0;

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        if (strlen(line) <= 1 || line[0] == '\n') continue;

        char sep = detect_separator(line);
        sanitize(line);

        int u_id, v_id;
        double w;
        int res;
        if (line[0] == sep) {
            char fstr[32];
            sprintf(fstr, "%c%%d%c%%d%c%%lf", sep, sep, sep);
            res = sscanf(line, fstr, &u_id, &v_id, &w);
        } else {
            char fstr[64];
            sprintf(fstr, " %%49[^%c]%c%%d%c%%d%c%%lf", sep, sep, sep, sep);
            res = sscanf(line, fstr, name, &u_id, &v_id, &w);
        }

        if (res >= 3) {
            if (u_id < 0 || u_id >= 10000 || v_id < 0 || v_id >= 10000) continue;

            if (id_map[u_id] == -1) {
                id_map[u_id] = g->node_count;
                g->nodes[g->node_count].id = u_id;
                g->node_count++;
            }
            if (id_map[v_id] == -1) {
                id_map[v_id] = g->node_count;
                g->nodes[g->node_count].id = v_id;
                g->node_count++;
            }

            int u_idx = id_map[u_id], v_idx = id_map[v_id];
            int is_duplicate = 0;
            for (int k = 0; k < g->edge_count; k++) {
                if ((g->edges[k].u_idx == u_idx && g->edges[k].v_idx == v_idx) ||
                    (g->edges[k].u_idx == v_idx && g->edges[k].v_idx == u_idx)) {
                    is_duplicate = 1;
                    break;
                }
            }

            if (!is_duplicate && g->edge_count < 1000) {
                g->edges[g->edge_count].u_idx = u_idx;
                g->edges[g->edge_count].v_idx = v_idx;
                g->edges[g->edge_count].weight = w;
                g->edge_count++;
            }
        }
    }
    fclose(f);
    return 0;
}

int save_graph(Graph *g, const char *filename, const char *format) {
    FILE *f = fopen(filename, (strcmp(format, "bin") == 0) ? "wb" : "w");
    if (!f) return -1;

    if (strcmp(format, "bin") == 0) {
        fwrite(&(g->node_count), sizeof(int), 1, f);
        fwrite(g->nodes, sizeof(Node), g->node_count, f);
    } else {
        for (int i = 0; i < g->node_count; i++) {
            fprintf(f, "%d %.2f %.2f\n", g->nodes[i].id, g->nodes[i].x, g->nodes[i].y);
        }
    }
    fclose(f);
    return 0;
}

void free_graph(Graph *g) {
    if(g->nodes) free(g->nodes);
    if(g->edges) free(g->edges);
    g->nodes = NULL; g->edges = NULL;
}