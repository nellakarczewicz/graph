#ifndef IO_H
#define IO_H
#include "graph.h"

/**
 * Funkcja analizuje plik wejściowy z danymi krawędzi.
 * Sprawdza poprawność formatu, separatorów, nazw, ID wierzchołków i wag.
 * Wykonuje automatyczne poprawki separatora dziesiętnego (',' → '.').
 * @param filename - plik wejściowy

* Zwraca:
 *   0  → jeśli plik jest poprawny (lub poprawialny)
 *  -1  → jeśli wykryto błędy krytyczne

*/
int validate_and_process(const char *filename);

/**
*
 * Wczytuje graf z pliku tekstowego po wcześniejszej walidacji.
 * Funkcja:
 *  - mapuje ID wierzchołków (1–1000) na indeksy tablicowe (0–999),
 *  - ignoruje puste linie i linie bez danych,
 *  - usuwa duplikaty krawędzi,
 *  - filtruje pętle własne i ID spoza zakresu,
 *  - tworzy tablice nodes[] i edges[].
 * @param g - wskaźnik na strukturę Graph
 * @param filename - plik wejściowy
 * Zwraca:
 *   0  → sukces
 *  -1  → błąd (np. brak pliku, błąd alokacji)
*/
int read_graph(Graph *g, const char *filename);

/*
* Zapisuje graf do pliku w formacie tekstowym lub binarnym.
 * @param g - wskaźnik na strukturę Graph
 * @param filename - plik wejściowy
 * @param format - format
 * Zwraca:
 *   0  → sukces
 *  -1  → błąd otwarcia pliku
*/
int save_graph(Graph *g, const char *filename, const char *format);

/*
* Zwalnia pamięć zaalokowaną dla grafu.
* @param g - wskaźnik na strukturę Graph
* zwraca void
*/
void free_graph(Graph *g);

/**
* Funkcja sprawdzająca spójność grafu za pomocą algorytmu przeszukiwania (DFS).
* @param g wskaźnik na strukturę Graph zawierającą listę wierzchołków i krawędzi
* @return int:
 *         - 1 jeśli graf jest spójny,
 *         - 0 jeśli graf jest niespójny,
 *         - -1 jeśli wystąpił błąd alokacji pamięci.
*/
int is_graph_connected(Graph *g);

/*
* Normalizuje zapis liczby w łańcuchu znaków, zamieniając przecinki na kropki.
* @param s wskaźnik na modyfikowalny łańcuch znaków
* @return int:
 *         - 1 jeśli dokonano przynajmniej jednej zamiany,
 *         - 0 jeśli łańcuch pozostał bez zmian.
*/
int sanitize(char *s);

/*
 * Wykrywa separator pól w pojedynczej linii tekstu.
 * @param line wskaźnik na łańcuch znaków reprezentujący jedną linię danych
 *
 * @return char:
 *         - ',' jeśli wykryto format z przecinkami,
 *         - ';' w pozostałych przypadkach.
 */

static char detect_separator(const char *line);

#endif