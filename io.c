#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"
#include "io.h"

/**

* Funkcja sprawdzająca spójność grafu za pomocą algorytmu przeszukiwania (DFS).

*/

int is_graph_connected(Graph *g)
{

    if (g->node_count <= 1)
        return 1;
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

    stack[++top] = 0;

    visited[0] = 1;

    int visited_count = 1;

    while (top >= 0)
    {

        int u = stack[top--];

        for (int i = 0; i < g->edge_count; i++)
        {

            int v = -1;

            if (g->edges[i].u_idx == u)
                v = g->edges[i].v_idx;

            else if (g->edges[i].v_idx == u)
                v = g->edges[i].u_idx;

            if (v != -1 && !visited[v])
            {

                visited[v] = 1;

                stack[++top] = v;

                visited_count++;
            }
        }
    }

    free(visited);

    free(stack);

    return (visited_count == g->node_count);
}

int sanitize(char *s)
{

    int changed = 0;

    for (int i = 0; s[i]; i++)
    {

        if (s[i] == ',')
        {

            s[i] = '.';

            changed = 1;
        }
    }

    return changed;
}

static char detect_separator(const char *line)
{

    return (strchr(line, ';') == NULL && strchr(line, ',') != NULL) ? ',' : ';';
}

/**

* Raportuje stan pliku przed wczytaniem.

* Uwzględnia teraz krawędzie bez nazw.

*/

int validate_and_process(const char *filename)
{

    FILE *f = fopen(filename, "r");

    if (!f) return -1;

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

        // POPRAWKA: Sprawdzanie czy linia jest pusta (same białe znaki)

        int has_data = 0;

        for (int i = 0; line[i]; i++)
        {

            if (!isspace((unsigned char)line[i]))
            {
                has_data = 1;
                break;
            }
        }

        if (!has_data)
        {

            printf("%-7d | [INFO] Pominięto pustą linię.\n", line_num);

            continue;
        }

        char sep = detect_separator(line);

        int fixed = sanitize(line); // zamiana ',' na '.'

        if (fixed)
            total_fixes++;

        char name[50] = {0}, u_str[64] = {0}, v_str[64] = {0}, w_str[64] = {0};

        char format_str[128];

        int res;

        // Każda krawędź MUSI mieć nazwę na początku.
        if (line[0] == sep)
        {
            res = 0; // Wymusi wejście w line_error (brak nazwy)
        }
        else
        {
            // OGRANICZENIE DO 10 ZNAKÓW W SSCANF (%10[^%c])
            sprintf(format_str, "%%%d[^%c]%c%%%d[^%c]%c%%%d[^%c]%c%%%d[^\n\r]", 10, sep, sep, 63, sep, sep, 63, sep, sep, 63);
            res = sscanf(line, format_str, name, u_str, v_str, w_str);
        }

        int line_error = 0;

        if (res == 4)
        {
            // WALIDACJA NAZWY (BRAK ZNAKÓW SPECJALNYCH)
            // DOZWOLONE TYLKO LITERY I CYFRY
            // Cel: Max 10 znaków (już w sscanf), brak spacji, brak wielkich liter, brak znaków specjalnych
            if (strcmp(name, "(brak)") != 0) {
                for (int i = 0; name[i]; i++) {
                    // Sprawdzamy czy znak to mała litera LUB cyfra
                    // Jeśli nie jest ani jednym, ani drugim -> błąd (wyłapie spacje, wielkie litery i symbole)
                    if (!islower((unsigned char)name[i]) && !isdigit((unsigned char)name[i])) {
                        line_error = 1;
                    }
                }
            }

            // Walidacja cyfr i formatu wagi

            for (int i = 0; u_str[i]; i++)
                if (!isdigit((unsigned char)u_str[i]) && !isspace((unsigned char)u_str[i]))
                    line_error = 1;

            for (int i = 0; v_str[i]; i++)
                if (!isdigit((unsigned char)v_str[i]) && !isspace((unsigned char)v_str[i]))
                    line_error = 1;

            int dots = 0;

            for (int i = 0; w_str[i]; i++)
            {

                if (w_str[i] == '.')
                    dots++;

                else if (!isdigit((unsigned char)w_str[i]) && !isspace((unsigned char)w_str[i]) && !(i == 0 && w_str[i] == '-'))
                    line_error = 1;
            }

            if (dots > 1)
                line_error = 1;
        }
        else
        {

            line_error = 1;
        }

        if (line_error)
        {

            printf("%-8d | %-45s | %-15s\n", line_num, "[ !!! BŁĄD KRYTYCZNY - ZŁY FORMAT !!! ]", "BŁĄD");

            critical_errors++;
        }
        else
        {
            // Konwersja na liczby, aby sprawdzić pętlę własną
            int u_val = atoi(u_str); // Zamieniamy tekst na liczbę
            int v_val = atoi(v_str);
            char data_buffer[256];
            
            // Przygotowujemy czytelny opis krawędzi
            snprintf(data_buffer, sizeof(data_buffer), "Krawedz %s: %d -> %d (w: %.2f)", name, u_val, v_val, atof(w_str));

            // --- WERYFIKACJA CZY ID MIEŚCI SIĘ W ZAKRESIE 1-1000 ---
            if (u_val < 1 || u_val > 1000 || v_val < 1 || v_val > 1000) {
                printf("%-8d | %-45s | %-15s\n", line_num, data_buffer, "BŁĄD KRYTYCZNY");
                printf("         | [STOP] ID WIERZCHOŁKA POZA DOPUSZCZALNYM ZAKRESEM [1, 1000]!\n");
                critical_errors++;
            } 
            else if (u_val == v_val) {
                printf("%-8d | %-45s | %-15s\n", line_num, data_buffer, "BŁĄD KRYTYCZNY");
                printf("         | [STOP] Wykryto pętlę własną (%d -> %d).\n", u_val, v_val);
                critical_errors++; 
            } else {
                const char *status = fixed ? "[Poprawiono ,]" : "[OK]";
                printf("%-8d | %-45s | %-15s\n", line_num, data_buffer, status);
            }
        }
    }

    printf("==============================================================================\n");

    fclose(f);

    if (critical_errors > 0)
    {

        printf("\nSTOP! Znaleziono %d bledow krytycznych w strukturze pliku.\n", critical_errors);

        printf(" -> INFO: Poprawny format to: Nazwa;U;V;Waga\n");

        printf(" -> Separator danych to ŚREDNIK (;), a separator dziesiętny to KROPKA (.)\n");

        printf(" -> ID wierzchołków muszą mieścić się w przedziale od 1 do 1000.\n");

        return -1;
    }

    if (total_fixes > 0)
        printf("INFO: Dokonano %d autokorekt separatora dziesiętnego.\n", total_fixes);

    return 0;
}

