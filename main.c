#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "graph.h"
#include "layout_tutte.h"
#include "fruchterman.h"
#include "io.h"                 

/**
 * arg wejscia: prog_name (nazwa pliku wykonywalnego)
 * arg wyjscia: brak (wypisuje tekst na standardowe wyjscie)
 * logika/funkcja: Wyswietla instrukcje obslugi programu i dostepne flagi CLI.
 */
void print_usage(char *prog_name) {
    printf("Użycie: %s -i <wejście.txt> -o <wyjście> -a <tutte|fruchterman> [-f <txt|bin>]\n", prog_name);
}

/**
 * arg wejscia: argc (liczba argumentow), argv (tablica argumentow)
 * arg wyjscia: 0 (sukces), kody bledow 1-10 (porazka)
 * logika/funkcja: Glowny kontroler programu; zarzadza przeplywem danych miedzy walidacja, odczytem, obliczeniami a zapisem.
 */
int main(int argc, char *argv[]) {
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych dla algorytmu Fruchtermanna
    int opt;
    int success = 0;
    char *input_path = NULL;
    char *output_path = NULL;
    char *algorithm = "tutte"; // Domyslny algorytm
    char *format = "txt";      // Domyslny format zapisu

    // --- PARSOWANIE ARGUMENTOW ---
    // Logika: Pobieranie opcji uzytkownika za pomoca funkcji getopt
    while ((opt = getopt(argc, argv, "i:o:a:f:h")) != -1) {
        switch (opt) {
            case 'i': input_path = optarg; break;
            case 'o': output_path = optarg; break;
            case 'a': algorithm = optarg; break;
            case 'f': format = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default: return 1;
        }
    }

    // Walidacja obecnosci sciezek plikow
    if (input_path == NULL || output_path == NULL) {
        fprintf(stderr, "Błąd: Brak podanego pliku wejściowego lub wyjściowego!\n");
        return 1;
    }

    Graph g = { .nodes = NULL, .edges = NULL, .node_count = 0, .edge_count = 0, .width = 1000.0, .height = 1000.0 };

    // --- FAZA 1: WALIDACJA I ODCZYT ---
    // Logika: Najpierw sprawdzamy poprawnosc struktury pliku (Kod 10), potem wczytujemy dane (Kod 2)
    if (validate_and_process(input_path) == -1) return 10;

    if (read_graph(&g, input_path) != 0) return 2;

    if (g.edge_count == 0) {
        fprintf(stderr, "Błąd: Plik pusty.\n");
        free_graph(&g);
        return 5;
    }

    // --- FAZA 2: WERYFIKACJA SPOJNOSCI ---
    // Logika: Sprawdzenie algorytmem DFS, czy graf nie jest rozbity na czesci (Kod 9)
    if (!is_graph_connected(&g)) {
        printf("\n============================================================\n");
        printf("   BŁĄD KRYTYCZNY: GRAF NIESPÓJNY    \n");
        printf("============================================================\n");
        printf("Dane z pliku wejściowego [%s] są błędne:\n", input_path);
        printf("-> Graf składa się z kilku niepołączonych ze sobą części.\n");
        printf("-> Algorytm wymaga grafu spójnego, aby poprawnie wyznaczyć pozycje.\n");
        printf("============================================================\n");
        
        free_graph(&g);
        return 9; 
    }

    success = 0; 

    // --- FAZA 3: OBLICZENIA (LOGIKA WYBORU ALGORYTMU) ---
    if (strcmp(algorithm, "tutte") == 0) {
        // Logika Tutte: Wymaga min. 3 wezlow do stworzenia ramy (Kod 6)
        if (g.node_count < 3) {
            fprintf(stderr, "Błąd: Tutte wymaga >= 3 wierzchołków!\n");
            free_graph(&g);
            return 6;
        }
        compute_tutte_layout(&g); 
        if (is_tutte_layout_planar(&g)) {
            printf("Sukces! Graf jest planarny (Tutte).\n");
            success = 1;
        } else {
            printf("[Ostrzeżenie] Graf Tutte'a ma przecięcia.\n");
            success = 1; 
        }
    } 
    else if (strcmp(algorithm, "fruchterman") == 0) {
        // Logika Fruchterman: Sprawdzenie potencjalnej planarnosci (Kod 7) i spojnosci
        if (!is_potentially_planar(&g)) {
            free_graph(&g);
            return 7;
        }
        if (!is_graph_connected(&g)) {
            free_graph(&g);
            return 9;
        }

        // Mechanizm wielokrotnych prob: Resetowanie pozycji i ponowna symulacja fizyczna
        int attempts = 0;
        while (!success && attempts < 3000) {
            attempts++;
            init_random_positions(&g);
            run_fruchterman(&g, 1000, 30.0);
            if (is_layout_planar(&g)) success = 1;
        }
    }

    // --- FAZA 4: ZAPIS I CZYSZCZENIE ---
    // Logika: Jesli osiagnieto uklad bez przeciec lub Tutte skonczyl prace, zapisujemy dane (Kod 8 przy porazce)
    if (success) {
        save_graph(&g, output_path, format);
    }

    free_graph(&g); // Zwolnienie pamieci dynamicznej
    return success ? 0 : 8;
}