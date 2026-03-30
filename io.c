#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"
#include "io.h"

/**

* Funkcja sprawdzająca spójność grafu za pomocą algorytmu przeszukiwania (DFS).
* @param g wskaźnik na strukturę Graph zawierającą listę wierzchołków i krawędzi
* @return int:
 *         - 1 jeśli graf jest spójny,
 *         - 0 jeśli graf jest niespójny,
 *         - -1 jeśli wystąpił błąd alokacji pamięci.
*/

int is_graph_connected(Graph *g){
    // Graf z 0 lub 1 wierzchołkiem jest zawsze spójny
    if (g->node_count <= 1)
        return 1;

    // Tablica odwiedzin i stos do DFS
    int *visited = calloc(g->node_count, sizeof(int));
    int *stack = malloc(g->node_count * sizeof(int));

    // Bezpieczeństwo pamięci - sprawdzenie alokacji w DFS
    if (!visited || !stack)
    {
        free(visited);
        free(stack);
        return -1;
    }

    int top = -1;

    // Zaczynamy DFS od wierzchołka 0
    stack[++top] = 0;
    visited[0] = 1;

    int visited_count = 1;

    // Główna pętla DFS
    while (top >= 0)
    {
        // Pobieramy wierzchołek ze stosu
        int u = stack[top--];

        // Przeglądamy wszystkie krawędzie grafu
        for (int i = 0; i < g->edge_count; i++)
        {

            int v = -1;

            // Jeśli krawędź wychodzi z u → bierzemy drugi koniec
            if (g->edges[i].u_idx == u)
                v = g->edges[i].v_idx;

            // Jeśli krawędź wchodzi do u → bierzemy drugi koniec
            else if (g->edges[i].v_idx == u)
                v = g->edges[i].u_idx;

            // Jeśli znaleziono sąsiada i nie był odwiedzony
            if (v != -1 && !visited[v])
            {
                visited[v] = 1; // Oznacz jako odwiedzony
                stack[++top] = v; // Dodaj na stos
                visited_count++; // Zwiększ licznik odwiedzonych
            }
        }
    }

    // Sprzątanie pamięci
    free(visited);
    free(stack);

    // Graf spójny, jeśli odwiedziliśmy wszystkie wierzchołki
    return (visited_count == g->node_count);
}

/*
* Normalizuje zapis liczby w łańcuchu znaków, zamieniając przecinki na kropki.
* @param s wskaźnik na modyfikowalny łańcuch znaków
* @return int:
 *         - 1 jeśli dokonano przynajmniej jednej zamiany,
 *         - 0 jeśli łańcuch pozostał bez zmian.
*/
int sanitize(char *s)
{
    int changed = 0;

    // Przechodzimy po całym stringu
    for (int i = 0; s[i]; i++)
    {
        // Jeśli znak to przecinek → zamieniamy na kropkę
        if (s[i] == ',')
        {
            s[i] = '.';
            changed = 1; // Oznaczamy, że dokonano zmiany
        }
    }

    return changed;
}

/*
 * Wykrywa separator pól w pojedynczej linii tekstu.
 * @param line wskaźnik na łańcuch znaków reprezentujący jedną linię danych
 *
 * @return char:
 *         - ',' jeśli wykryto format z przecinkami,
 *         - ';' w pozostałych przypadkach.
 */

static char detect_separator(const char *line)
{
    // Jeśli linia NIE zawiera ';' i zawiera ',' → używamy przecinka
    // W przeciwnym przypadku (np. są średniki) → separator to ';'
    return (strchr(line, ';') == NULL && strchr(line, ',') != NULL) ? ',' : ';';
}
/**
 * Funkcja analizuje plik wejściowy z danymi krawędzi.
 * Sprawdza poprawność formatu, separatorów, nazw, ID wierzchołków i wag.
 * Wykonuje automatyczne poprawki separatora dziesiętnego (',' → '.').
 * @param filename - plik wejściowy

* Zwraca:
 *   0  → jeśli plik jest poprawny (lub poprawialny)
 *  -1  → jeśli wykryto błędy krytyczne

*/

