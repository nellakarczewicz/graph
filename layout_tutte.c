#include "layout_tutte.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define MAX_ITERATIONS 200 // Liczba kroków przybliżania pozycji

void initialize_tutte_fixed_nodes(Graph *g) {
    if (g->node_count < 3) return;

    // Wybieramy pierwsze 4 wierzchołki jako "ścianę zewnętrzną"
    // i rozstawiamy je na planie koła wpisanego w obszar roboczy.
    double centerX = g->width / 2.0;
    double centerY = g->height / 2.0;
    double radius = (g->width < g->height ? g->width : g->height) / 3.0;

    for (int i = 0; i < 4 && i < g->node_count; i++) {
        double angle = 2.0 * M_PI * i / 4.0;
        g->nodes[i].x = centerX + radius * cos(angle);
        g->nodes[i].y = centerY + radius * sin(angle);
    }

    // Pozostałe wierzchołki (ruchome) ustawiamy wstępnie w centrum
    for (int i = 4; i < g->node_count; i++) {
        g->nodes[i].x = centerX;
        g->nodes[i].y = centerY;
    }
}

void compute_tutte_layout(Graph *g) {
    if (g->node_count < 3) return;

    // 1. Przygotowanie "ramy" (fixed nodes)
    initialize_tutte_fixed_nodes(g);

    // 2. Główna pętla algorytmu (Metoda Barycentryczna)
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        
        // Obliczamy nowe pozycje tylko dla wierzchołków ruchomych (i >= 4)
        for (int i = 4; i < g->node_count; i++) {
            double sum_x = 0;
            double sum_y = 0;
            int neighbors_count = 0;

            // Szukamy sąsiadów wierzchołka i w tablicy krawędzi
            for (int j = 0; j < g->edge_count; j++) {
                int neighbor_idx = -1;
                
                if (g->edges[j].u_idx == i) {
                    neighbor_idx = g->edges[j].v_idx;
                } else if (g->edges[j].v_idx == i) {
                    neighbor_idx = g->edges[j].u_idx;
                }

                if (neighbor_idx != -1) {
                    sum_x += g->nodes[neighbor_idx].x;
                    sum_y += g->nodes[neighbor_idx].y;
                    neighbors_count++;
                }
            }

            // Wierzchołek ląduje w średniej arytmetycznej swoich sąsiadów
            if (neighbors_count > 0) {
                g->nodes[i].x = sum_x / neighbors_count;
                g->nodes[i].y = sum_y / neighbors_count;
            }
        }
    }
}