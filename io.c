#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"
#include "io.h"

// arg wejscia: Graph *g (wskaznik na strukture grafu)
// arg wyjscia: 1 (spojny), 0 (niespojny), -1 (blad pamieci)
// logika/funkcja: Sprawdza spójność grafu DFS, aby upewnić się, że graf nie jest rozbity.
int is_graph_connected(Graph *g) {
    if (g->node_count <= 1) return 1; // Graf z jednym wezlem jest zawsze spojny

    // Alokacja tablicy odwiedzin i stosu dla algorytmu DFS
    int *visited = calloc(g->node_count, sizeof(int));
    int *stack = malloc(g->node_count * sizeof(int));

    if (!visited || !stack) { // Sprawdzenie czy system przyznal pamiec
        free(visited); free(stack);
        return -1; 
    }

    int top = -1;
    stack[++top] = 0; // Wrzucamy pierwszy wierzcholek na stos
    visited[0] = 1;   // Oznaczamy go jako odwiedzony
    int visited_count = 1;

    while (top >= 0) {
        int u = stack[top--]; // Pobieramy wierzcholek ze stosu

        for (int i = 0; i < g->edge_count; i++) {
            int v = -1;
            // Szukamy sasiadow wierzcholka 'u' w tablicy krawedzi
            if (g->edges[i].u_idx == u) v = g->edges[i].v_idx;
            else if (g->edges[i].v_idx == u) v = g->edges[i].u_idx;

            // Jesli sasiad 'v' istnieje i nie byl jeszcze odwiedzony
            if (v != -1 && !visited[v]) {
                visited[v] = 1;      // Zaznaczamy jako odwiedzony
                stack[++top] = v;    // Dodajemy go na stos do dalszej analizy
                visited_count++;     // Zwiekszamy licznik znalezionych wezlow
            }
        }
    }

    free(visited);
    free(stack);
    // Jesli odwiedzilismy tyle wezlow, ile jest w grafie, to jest spojny
    return (visited_count == g->node_count); 
}

// arg wejscia: char *s (linia tekstu)
// arg wyjscia: 1 (zmiana), 0 (brak)
// logika/funkcja: Zamienia przecinki na kropki w wagach krawedzi.
int sanitize(char *s) {
    int changed = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == ',') { // Jesli znajdziemy przecinek
            s[i] = '.';    // Zamieniamy go na kropke (format double w C)
            changed = 1;
        }
    }
    return changed;
}

// arg wejscia: const char *line (tekst)
// arg wyjscia: znak separatora (';' lub ',')
// logika/funkcja: Wykrywa separator uzyty w pliku.
static char detect_separator(const char *line) {
    // Jesli nie ma srednika, a jest przecinek, uzywamy przecinka jako separatora
    return (strchr(line, ';') == NULL && strchr(line, ',') != NULL) ? ',' : ';';
}

