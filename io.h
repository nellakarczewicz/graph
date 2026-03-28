#ifndef IO_H
#define IO_H
#include "graph.h"

/**
 * Wyświetla podgląd danych z pliku i czeka na akceptację użytkownika (Enter).
 */
int validate_and_process(const char *filename);

/**
 * Wczytuje dane grafu z pliku do struktury Graph.
 * Zwraca 0 przy sukcesie, wartości ujemne przy błędach.
 */
int read_graph(Graph *g, const char *filename);

/**
 * Zapisuje współrzędne wierzchołków grafu do pliku.
 */
int save_graph(Graph *g, const char *filename, const char *format);

/**
 * Zwalnia pamięć zaalokowaną dla wierzchołków i krawędzi grafu.
 */
void free_graph(Graph *g);

/**
 * Zapewnia planarność dla algorytmu Fruchtermana.
 */
int is_potentially_planar(Graph *g);

/**
 * Funkcja sprawdzająca spójność grafu za pomocą prostego algorytmu przeszukiwania (BFS/DFS).
 * @param g wskaźnik na strukturę Graph
 * Zwraca: 1 jeśli graf jest spójny, 0 w przeciwnym razie.
 */
int is_graph_connected(Graph *g);

#endif