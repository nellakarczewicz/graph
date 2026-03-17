#ifndef GRAPH_H
#define GRAPH_H

// Definicja struktur wierzchołka, krawędzi oraz grafu

/**
 * Struktura reprezentująca wierzchołek grafu (Node)
 */
typedef struct {
    int id;             // oryginalne ID z pliku wejściowego
    double x, y;        // aktualne współrzędne
    
    // do Fruchterman-Reingold
    double dx, dy;      // wektory sił (przemieszczenia) w danej iteracji

} Node;

/**
 * Struktura reprezentująca krawędź grafu (Edge)
 */
typedef struct {
    int u_idx;          // indeks wierzchołka źródłowego w tablicy nodes[]
    int v_idx;          // indeks wierzchołka docelowego w tablicy nodes[]
    double weight;      // waga krawędzi (z pliku wejściowego)
} Edge;

/**
 * Główna struktura przechowująca cały graf
 */
typedef struct {
    Node *nodes;        // dynamiczna tablica wierzchołków
    Edge *edges;        // dynamiczna tablica krawędzi
    int node_count;     // liczba wierzchołków
    int edge_count;     // liczba krawędzi
    double width;       // szerokość obszaru roboczego
    double height;      // wysokość obszaru roboczego
} Graph;

#endif // GRAPH_H