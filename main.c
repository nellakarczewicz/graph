#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>  // Do optarg
#include "graph.h"
#include "layout_tutte.h"
#include "fruchterman.h"
#include "io.h"                 

void print_usage(char *prog_name) {
    printf("Użycie: %s -i <wejście.txt> -o <wyjście> -a <tutte|fruchterman> [-f <txt|bin>]\n", prog_name);
    printf("Opcje:\n");
    printf("  -i <plik>    Ścieżka do pliku z listą krawędzi (wymagane)\n");
    printf("  -o <plik>    Ścieżka do pliku wynikowego (wymagane)\n");
    printf("  -a <alg>     Wybór algorytmu: tutte lub fruchterman\n");
    printf("  -f <format>  Format zapisu: txt (domyślnie) lub bin\n");
}

int main(int argc, char *argv[]) {
    int opt;
    char *input_path = NULL;
    char *output_path = NULL;
    char *algorithm = "tutte"; // Domyślny algorytm
    char *format = "txt";      // Domyślny format

    // 1. Obsługa argumentów CLI (Twoja działka: getopt)
    while ((opt = getopt(argc, argv, "i:o:a:f:h")) != -1) {
        switch (opt) {
            case 'i': input_path = optarg; break;
            case 'o': output_path = optarg; break;
            case 'a': algorithm = optarg; break;
            case 'f': format = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default: 
                fprintf(stderr, "Błąd: Nieznana opcja.\n");
                print_usage(argv[0]); 
                return 1;
        }
    }

    // Walidacja argumentów (Obsługa błędów)
    if (input_path == NULL || output_path == NULL) {
        fprintf(stderr, "Błąd: Brak podanego pliku wejściowego lub wyjściowego!\n");
        print_usage(argv[0]);
        return 1;
    }

    // 2. Inicjalizacja struktury grafu
    Graph g = {
        .nodes = NULL, 
        .edges = NULL, 
        .node_count = 0, 
        .edge_count = 0, 
        .width = 1000.0,  // Przykładowy rozmiar obszaru roboczego
        .height = 1000.0
    };

    // 3. Sterowanie przepływem programu

    // Etap walidacji (pokazuje tabelę poprawek i czeka na ENTER)
    int status = validate_and_process(input_path);

    if (status == -1) {
        // Tu program wpada, gdy są BŁĘDY KRYTYCZNE - nie ma opcji kliknięcia Enter
        fprintf(stderr, "\n[PRZERWANO] Plik zawiera bledy uniemozliwiajace procesowanie.\n");
        return 10; 
    }

    // KROK 2: Wczytywanie danych do pamięci (wykonywane po ENTER w kroku 1)
    printf("Wczytywanie grafu z: %s...\n", input_path);
    if (read_graph(&g, input_path) != 0) { 
        fprintf(stderr, "Nieoczekiwany blad przy wczytywaniu danych do pamieci.\n");
        return 2;
    }

    // Dodatkowe zabezpieczenie po wczytaniu
    if (g.edge_count == 0) {
        fprintf(stderr, "\n[BŁĄD] Plik nie zawiera żadnych poprawnych krawędzi!\n");
        free_graph(&g);
        return 5;
    }

   // KROK B: Wybór i uruchomienie algorytmu
    if (strcmp(algorithm, "tutte") == 0) {
        if (g.node_count < 3) {
            fprintf(stderr, "Blad: Algorytm Tutte wymaga co najmniej 3 wierzcholkow!\n");
            free_graph(&g);
            return 6;
        }
        printf("Uruchamianie metody barycentrycznej (Tutte)...\n");
        compute_tutte_layout(&g); 
    } 
    else if (strcmp(algorithm, "fruchterman") == 0) {
        printf("Inicjalizacja losowych pozycji i uruchamianie modelu siłowego...\n");
        init_random_positions(&g); // Musisz to dodać przed algorytmem!
        run_fruchterman_reingold(&g, 100, 10.0); // 100 iteracji, temp 10.0
    } 
    else {
        fprintf(stderr, "Błąd: Nieznany algorytm '%s'.\n", algorithm);
        return 3;
    }

    // KROK C: Zapis wyników (używając io.c)
    printf("Zapisywanie wyników w formacie %s do: %s...\n", format, output_path);
     if (save_graph(&g, output_path, format) != 0) {
         fprintf(stderr, "Błąd: Nie udało się zapisać wyników.\n");
         free_graph(&g);
         return 4;
     }

    printf("Sukces! Program zakończył działanie.\n");

    // Zwolnienie pamięci (wspólna odpowiedzialność)
     free_graph(&g); 

    return 0;
}