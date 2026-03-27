#ifndef PARSER_H
#define PARSER_H

#include "graph.h"

#define MAX_LINE 256

/**
 * Główna funkcja procesująca plik wejściowy.
 * 1. Naprawia błędy użytkownika (separatory, kropki).
 * 2. Zapisuje czyste dane do pliku wyjściowego.
 * 3. Alokuje pamięć i wypełnia strukturę Graph.
 */
void process_and_load_graph(const char *input_path, const char *output_path, Graph *graph);

#endif