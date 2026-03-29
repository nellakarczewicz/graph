#include "fruchterman.h" // dołączenie pliku nagłówkowego z deklaracjami funkcji
#include <math.h> // do sqrt()
#include <stdlib.h> // do rand()

// --- Implementacja funkcji pomocniczych ---

/**
 * Funkcja pomocnicza: sprawdza, czy punkty są ułożone przeciwnie do ruchu wskazówek zegara.
 * Wykorzystywana do detekcji przecięć odcinków.
 */
int ccw(double ax, double ay, double bx, double by, double cx, double cy) {
    return (cy - ay) * (bx - ax) > (by - ay) * (cx - ax);
}

/**
 * Czy dwa odcinki (krawędzie) się przecinają?
 * Implementacja algorytmu opartego na orientacji punktów (CCW).
 */
int intersect(double x1, double y1, double x2, double y2, 
              double x3, double y3, double x4, double y4) {
    return ccw(x1, y1, x3, y3, x4, y4) != ccw(x2, y2, x3, y3, x4, y4) &&
           ccw(x1, y1, x2, y2, x3, y3) != ccw(x1, y1, x2, y2, x4, y4);
}

/**
 * Funkcja sprawdzająca planarność grafu w algorytmie Fruchtermana
 * Wykorzystuje algorytm orientacji punktów (CCW) do detekcji kolizji odcinków.
 * @param g wskaźnik na strukturę Graph zawierającą aktualne współrzędne wierzchołków i listę krawędzi
 * Zwraca: int (1 jeśli układ jest planarny - brak przecięć, 0 w przypadku znalezienia chociaż jednego przecięcia)
 */
int is_layout_planar(Graph *g) {
    for (int i = 0; i < g->edge_count; i++) {
        for (int j = i + 1; j < g->edge_count; j++) {
            int u1 = g->edges[i].u_idx, v1 = g->edges[i].v_idx;
            int u2 = g->edges[j].u_idx, v2 = g->edges[j].v_idx;

            // Pomiń krawędzie dzielące wspólny wierzchołek (one zawsze się stykają w końcach)
            if (u1 == u2 || u1 == v2 || v1 == u2 || v1 == v2) continue;

            if (intersect(g->nodes[u1].x, g->nodes[u1].y, g->nodes[v1].x, g->nodes[v1].y,
                          g->nodes[u2].x, g->nodes[u2].y, g->nodes[v2].x, g->nodes[v2].y)) {
                return 0; // Znaleziono przecięcie!
            }
        }
    }
    return 1; // Brak przecięć - sukces!
}

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
 * Zamiast losować po całym ekranie, startujemy ze środka.
 * To zapewnia, że graf "rośnie" ze wspólnego punktu, co trzyma go w spójności.
 * @param graph wskaźnik na strukturę przechowującą wierzchołki i krawędzie
 * Zwraca: void
 */
void init_random_positions(Graph *graph) {
    double centerX = graph->width / 2.0;
    double centerY = graph->height / 2.0;
    
    // Inicjalizacja generatora powinna być w main, ale upewniamy się tutaj
    for (int i = 0; i < graph->node_count; i++) {
        // Startujemy w małym obszarze 10x10 na środku
        // Dzięki temu siły odpychania "wypchną" graf symetrycznie na zewnątrz
        graph->nodes[i].x = (double)rand() / RAND_MAX * 10.0 - 5.0; // Losowanie X
        graph->nodes[i].y = (double)rand() / RAND_MAX * 10.0 - 5.0; // Losowanie Y
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
void run_fruchterman(Graph *graph, int iterations, double temp_start) {
    double area = graph->width * graph->height; // Oblicza pole powierzchni rysowania
    double k = sqrt(area / graph->node_count); // Oblicza idealną odległość między wierzchołkami
    double t = temp_start; // Aktualna temperatura

    // Definiujemy parametry grawitacji raz przed pętlami
    double gravity = 0.05;
    double centerX = graph->width / 2.0;
    double centerY = graph->height / 2.0;

    for (int iter = 0; iter < iterations; iter++) {
        
        // 1. ODPYCHANIE: Każdy wierzchołek odpycha się od każdego innego; + GRAWITACJA
        for (int i = 0; i < graph->node_count; i++) {
            graph->nodes[i].dx = 0; // Resetuje siłę X dla tego wierzchołka
            graph->nodes[i].dy = 0; // Resetuje siłę Y dla tego wierzchołka

            // Grawitacja - działa na każdy wierzchołek niezależnie od krawędzi
            // Obliczamy wektor od wierzchołka do środka ekranu
            double dx_center = centerX - graph->nodes[i].x;
            double dy_center = centerY - graph->nodes[i].y;
            // Dodajemy siłę grawitacji do wektora przesunięcia
            graph->nodes[i].dx += dx_center * gravity;
            graph->nodes[i].dy += dy_center * gravity;

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
            // Kierunek wektora siły
            double fx = (vx / dist) * force; // siła składowa X
            double fy = (vy / dist) * force; // siła składowa Y

            // Przesuwamy oba końce ku sobie, U odejmujemy, V dodajemy
            // (siły działają w przeciwnych kierunkach)
            graph->nodes[u].dx -= fx; // u przesuwa się w stronę v
            graph->nodes[u].dy -= fy;
            graph->nodes[v].dx += fx; // v przesuwa się w stronę u
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