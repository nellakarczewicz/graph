#include "layout_tutte.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define MAX_ITERATIONS 200 // Liczba kroków przybliżania pozycji (zbieżność układu)

// --- Funkcje pomocnicze do weryfikacji geometrii (zaimplementowane analogicznie do Fruchtermana) ---

/**
 * Sprawdza orientację trzech punktów (A, B, C).
 * Zwraca 1, jeśli punkty są ułożone przeciwnie do ruchu wskazówek zegara (CCW).
 */
static int ccw(double ax, double ay, double bx, double by, double cx, double cy) {
    return (cy - ay) * (bx - ax) > (by - ay) * (cx - ax);
}

/**
 * Detekcja przecięcia dwóch odcinków (krawędzi) AB i CD.
 * Wykorzystuje test orientacji CCW.
 */
static int intersect(double x1, double y1, double x2, double y2, 
                     double x3, double y3, double x4, double y4) {
    return ccw(x1, y1, x3, y3, x4, y4) != ccw(x2, y2, x3, y3, x4, y4) &&
           ccw(x1, y1, x2, y2, x3, y3) != ccw(x1, y1, x2, y2, x4, y4);
}

/**
 * Weryfikuje, czy aktualne ułożenie grafu jest planarne (brak przecięć krawędzi).
 * Funkcja pozwala potwierdzić sukces matematycznej gwarancji Tutte'a.
 * @return 1 jeśli brak przecięć, 0 w przypadku znalezienia kolizji.
 */
int is_tutte_layout_planar(Graph *g) {
    for (int i = 0; i < g->edge_count; i++) {
        for (int j = i + 1; j < g->edge_count; j++) {
            int u1 = g->edges[i].u_idx, v1 = g->edges[i].v_idx;
            int u2 = g->edges[j].u_idx, v2 = g->edges[j].v_idx;

            // Pomiń krawędzie mające wspólny wierzchołek (stykają się tylko w końcach)
            if (u1 == u2 || u1 == v2 || v1 == u2 || v1 == v2) continue;

            if (intersect(g->nodes[u1].x, g->nodes[u1].y, g->nodes[v1].x, g->nodes[v1].y,
                          g->nodes[u2].x, g->nodes[u2].y, g->nodes[v2].x, g->nodes[v2].y)) {
                return 0; 
            }
        }
    }
    return 1;
}

// --- Główna logika algorytmu Tutte'a ---

/**
 * Inicjalizacja "ramy" grafu (Fixed Nodes).
 * Zgodnie z twierdzeniem Tutte'a, wierzchołki zewnętrzne muszą tworzyć wielokąt wypukły.
 * Wybieramy pierwsze 4 wierzchołki i rozstawiamy je na obwodzie koła.
 */
void initialize_tutte_fixed_nodes(Graph *g) {
    if (g->node_count < 3) return;

    double centerX = g->width / 2.0;
    double centerY = g->height / 2.0;
    double radius = (g->width < g->height ? g->width : g->height) / 3.0;

    // Rozstawienie wierzchołków "zakotwiczonych" (indeksy 0-3)
    for (int i = 0; i < 4 && i < g->node_count; i++) {
        double angle = 2.0 * M_PI * i / 4.0;
        g->nodes[i].x = centerX + radius * cos(angle);
        g->nodes[i].y = centerY + radius * sin(angle);
    }

    // Pozostałe wierzchołki (ruchome) startują ze środka obszaru
    for (int i = 4; i < g->node_count; i++) {
        g->nodes[i].x = centerX;
        g->nodes[i].y = centerY;
    }
}

/**
 * Implementacja Metody Barycentrycznej (Tutte's Algorithm).
 * Każdy ruchomy wierzchołek jest przesuwany do geometrycznego środka ciężkości swoich sąsiadów.
 * Proces jest powtarzany iteracyjnie aż do stabilizacji układu.
 */
void compute_tutte_layout(Graph *g) {
    if (g->node_count < 3) return;

    // 1. Ustalenie pozycji wierzchołków zewnętrznych (rama)
    initialize_tutte_fixed_nodes(g);

    // 2. Iteracyjne rozwiązywanie układu równań (metoda relaksacji)
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        
        // Aktualizujemy tylko wierzchołki wewnętrzne (indeksy >= 4)
        for (int i = 4; i < g->node_count; i++) {
            double sum_x = 0;
            double sum_y = 0;
            int neighbors_count = 0;

            // Iteracja po krawędziach w poszukiwaniu sąsiedztwa
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

            // Przesunięcie wierzchołka do średniej pozycji sąsiadów (barycentrum)
            if (neighbors_count > 0) {
                g->nodes[i].x = sum_x / neighbors_count;
                g->nodes[i].y = sum_y / neighbors_count;
            }
        }
    }
}