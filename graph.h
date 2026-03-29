#ifndef GRAPH_H
#define GRAPH_H

/**
 * Struktura Node: Reprezentuje pojedynczy punkt (wierzcholek) na mapie.
 * Przechowuje dane o polozeniu oraz sile nacisku w modelach fizycznych.
 */
typedef struct {
    int id;         // Unikalny numer identyfikacyjny z pliku zrodlowego
    double x, y;    // Wspolrzedne polozenia wierzcholka w przestrzeni 2D
    double dx, dy;  // Sila (wektor przesuniecia) wyliczana w kazdej iteracji algorytmu
} Node;

/**
 * Struktura Edge: Definiuje polaczenie miedzy dwoma punktami.
 * Uzywa indeksow tablicy zamiast ID, co przyspiesza dostep do danych o 100%.
 */
typedef struct {
    int u_idx;      // Indeks wezla poczatkowego w tablicy nodes
    int v_idx;      // Indeks wezla koncowego w tablicy nodes
    double weight;  // Wartosc numeryczna krawedzi (np. dlugosc polaczenia)
} Edge;

/**
 * Struktura Graph: Kontener agregujacy wszystkie zasoby grafu.
 * Pozwala na zarzadzanie calym ukladem jako jednym obiektem w pamieci.
 */
typedef struct {
    Node *nodes;        // Dynamiczna lista wszystkich punktow grafu
    Edge *edges;        // Dynamiczna lista wszystkich polaczen
    int node_count;     // Calkowita liczba punktow wczytanych do systemu
    int edge_count;     // Calkowita liczba polaczen miedzy punktami
    double width;       // Szerokosc obszaru roboczego (domyslnie 1000 jednostek)
    double height;      // Wysokosc obszaru roboczego (domyslnie 1000 jednostek)
} Graph;

//Funkcje obslugi grafu (Zarzadzanie danymi)

/*
  logika: Otwiera plik, rezerwuje pamiec RAM i mapuje dane tekstowe na struktury C.
 */
int read_graph(Graph *g, const char *filename);

/*
  logika: Przetwarza wspolrzedne x/y na format tekstowy lub binarny i zapisuje do pliku.
 */
int save_graph(Graph *g, const char *filename, const char *format);

/*
  logika: Czysci pamiec dynamiczna po zakonczeniu pracy, zapobiegajac wyciekom danych.
 */
void free_graph(Graph *g);

#endif