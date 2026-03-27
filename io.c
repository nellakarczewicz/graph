#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

int read_graph(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char edge_name[50];
    int u, v;
    double w;
    
    // Prosta alokacja na start (można rozbudować o dynamiczne realloc)
    g->edges = malloc(1000 * sizeof(Edge)); 
    g->nodes = malloc(1000 * sizeof(Node));
    g->edge_count = 0;
    g->node_count = 0;

    while (fscanf(f, "%s %d %d %lf", edge_name, &u, &v, &w) == 4) {
        g->edges[g->edge_count].u_idx = u - 1; // Mapowanie na indeksy 0..n-1
        g->edges[g->edge_count].v_idx = v - 1;
        g->edges[g->edge_count].weight = w;
        g->edge_count++;
        
        if (u > g->node_count) g->node_count = u;
        if (v > g->node_count) g->node_count = v;
    }

    // Inicjalizacja pozycji wierzchołków na 0,0 przed algorytmem
    for(int i=0; i < g->node_count; i++) {
        g->nodes[i].x = 0; g->nodes[i].y = 0;
    }

    fclose(f);
    return 0;
}

int save_graph(Graph *g, const char *filename, const char *format) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    for (int i = 0; i < g->node_count; i++) {
        fprintf(f, "%d %.2f %.2f\n", i + 1, g->nodes[i].x, g->nodes[i].y);
    }

    fclose(f);
    return 0;
}

void free_graph(Graph *g) {
    if(g->nodes) free(g->nodes);
    if(g->edges) free(g->edges);
}