#ifndef GRAPH_H
#define GRAPH_H

typedef struct {
    int id;
    double x, y;
    double dx, dy; 
} Node;

typedef struct {
    int u_idx;
    int v_idx;
    double weight;
} Edge;

typedef struct {
    Node *nodes;
    Edge *edges;
    int node_count;
    int edge_count;
    double width;
    double height;
} Graph;

//do czytania i zapisywania itd
int read_graph(Graph *g, const char *filename);
int save_graph(Graph *g, const char *filename, const char *format);
void free_graph(Graph *g);

#endif