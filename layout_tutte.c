#include "layout_tutte.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define MAX_ITERATIONS 200 

// --- Funkcje pomocnicze do weryfikacji geometrii ---

/**
 * arg wejscia: ax, ay, bx, by, cx, cy (wspolrzedne trzech punktow)
 * arg wyjscia: 1 (jesli orientacja CCW), 0 (w przeciwnym razie)
 * logika/funkcja: Sprawdza, czy punkty tworza zakret w lewo; uzywane do badania przeciec krawedzi.
 */
static int ccw(double ax, double ay, double bx, double by, double cx, double cy) {
    // Obliczanie iloczynu wektorowego dla trzech punktów
    return (cy - ay) * (bx - ax) > (by - ay) * (cx - ax);
}

/**
 * arg wejscia: x1, y1, x2, y2, x3, y3, x4, y4 (wspolrzedne dwoch odcinkow)
 * arg wyjscia: 1 (przecinaja sie), 0 (nie przecinaja sie)
 * logika/funkcja: Wykorzystuje test CCW, aby stwierdzic, czy dwie krawedzie grafu krzyzuja sie ze soba.
 */
static int intersect(double x1, double y1, double x2, double y2, 
                     double x3, double y3, double x4, double y4) {
    // Odcinki przecinaja sie, gdy konce jednego leza po przeciwnych stronach drugiego
    return ccw(x1, y1, x3, y3, x4, y4) != ccw(x2, y2, x3, y3, x4, y4) &&
           ccw(x1, y1, x2, y2, x3, y3) != ccw(x1, y1, x2, y2, x4, y4);
}

/**
 * arg wejscia: Graph *g (wskaznik na strukture grafu)
 * arg wyjscia: 1 (uklad jest planarny), 0 (znaleziono przeciecia)
 * logika/funkcja: Iteruje po wszystkich parach krawedzi, sprawdzajac, czy wynik algorytmu Tutte'a jest poprawny geometrycznie.
 */
int is_tutte_layout_planar(Graph *g) {
    for (int i = 0; i < g->edge_count; i++) {
        for (int j = i + 1; j < g->edge_count; j++) {
            int u1 = g->edges[i].u_idx, v1 = g->edges[i].v_idx;
            int u2 = g->edges[j].u_idx, v2 = g->edges[j].v_idx;

            // Pominiecie krawedzi sasiadujacych (wspolny wezel)
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
 * arg wejscia: Graph *g (wskaznik na strukture grafu)
 * arg wyjscia: brak (modyfikuje wspolrzedne wezlow w strukturze)
 * logika/funkcja: Rozmieszcza pierwsze 4 wezly na okregu (tworzy wypukla rame), a reszte ustawia w srodku ekranu jako punkt startowy.
 */
void initialize_tutte_fixed_nodes(Graph *g) {
    if (g->node_count < 3) return;

    double centerX = g->width / 2.0;
    double centerY = g->height / 2.0;
    double radius = (g->width < g->height ? g->width : g->height) / 3.0;

    // Ustawienie wezlow zablokowanych (rama zewnetrzna) na okregu
    for (int i = 0; i < 4 && i < g->node_count; i++) {
        double angle = 2.0 * M_PI * i / 4.0;
        g->nodes[i].x = centerX + radius * cos(angle);
        g->nodes[i].y = centerY + radius * sin(angle);
    }

    // Centrowanie wezlow ruchomych przed rozpoczeciem obliczen
    for (int i = 4; i < g->node_count; i++) {
        g->nodes[i].x = centerX;
        g->nodes[i].y = centerY;
    }
}

/**
 * arg wejscia: Graph *g (wskaznik na strukture grafu)
 * arg wyjscia: brak (modyfikuje wspolrzedne wezlow w strukturze)
 * logika/funkcja: Glowna petla algorytmu; przez 200 iteracji przesuwa kazdy ruchomy wezel do sredniej pozycji jego sasiadow (barycentrum).
 */
void compute_tutte_layout(Graph *g) {
    if (g->node_count < 3) return;

    // Przygotowanie ramy i pozycji startowych
    initialize_tutte_fixed_nodes(g);

    // Glowna petla relaksacji barycentrycznej
    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        // Przetwarzanie tylko wezlow ruchomych (indeksy >= 4)
        for (int i = 4; i < g->node_count; i++) {
            double sum_x = 0;
            double sum_y = 0;
            int neighbors_count = 0;

            // Szukanie sasiadow wezla i poprzez przegladanie krawedzi
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

            // Aktualizacja pozycji do srodka ciezkosci sasiadow
            if (neighbors_count > 0) {
                g->nodes[i].x = sum_x / neighbors_count;
                g->nodes[i].y = sum_y / neighbors_count;
            }
        }
    }
}