// arg wejscia: const char *filename (sciezka)
// arg wyjscia: 0 (ok), -1 (blad)
// logika/funkcja: Wstepna walidacja znakow i formatu przed odczytem.
int validate_and_process(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1; // Blad jesli plik nie istnieje

    char line[256];
    int line_num = 0, critical_errors = 0, total_fixes = 0;

    printf("\nUruchamiam przetwarzanie...\n");
    printf("------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), f)) {
        line_num++;
        int has_data = 0;
        // Sprawdzanie czy linia zawiera jakiekolwiek znaki drukowalne
        for(int i=0; line[i]; i++) {
            if(!isspace((unsigned char)line[i])) { has_data = 1; break; }
        }
        if(!has_data) continue; // Pominiecie linii pustych lub ze spacjami

        char sep = detect_separator(line);
        int fixed = sanitize(line); // Proba naprawy przecinkow
        if (fixed) total_fixes++;

        char name[50] = {0}, u_str[64] = {0}, v_str[64] = {0}, w_str[64] = {0};
        char format_str[128];
        int res;
        
        // Dynamiczne tworzenie formatu dla sscanf w zaleznosci od separatora
        if (line[0] == sep) { // Przypadek: krawedz bez nazwy (zaczyna sie od ;)
            sprintf(format_str, "%c%%%d[^%c]%c%%%d[^%c]%c%%%d[^\n\r]", sep, 63, sep, sep, 63, sep, sep, 63);
            res = sscanf(line, format_str, u_str, v_str, w_str);
            res++; // Dodajemy 1, aby symulowac odczyt 4 pol (nazwa jest pusta)
            strcpy(name, "(brak)");
        } else { // Przypadek: pelna linia (Nazwa;U;V;Waga)
            sprintf(format_str, "%%%d[^%c]%c%%%d[^%c]%c%%%d[^%c]%c%%%d[^\n\r]", 49, sep, sep, 63, sep, sep, 63, sep, sep, 63);
            res = sscanf(line, format_str, name, u_str, v_str, w_str);
        }

        int line_error = 0;
        if (res == 4) {
            // Weryfikacja czy pola U, V i Waga to faktycznie liczby (nie litery)
            for (int i = 0; u_str[i]; i++) if (!isdigit((unsigned char)u_str[i]) && !isspace((unsigned char)u_str[i])) line_error = 1;
            for (int i = 0; v_str[i]; i++) if (!isdigit((unsigned char)v_str[i]) && !isspace((unsigned char)v_str[i])) line_error = 1;
            int dots = 0;
            for (int i = 0; w_str[i]; i++) {
                if (w_str[i] == '.') dots++; // Liczymy kropki w wadze
                else if (!isdigit((unsigned char)w_str[i]) && !isspace((unsigned char)w_str[i])) line_error = 1;
            }
            if (dots > 1) line_error = 1; // Waga nie moze miec dwoch kropek
        } else line_error = 1; // Blad jesli sscanf nie wyodrebnil pol

        if (line_error) critical_errors++;
    } 
    fclose(f);
    // Jesli znaleziono choc jeden blad formatu, caly proces walidacji konczy sie porazka
    return (critical_errors > 0) ? -1 : 0; 
}

// arg wejscia: Graph *g (struktura)
// arg wyjscia: 1 (planar), 0 (nie)
// logika/funkcja: Sprawdza warunek Eulera E <= 3V - 6.
int is_potentially_planar(Graph *g) {
    if (g->node_count <= 2) return 1; // Male grafy sa zawsze planarne
    // Matematyczny warunek konieczny dla grafow planarnych
    return (g->edge_count <= (3 * g->node_count - 6));
}

