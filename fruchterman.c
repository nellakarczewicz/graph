#include "fruchterman.h" // dołączenie pliku nagłówkowego z deklaracjami funkcji
#include <math.h> // do sqrt()
#include <stdlib.h> // do rand()
#include <time.h> // do time()

// --- Implementacja funkcji pomocniczych ---

/**
 * Funkcja obliczająca siłę przyciągania (attractive force)
 * f_a(d) = d^2 / k
 * @param dist odległość geometryczna między dwoma wierzchołkami w danym momencie symulacji
 * @param k stała sprężystości/optymalna odległość
 * Zwraca: double (wartość siły)
 */
double force_attraction(double dist, double k) {
    return (dist * dist) / k;
}

/**
 * Funkcja obliczająca siłę odpychania (repulsive force)
 * f_r(d) = k^2 / d
 * @param dist odległość geometryczna między dwoma wierzchołkami w danym momencie symulacji
 * @param k stała sprężystości/optymalna odległość
 * Zwraca: double (wartość siły)
 */
double force_repulsion(double dist, double k) {
    return (k * k) / dist;
}

/**
 * Funkcja pomocnicza do inicjalizacji pozycji wierzchołków.
 * Rozmieszcza wierzchołki losowo wewnątrz obszaru zdefiniowanego w Graph.
 * Powinna być wywołana raz przed pętlą algorytmu.
 * @param graph wskaźnik na strukturę przechowującą wierzchołki i krawędzie
 * Zwraca: void
 */
void init_random_positions(Graph *graph) {
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych
    for (int i = 0; i < graph->node_count; i++) {
        // Losuje X od 0 do szerokości obszaru
        graph->nodes[i].x = (double)rand() / RAND_MAX * graph->width;
        // Losuje Y od 0 do wysokości obszaru
        graph->nodes[i].y = (double)rand() / RAND_MAX * graph->height;
        // Wyzerowuje wektory przesunięcia na start
        graph->nodes[i].dx = 0;
        graph->nodes[i].dy = 0;
    }
}

// --- Główny algorytm ---

/**
 * Główna funkcja wykonująca algorytm Fruchterman-Reingold.
 * Przeprowadza symulację fizyczną, aktualizując współrzędne w strukturze Graph.
 * @param graph wskaźnik na strukturę przechowującą wierzchołki i krawędzie
 * @param iterations liczba kroków symulacji
 * @param temp_start temperatura początkowa
 * Zwraca: void
 */
void run_fruchterman_reingold(Graph *graph, int iterations, double temp_start) {
    double area = graph->width * graph->height; // Oblicza pole powierzchni rysowania
    double k = sqrt(area / graph->node_count); // Oblicza idealną odległość między wierzchołkami
    double t = temp_start; // Aktualna temperatura

    for (int iter = 0; iter < iterations; iter++) {
        
        // 1. ODPYCHANIE: Każdy wierzchołek odpycha się od każdego innego
        for (int i = 0; i < graph->node_count; i++) {
            graph->nodes[i].dx = 0; // Resetuje siłę X dla tego wierzchołka
            graph->nodes[i].dy = 0; // Resetuje siłę Y dla tego wierzchołka

            for (int j = 0; j < graph->node_count; j++) {
                if (i == j) continue; // Wierzchołek nie odpycha samego siebie

                // Oblicza wektor różnicy pozycji między wierzchołkiem i a wierzchołkiem j
                double vx = graph->nodes[i].x - graph->nodes[j].x; 
                double vy = graph->nodes[i].y - graph->nodes[j].y;
                double dist = sqrt(vx * vx + vy * vy); // Odległość między nimi (z tw Pitagorasa)

                // Unikanie dzielenia przez zero, jeśli są w tym samym miejscu
                if (dist < 0.01) dist = 0.01; 

                double force = force_repulsion(dist, k); // Oblicza siłę odpychania
                graph->nodes[i].dx += (vx / dist) * force; // Dodaje składową X siły do wierzchołka i
                graph->nodes[i].dy += (vy / dist) * force; // Dodaje składową Y siły do wierzchołka i
            }
        }

        // 2. PRZYCIĄGANIE: Tylko wierzchołki połączone krawędzią
        for (int i = 0; i < graph->edge_count; i++) {
            int u = graph->edges[i].u_idx; // Indeks pierwszego końca krawędzi
            int v = graph->edges[i].v_idx; // Indeks drugiego końca krawędzi

            // Oblicza wektor między dwoma końcami krawędzi
            double vx = graph->nodes[u].x - graph->nodes[v].x;
            double vy = graph->nodes[u].y - graph->nodes[v].y;
            double dist = sqrt(vx * vx + vy * vy); // Odległość końców krawędzi

            // Unikanie dzielenia przez zero, jeśli są w tym samym miejscu
            if (dist < 0.01) dist = 0.01;

            double force = force_attraction(dist, k); // Oblicza siłę przyciągania
            double fx = (vx / dist) * force; // siła składowa X
            double fy = (vy / dist) * force; // siła składowa Y

            // Przesuwamy oba końce ku sobie, U odejmujemy, V dodajemy
            // (siły działają w przeciwnych kierunkach)
            graph->nodes[u].dx -= fx;
            graph->nodes[u].dy -= fy;
            graph->nodes[v].dx += fx;
            graph->nodes[v].dy += fy;
        }

        // 3. AKTUALIZACJA POZYCJI: Przesunięcie ograniczone temperaturą
        for (int i = 0; i < graph->node_count; i++) {
            // Oblicza całkowitą siłę (długość wektora przesunięcia)
            double disp_dist = sqrt(graph->nodes[i].dx * graph->nodes[i].dx + 
                                   graph->nodes[i].dy * graph->nodes[i].dy);

            if (disp_dist > 0) {
                // Ograniczenie przemieszczenia temperaturą t 
                double limited_dist = (disp_dist < t) ? disp_dist : t;
                // Aktualizacja współrzędnej X
                graph->nodes[i].x += (graph->nodes[i].dx / disp_dist) * limited_dist;
                // Aktualizacja współrzędnej Y
                graph->nodes[i].y += (graph->nodes[i].dy / disp_dist) * limited_dist;
            }

            // Ograniczenie do ramki (zapobiega ucieczce punktów)
            if (graph->nodes[i].x < 0) graph->nodes[i].x = 0; //lewa krawędź
            if (graph->nodes[i].x > graph->width) graph->nodes[i].x = graph->width;//prawa krawędź
            if (graph->nodes[i].y < 0) graph->nodes[i].y = 0; //górna krawędź
            if (graph->nodes[i].y > graph->height) graph->nodes[i].y = graph->height; //dolna krawędź
        }

        // 4. CHŁODZENIE: Zmniejszanie temperatury
        t *= 0.95; // Zmniejszenie temperatury o 5% (dzięki temu wierzchołki wykonają w następnej pętli mniejsze skoki)
    }
}