int validate_and_process(const char *filename)
{

    FILE *f = fopen(filename, "r");
    if (!f) return -1; // Nie udało się otworzyć pliku

    char line[256];
    int line_num = 0, critical_errors = 0, total_fixes = 0;

    printf("\nUruchamiam przetwarzanie...\n");
    printf("\n==============================================================================\n");
    printf(" RAPORT ANALIZY DANYCH WEJSCIOWYCH \n");
    printf("==============================================================================\n");
    printf("%-8s | %-45s | %-15s\n", "Linia", "Zinterpretowane dane", "Status");
    printf("------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), f))
    {
        line_num++;

        // --- SPRAWDZENIE CZY LINIA NIE JEST PUSTA (same spacje/taby) ---
        int has_data = 0;
        for (int i = 0; line[i]; i++)
        {
            if (!isspace((unsigned char)line[i])) {
                has_data = 1;
                break;
            }
        }

        if (!has_data)
        {
            // Pomijamy puste linie, ale raportujemy
            printf("%-7d | [INFO] Pominięto pustą linię.\n", line_num);
            continue;
        }

        // --- WYKRYWANIE SEPARATORA (',' lub ';') ---
        char sep = detect_separator(line);

        // --- AUTOMATYCZNA POPRAWKA ZAPISU LICZB (',' → '.') ---
        int fixed = sanitize(line);
        if (fixed)
            total_fixes++;

        // Bufory na odczytane pola
        char name[50] = {0}, u_str[64] = {0}, v_str[64] = {0}, w_str[64] = {0};
        char format_str[128];
        int res;

        // --- WALIDACJA: nazwa nie może zaczynać się od separatora ---
        if (line[0] == sep)
        {
            // Brak nazwy → wymusza błąd
            res = 0;
        }
        else
        {
            // Budujemy format sscanf z ograniczeniem długości pól
            sprintf(format_str,
                    "%%%d[^%c]%c%%%d[^%c]%c%%%d[^%c]%c%%%d[^\n\r]",
                    10, sep, sep,
                    63, sep, sep,
                    63, sep, sep,
                    63);

            // Próba sparsowania 4 pól
            res = sscanf(line, format_str, name, u_str, v_str, w_str);
        }

        int line_error = 0;

        // --- JEŚLI UDAŁO SIĘ ODCZYTAĆ 4 POLA ---
        if (res == 4)
        {
            // --- WALIDACJA NAZWY ---
            // Dozwolone: małe litery + cyfry, max 10 znaków (ograniczone w sscanf)
            if (strcmp(name, "(brak)") != 0) {
                for (int i = 0; name[i]; i++) {
                    if (!islower((unsigned char)name[i]) &&
                        !isdigit((unsigned char)name[i])) {
                        line_error = 1; // Niedozwolony znak
                    }
                }
            }

            // --- WALIDACJA POLA U ---
            for (int i = 0; u_str[i]; i++)
                if (!isdigit((unsigned char)u_str[i]) &&
                    !isspace((unsigned char)u_str[i]))
                    line_error = 1;

            // --- WALIDACJA POLA V ---
            for (int i = 0; v_str[i]; i++)
                if (!isdigit((unsigned char)v_str[i]) &&
                    !isspace((unsigned char)v_str[i]))
                    line_error = 1;

            // --- WALIDACJA POLA W (waga) ---
            int dots = 0;
            for (int i = 0; w_str[i]; i++)
            {
                if (w_str[i] == '.')
                    dots++;
                else if (!isdigit((unsigned char)w_str[i]) &&
                         !isspace((unsigned char)w_str[i]) &&
                         !(i == 0 && w_str[i] == '-'))
                    line_error = 1;
            }

            // Waga nie może mieć więcej niż jednej kropki
            if (dots > 1)
                line_error = 1;
        }
        else
        {
            // Nie udało się sparsować 4 pól → błąd formatu
            line_error = 1;
        }

        // --- RAPORTOWANIE BŁĘDÓW ---
        if (line_error)
        {
            printf("%-8d | %-45s | %-15s\n",
                   line_num,
                   "[ !!! BŁĄD KRYTYCZNY - ZŁY FORMAT !!! ]",
                   "BŁĄD");

            critical_errors++;
        }
        else
        {
            // --- KONWERSJA NA LICZBY ---
            int u_val = atoi(u_str);
            int v_val = atoi(v_str);
            double w_val = atof(w_str);

            char data_buffer[256];
            snprintf(data_buffer, sizeof(data_buffer),
                     "Krawedz %s: %d -> %d (w: %.2f)",
                     name, u_val, v_val, w_val);

            // --- WALIDACJA WAGI ---
            int valid_weight_format = 0;
            char *dot_ptr = strchr(w_str, '.');

            // Waga musi mieć dokładnie jedną cyfrę po kropce
            if (dot_ptr != NULL && strlen(dot_ptr) == 2)
                valid_weight_format = 1;

            if (!valid_weight_format || w_val < 0.1 || w_val > 100.0)
            {
                printf("%-8d | %-45s | %-15s\n",
                       line_num, data_buffer, "BŁĄD KRYTYCZNY");
                printf("         | [STOP] WAGA MUSI BYĆ W FORMACIE X.X I W ZAKRESIE [0.1, 100.0]\n");
                critical_errors++;
            }

            // --- WALIDACJA ZAKRESU ID ---
            else if (u_val < 1 || u_val > 1000 ||
                     v_val < 1 || v_val > 1000)
            {
                printf("%-8d | %-45s | %-15s\n",
                       line_num, data_buffer, "BŁĄD KRYTYCZNY");
                printf("         | [STOP] ID WIERZCHOŁKA POZA ZAKRESEM [1, 1000]\n");
                critical_errors++;
            }

            // --- WYKRYWANIE PĘTLI WŁASNEJ ---
            else if (u_val == v_val)
            {
                printf("%-8d | %-45s | %-15s\n",
                       line_num, data_buffer, "BŁĄD KRYTYCZNY");
                printf("         | [STOP] Wykryto pętlę własną (%d -> %d)\n", u_val, v_val);
                critical_errors++;
            }

            // --- LINIA POPRAWNA ---
            else
            {
                const char *status = fixed ? "[Poprawiono ,]" : "[OK]";
                printf("%-8d | %-45s | %-15s\n",
                       line_num, data_buffer, status);
            }
        }
    }

    printf("==============================================================================\n");
    fclose(f);

    // --- PODSUMOWANIE BŁĘDÓW ---
    if (critical_errors > 0)
    {
        printf("\nSTOP! Znaleziono %d bledow krytycznych w strukturze pliku.\n", critical_errors);
        printf(" -> INFO: Poprawny format to: Nazwa;U;V;Waga\n");
        printf(" -> Separator danych to ŚREDNIK (;), a separator dziesiętny to KROPKA (.)\n");
        printf(" -> ID wierzchołków muszą mieścić się w przedziale od 1 do 1000.\n");
        printf(" -> Waga musi być dodatnia, z jednym miejscem po kropce.\n");
        return -1;
    }

    // Informacja o automatycznych poprawkach
    if (total_fixes > 0)
        printf("INFO: Dokonano %d autokorekt separatora dziesiętnego.\n", total_fixes);

    return 0;
}

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

int read_graph(Graph *g, const char *filename)
{

    FILE *f = fopen(filename, "r");
    if (!f)
        return -1; // Nie udało się otworzyć pliku

    // Rezerwujemy miejsce na maksymalnie 1000 węzłów i 1000 krawędzi
    g->edges = malloc(1000 * sizeof(Edge));
    g->nodes = calloc(1000, sizeof(Node));

    // Sprawdzamy, czy alokacja pamięci się powiodła
    if (!g->edges || !g->nodes)
    {
        fclose(f);
        return -1;
    }

    // Mapa ID → indeks w tablicy nodes[]
    // -1 oznacza, że dany ID nie został jeszcze przypisany
    int id_map[1001];
    for (int i = 0; i < 1001; i++)
        id_map[i] = -1;

    g->edge_count = 0;
    g->node_count = 0;

    char line[256];
    int line_num = 0;

    while (fgets(line, sizeof(line), f))
    {
        line_num++;

        // --- 1. Ignorowanie linii bez liter/cyfr (np. puste, komentarze, whitespace) ---
        int has_data = 0;
        for (int i = 0; line[i]; i++)
        {
            if (isalnum((unsigned char)line[i]))
            {
                has_data = 1;
                break;
            }
        }

        if (!has_data)
        {
            printf("Linia %d: [INFO] Napotkano pustą linię - pomijam.\n", line_num);
            continue;
        }

        // Wykrycie separatora (',' lub ';')
        char sep = detect_separator(line);

        // Zamiana przecinków na kropki (jeśli występują)
        sanitize(line);

        // --- 2. Reset zmiennych, aby nie użyć danych z poprzedniej iteracji ---
        int u_id = -1, v_id = -1;
        double w = 0.0;
        char name[50] = {0};
        int res = 0;

        // --- 3. Parsowanie linii ---
        if (line[0] == sep)
        {
            // Brak nazwy → format: ;U;V;W
            char fstr[64];
            sprintf(fstr, "%c%%d%c%%d%c%%lf", sep, sep, sep);
            res = sscanf(line, fstr, &u_id, &v_id, &w);
        }
        else
        {
            // Format: Nazwa;U;V;W
            char fstr[64];
            sprintf(fstr, "%%49[^%c]%c%%d%c%%d%c%%lf", sep, sep, sep, sep);
            res = sscanf(line, fstr, name, &u_id, &v_id, &w);
        }

        // --- 4. Walidacja ID i filtracja błędnych danych ---
        // Sprawdzamy:
        //  - czy odczytano co najmniej U i V
        //  - czy ID są w zakresie [1, 1000]
        //  - czy nie ma pętli własnej
        if (res >= 2 &&
            u_id >= 1 && u_id <= 1000 &&
            v_id >= 1 && v_id <= 1000 &&
            u_id != v_id)
        {
            // --- 5. Dodawanie nowych węzłów do tablicy nodes[] ---
            if (g->node_count >= 1000)
                continue; // Osiągnięto limit

            // Jeśli ID nie było wcześniej widziane → dodajemy nowy węzeł
            if (id_map[u_id] == -1)
            {
                id_map[u_id] = g->node_count;
                g->nodes[g->node_count].id = u_id;
                g->node_count++;
            }

            if (id_map[v_id] == -1 && g->node_count < 1000)
            {
                id_map[v_id] = g->node_count;
                g->nodes[g->node_count].id = v_id;
                g->node_count++;
            }

            int u_idx = id_map[u_id];
            int v_idx = id_map[v_id];

            if (u_idx == -1 || v_idx == -1)
                continue;

            // --- 6. Sprawdzanie duplikatów krawędzi ---
            int is_duplicate = 0;
            for (int k = 0; k < g->edge_count; k++)
            {
                if ((g->edges[k].u_idx == u_idx && g->edges[k].v_idx == v_idx) ||
                    (g->edges[k].u_idx == v_idx && g->edges[k].v_idx == u_idx))
                {
                    is_duplicate = 1;
                    break;
                }
            }

            // --- 7. Dodanie krawędzi, jeśli nie jest duplikatem ---
            if (!is_duplicate && g->edge_count < 1000)
            {
                g->edges[g->edge_count].u_idx = u_idx;
                g->edges[g->edge_count].v_idx = v_idx;
                g->edges[g->edge_count].weight = w;
                g->edge_count++;
            }
        }
    }

    fclose(f);

    // Podsumowanie wczytanych danych
    printf("\nWczytywanie zakończone sukcesem. Dane są poprawne technicznie.\n");
    printf("Statystyki: %d węzłów, %d krawędzi.\n", g->node_count, g->edge_count);
    printf("------------------------------------------------------------------------------\n");

    return 0;
}

/*
* Zapisuje graf do pliku w formacie tekstowym lub binarnym.
 * @param g - wskaźnik na strukturę Graph
 * @param filename - plik wejściowy
 * @param format - format
 * Zwraca:
 *   0  → sukces
 *  -1  → błąd otwarcia pliku
*/

int save_graph(Graph *g, const char *filename, const char *format)
{
    // Wybór trybu zapisu: binarny ("wb") lub tekstowy ("w")
    FILE *f = fopen(filename, (strcmp(format, "bin") == 0) ? "wb" : "w");
    if (!f)
        return -1; // Nie udało się otworzyć pliku

    // --- ZAPIS BINARNY ---
    if (strcmp(format, "bin") == 0)
    {
        // Najpierw zapisujemy liczbę węzłów
        fwrite(&(g->node_count), sizeof(int), 1, f);

        // Następnie całą tablicę nodes[] w jednym bloku
        fwrite(g->nodes, sizeof(Node), g->node_count, f);
    }
    else
    {
        // --- ZAPIS TEKSTOWY ---
        // Każdy węzeł w formacie:
        // ID  X  Y
        for (int i = 0; i < g->node_count; i++)
        {
            // Zapis współrzędnych z dokładnością do dwóch miejsc po przecinku
            fprintf(f, "%d %.2f %.2f\n",
                    g->nodes[i].id,
                    g->nodes[i].x,
                    g->nodes[i].y);
        }
    }

    fclose(f);
    return 0;
}

/*
* Zwalnia pamięć zaalokowaną dla grafu.
* @param g - wskaźnik na strukturę Graph
* zwraca void
*/
void free_graph(Graph *g)
{

    // Zwalniamy tablicę węzłów, jeśli istnieje
    if (g->nodes)
        free(g->nodes);

    // Zwalniamy tablicę krawędzi, jeśli istnieje
    if (g->edges)
        free(g->edges);

    // Zerujemy wskaźniki, aby uniknąć wiszących referencji
    g->nodes = NULL;
    g->edges = NULL;
}
