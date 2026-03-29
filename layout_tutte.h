#ifndef LAYOUT_TUTTE_H
#define LAYOUT_TUTTE_H

#include "graph.h"

/**
 * Główna funkcja wyliczająca pozycje wierzchołków metodą Tutte'a.
 */
void compute_tutte_layout(Graph *g);

/**
 * Funkcja pomocnicza: rozmieszcza wierzchołki zewnętrznej ściany na obwodzie koła.
 */
void initialize_tutte_fixed_nodes(Graph *g);

/**
 * Weryfikuje, czy ułożenie Tutte'a jest planarne (brak przecięć).
 * Zwraca 1 dla sukcesu, 0 dla porażki.
 */
int is_tutte_layout_planar(Graph *g);

#endif