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

/**
 * Funkcja sprawdzająca planarność grafu w algorytmie Fruchtermana
 * Wykorzystuje algorytm orientacji punktów (CCW) do detekcji kolizji odcinków.
 * @param g wskaźnik na strukturę Graph zawierającą aktualne współrzędne wierzchołków i listę krawędzi
 * Zwraca: int (1 jeśli układ jest planarny - brak przecięć, 0 w przypadku znalezienia chociaż jednego przecięcia)
 */
int is_layout_planar(Graph *g);

/**
 * Funkcja pomocnicza: sprawdza, czy punkty są ułożone przeciwnie do ruchu wskazówek zegara.
 * Wykorzystywana do detekcji przecięć odcinków.
 * Parametry wejściowe to współrzędne punktów 
 * Zwraca: int (1 jeśli orientacja jest CCW, 0 w przeciwnym przypadku)
 */
int ccw(double ax, double ay, double bx, double by, double cx, double cy);

/**
 * Funkcja sprawdzająca, czy dwa odcinki (x1,y1)-(x2,y2) oraz (x3,y3)-(x4,y4) przecinają się.
 * Implementacja algorytmu opartego na orientacji punktów (CCW).
 * Parametry wejściowe to współrzędne punktów pierwszego i drugiego odcinka
 * Zwraca: int (1 jeśli odcinki się przecinają, 0 jeśli nie)
 */
int intersect(double x1, double y1, double x2, double y2, 
              double x3, double y3, double x4, double y4);

#endif