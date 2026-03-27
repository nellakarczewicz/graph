#ifndef LAYOUT_TUTTE_H
#define LAYOUT_TUTTE_H

#include "graph.h"

/**
 * Główna funkcja wyliczająca pozycje wierzchołków metodą Tutte'a.
 * Zgodnie z tabelą: Implementacja metody barycentrycznej.
 */
void compute_tutte_layout(Graph *g);

/**
 * Funkcja pomocnicza: rozmieszcza wierzchołki zewnętrznej ściany 
 * na obwodzie koła, aby stworzyć ramę dla grafu.
 */
void initialize_tutte_fixed_nodes(Graph *g);

#endif // LAYOUT_TUTTE_H