// arg wejscia: Graph *g, const char *filename
// arg wyjscia: 0 (sukces), -1 (blad)
// logika/funkcja: Wlasciwy odczyt danych i budowanie grafu w pamieci.
int read_graph(Graph *g, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    // Rezerwacja pamieci na krawedzie i wierzcholki (limit 1000)
    g->edges = malloc(1000 * sizeof(Edge)); 
    g->nodes = calloc(1000, sizeof(Node)); 
    if (!g->edges || !g->nodes) { fclose(f); return -1; }

    int id_map[10001]; // Mapa do szybkiego sprawdzania ID wierzcholkow
    for(int i=0; i<10001; i++) id_map[i] = -1; // -1 oznacza, ze ID nie bylo jeszcze uzyte

    g->edge_count = 0;
    g->node_count = 0;
    char line[256];

    while (fgets(line, sizeof(line), f)) {
        int has_data = 0;
        // Sprawdzamy czy linia zawiera tekst lub liczby
        for(int i=0; line[i]; i++) if(isalnum((unsigned char)line[i])) { has_data = 1; break; }
        if(!has_data) continue;
        
        char sep = detect_separator(line);
        sanitize(line); // Finalne upewnienie sie co do kropek

        int u_id = -1, v_id = -1;
        double w = 0.0;
        char name[50] = {0};
        int res = 0;

        // Parsowanie linii w celu wyciagniecia ID wierzcholkow i wagi
        if (line[0] == sep) {
            char fstr[64];
            sprintf(fstr, "%c%%d%c%%d%c%%lf", sep, sep, sep);
            res = sscanf(line, fstr, &u_id, &v_id, &w);
        } else {
            char fstr[64];
            sprintf(fstr, "%%49[^%c]%c%%d%c%%d%c%%lf", sep, sep, sep, sep);
            res = sscanf(line, fstr, name, &u_id, &v_id, &w);
        }

        if (res >= 2 && u_id != -1 && v_id != -1) {
            // Walidacja czy ID miesci sie w tablicy mapowania (0-9999)
            if (u_id < 0 || u_id >= 10000 || v_id < 0 || v_id >= 10000) continue;
            if (g->node_count >= 1000) continue; // Nie przekraczamy limitu 1000 wezlow

            // Jesli wierzcholek U pojawia sie pierwszy raz, dodajemy go do tablicy 'nodes'
            if (id_map[u_id] == -1) {
                id_map[u_id] = g->node_count;
                g->nodes[g->node_count].id = u_id;
                g->node_count++;
            }
            // Jesli wierzcholek V pojawia sie pierwszy raz, robimy to samo
            if (id_map[v_id] == -1 && g->node_count < 1000) {
                id_map[v_id] = g->node_count;
                g->nodes[g->node_count].id = v_id;
                g->node_count++;
            }

            // Pobieramy wewnetrzne indeksy wierzcholkow
            int u_idx = id_map[u_id], v_idx = id_map[v_id];
            int is_duplicate = 0;
            // Sprawdzamy czy ta krawedz juz istnieje (zabezpieczenie przed duplikatami)
            for (int k = 0; k < g->edge_count; k++) {
                if ((g->edges[k].u_idx == u_idx && g->edges[k].v_idx == v_idx) ||
                    (g->edges[k].u_idx == v_idx && g->edges[k].v_idx == u_idx)) {
                    is_duplicate = 1;
                    break;
                }
            }

            // Jesli krawedz jest nowa, zapisujemy ja w strukturze
            if (!is_duplicate && g->edge_count < 1000) {
                g->edges[g->edge_count].u_idx = u_idx;
                g->edges[g->edge_count].v_idx = v_idx;
                g->edges[g->edge_count].weight = w;
                g->edge_count++;
            }
        }
    }
    fclose(f);
    return 0;
}

// arg wejscia: Graph *g, const char *filename, const char *format
// arg wyjscia: 0 (ok), -1 (blad)
// logika/funkcja: Zapisuje wspolrzedne do pliku tekstowego lub binarnego.
int save_graph(Graph *g, const char *filename, const char *format) {
    // Wybor trybu zapisu: wb (binary write) lub w (text write)
    FILE *f = fopen(filename, (strcmp(format, "bin") == 0) ? "wb" : "w");
    if (!f) return -1;

    if (strcmp(format, "bin") == 0) {
        // Zapis binarny: najpierw liczba wezlow, potem cala tablica struktur
        fwrite(&(g->node_count), sizeof(int), 1, f);
        fwrite(g->nodes, sizeof(Node), g->node_count, f);
    } else {
        // Zapis tekstowy: kazda linia to ID oraz wspolrzedne X i Y
        for (int i = 0; i < g->node_count; i++) {
            fprintf(f, "%d %.2f %.2f\n", g->nodes[i].id, g->nodes[i].x, g->nodes[i].y);
        }
    }
    fclose(f);
    return 0;
}

// arg wejscia: Graph *g
// arg wyjscia: brak (void)
// logika/funkcja: Zwalnia pamiec po wezlach i krawedziach.
void free_graph(Graph *g) {
    if(g->nodes) free(g->nodes); // Jesli tablica wezlow istnieje, zwolnij ja
    if(g->edges) free(g->edges); // Jesli tablica krawedzi istnieje, zwolnij ja
    g->nodes = NULL; g->edges = NULL; // Zapobieganie uzyciu "wiszących" wskaznikow
}