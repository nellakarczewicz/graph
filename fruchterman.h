#ifndef LAYOUT_FRUCHTERMAN_H
#define LAYOUT_FRUCHTERMAN_H

#include "graph.h"

/**
 * Główna funkcja wykonująca algorytm Fruchterman-Reingold.
 * Przeprowadza symulację fizyczną, aktualizując współrzędne w strukturze Graph.
 * @param graph wskaźnik na strukturę przechowującą wierzchołki i krawędzie
 * @param iterations liczba kroków symulacji
 * @param temp_start temperatura początkowa
 * Zwraca: void
 */
void run_fruchterman(Graph *graph, int iterations, double temp_start);

/**
 * Funkcja pomocnicza do inicjalizacji pozycji wierzchołków.
 * Rozmieszcza wierzchołki losowo wewnątrz obszaru zdefiniowanego w Graph.
 * Powinna być wywołana raz przed pętlą algorytmu.
 * @param graph wskaźnik na strukturę przechowującą wierzchołki i krawędzie
 * Zwraca: void
 */
void init_random_positions(Graph *graph);

/**
 * Funkcja obliczająca siłę przyciągania (attractive force)
 * f_a(d) = d^2 / k
 * @param dist odległość geometryczna między dwoma wierzchołkami w danym momencie symulacji
 * @param k stała sprężystości/optymalna odległość
 * Zwraca: double (wartość siły)
 */
double force_attraction(double dist, double k);

/**
 * Funkcja obliczająca siłę odpychania (repulsive force)
 * f_r(d) = k^2 / d
 * @param dist odległość geometryczna między dwoma wierzchołkami w danym momencie symulacji
 * @param k stała sprężystości/optymalna odległość
 * Zwraca: double (wartość siły)
 */
double force_repulsion(double dist, double k);

int is_layout_planar(Graph *g);

#endif