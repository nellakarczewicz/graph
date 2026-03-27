#ifndef IO_H
#define IO_H
#include "graph.h"

int read_graph(Graph *g, const char *filename);
int save_graph(Graph *g, const char *filename, const char *format);
void free_graph(Graph *g);

#endif