int is_potentially_planar(Graph *g){

    if (g->node_count <= 2) return 1;

    return (g->edge_count <= (3 * g->node_count - 6));
}


/**

* Czyta graf z pliku, obsługuje brak nazw i eliminuje duplikaty.

*/

int read_graph(Graph *g, const char *filename)
{

    FILE *f = fopen(filename, "r");

    if (!f)
        return -1;

    g->edges = malloc(1000 * sizeof(Edge));

    g->nodes = calloc(1000, sizeof(Node));

    // Sprawdzenie powodzenia malloc/calloc

    if (!g->edges || !g->nodes)
    {
        fclose(f);
        return -1;
    }

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

        // 1. Całkowite ignorowanie linii, które nie mają cyfr/liter

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

            // Komunikat o napotkaniu pustej linii w read_graph

            printf("Linia %d: [INFO] Napotkano pustą linię - pomijam.\n", line_num);

            continue;
        }

        char sep = detect_separator(line);

        sanitize(line);

        // 2. KLUCZOWE: Resetujemy zmienne, żeby nie użyć danych z poprzedniej linii!

        int u_id = -1, v_id = -1;

        double w = 0.0;

        char name[50] = {0};

        int res = 0;

        // Czytamy bezpośrednio do zmiennych docelowych.

        if (line[0] == sep)
        {

            char fstr[64];

            sprintf(fstr, "%c%%d%c%%d%c%%lf", sep, sep, sep);

            res = sscanf(line, fstr, &u_id, &v_id, &w);
        }
        else
        {

            char fstr[64];

            sprintf(fstr, "%%49[^%c]%c%%d%c%%d%c%%lf", sep, sep, sep, sep);

            res = sscanf(line, fstr, name, &u_id, &v_id, &w);
        }

        // 3. Sprawdzamy, czy faktycznie wczytaliśmy U i V

        // --- FILTROWANIE ID SPOZA ZAKRESU [1, 1000] PODCZAS WCZYTYWANIA ---
        if (res >= 2 && u_id >= 1 && u_id <= 1000 && v_id >= 1 && v_id <= 1000 && u_id != v_id)
        {

            if (g->node_count >= 1000)
                continue;

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

            int u_idx = id_map[u_id], v_idx = id_map[v_id];

            if (u_idx == -1 || v_idx == -1)
                continue;

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

    // Komunikat o uruchomieniu algorytmu z wczytanymi danymi

    printf("\nWczytywanie zakończone sukcesem. Dane są poprawne technicznie.\n");

    printf("Statystyki: %d węzłów, %d krawędzi.\n", g->node_count, g->edge_count);

    printf("------------------------------------------------------------------------------\n");

    return 0;
}

int save_graph(Graph *g, const char *filename, const char *format)
{

    FILE *f = fopen(filename, (strcmp(format, "bin") == 0) ? "wb" : "w");

    if (!f)
        return -1;

    if (strcmp(format, "bin") == 0)
    {

        fwrite(&(g->node_count), sizeof(int), 1, f);

        fwrite(g->nodes, sizeof(Node), g->node_count, f);
    }
    else
    {

        for (int i = 0; i < g->node_count; i++)
        {

            fprintf(f, "%d %.2f %.2f\n", g->nodes[i].id, g->nodes[i].x, g->nodes[i].y);
        }
    }

    fclose(f);

    return 0;
}

void free_graph(Graph *g)
{

    if (g->nodes)
        free(g->nodes);

    if (g->edges)
        free(g->edges);

    g->nodes = NULL;
    g->edges = NULL;
}
