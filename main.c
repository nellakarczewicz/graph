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
    // --- OBSŁUGA FLAG POMOCY ---
    if (argc >1 ) {
        char *arg = argv[1];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0 || 
            strcmp(arg, "-help") == 0 || strcmp(arg, "/h") == 0 || 
            strcmp(arg, "help") == 0) {
            
            printf("============================================================\n");
            printf("   INSTRUKCJA OBSLUGI PROGRAMU I ZASADY PISOWNI\n");
            printf("============================================================\n");
            printf("Uzycie: ./program -i <wejscie> -o <wyjscie> -a <tutte|fruchterman>\n\n");
            printf("ZASADY PISOWNI PLIKU WEJSCIOWEGO:\n");
            printf("1. Format linii: Nazwa;U;V;Waga (separator ';' lub ',')\n");
            printf("2. Nazwa:  max 10 znakow (a-z, 0-9), brak spacji i PL znakow.\n");
            printf("3. ID:     Liczby calkowite dodatnie (zakres 1-1000).\n");
            printf("4. Waga:   Liczba dziesietna (np. 1.50). Przecinek korygowany na kropke.\n");
            printf("5. Uwagi:  Linie zaczynajace sie od '#' oraz puste sa ignorowane.\n");
            printf("============================================================\n");
            return 0; // Wyjście z programu po wyświetleniu pomocy
        }
    }
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych dla algorytmu Fruchtermanna
    int opt;
    char *input_path = NULL;
    char *output_path = NULL;
    char *algorithm = "tutte"; // Domyslny algorytm
    char *format = "txt";      // Domyslny format zapisu

    // FLAGA SPRAWDZAJĄCA CZY UŻYTKOWNIK SAM WPISAŁ FORMAT
    int format_provided = 0;
    // FLAGA SPRAWDZAJĄCA CZY UŻYTKOWNIK SAM WYBRAŁ ALGORYTM
    int algo_provided = 0;

    // --- PARSOWANIE ARGUMENTOW ---
    // Logika: Pobieranie opcji uzytkownika za pomoca funkcji getopt
    while ((opt = getopt(argc, argv, "i:o:a:f:h")) != -1) {
        switch (opt) {
            case 'i': input_path = optarg; break;
            case 'o': output_path = optarg; break;
            case 'a': 
                algorithm = optarg;
                algo_provided = 1; // ZAZNACZAMY, ŻE UŻYTKOWNIK PODAŁ -a
                break;
            case 'f': 
                format = optarg;
                format_provided = 1; // ZAZNACZAMY, ŻE UŻYTKOWNIK PODAŁ -f
                break;
            case 'h': print_usage(argv[0]); return 0;
            default: return 1;
        }
    }

    // Walidacja obecnosci sciezek plikow
    if (input_path == NULL || output_path == NULL) {
        printf("\n=====================================================================\n");
        printf("   BŁĄD: BRAK PODANEGO PLIKU WEJŚCIOWEGO LUB WYJŚCIOWEGO!   \n");
        printf("======================================================================\n");
        printf("Należy podać parametry -i oraz -o, żeby plik został poprawnie wczytany.\n");
        printf("Przykład: ./program -i plik.txt -o wynik.txt\n");
        printf("Użyj flagi -h, aby zobaczyć pełną instrukcję.\n");
        printf("======================================================================\n");
        return 1;
    }

    // --- WERYFIKACJA NAZWY ALGORYTMU ---
    // SPRAWDZAMY CZY PODANY ALGORYTM JEST NA LIŚCIE OBSŁUGIWANYCH
    if (strcmp(algorithm, "tutte") != 0 && strcmp(algorithm, "fruchterman") != 0) {
        printf("\n============================================================\n");
        printf("   BŁĄD KRYTYCZNY: NIEZNANY ALGORYTM    \n");
        printf("============================================================\n");
        printf("Podana nazwa: [%s] jest nieprawidłowa.\n", algorithm);
        printf("Dostępne opcje to: 'tutte' lub 'fruchterman'.\n");
        printf("============================================================\n");
        return 4;
    }

    // --- WERYFIKACJA FORMATU PLIKU WYJŚCIOWEGO ---
    // LOGIKA: SPRAWDZAMY CZY FORMAT TO 'txt' LUB 'bin'
    if (strcmp(format, "txt") != 0 && strcmp(format, "bin") != 0) {
        printf("\n============================================================\n");
        printf("   BŁĄD KRYTYCZNY: NIEPRAWIDŁOWY FORMAT PLIKU    \n");
        printf("============================================================\n");
        printf("Podany format: [%s] nie jest obsługiwany.\n", format);
        printf("Dostępne opcje to: 'txt' (tekstowy) lub 'bin' (binarny).\n");
        printf("============================================================\n");
        return 11;
    }

    // --- WERYFIKACJA ISTNIENIA PLIKU WEJŚCIOWEGO ---
    // F_OK sprawdza samo istnienie pliku. R_OK sprawdziłoby uprawnienia do odczytu.
    if (access(input_path, F_OK) != 0) {
        printf("\n============================================================\n");
        printf("   BŁĄD KRYTYCZNY: NIEPRAWIDŁOWA ŚCIEŻKA    \n");
        printf("============================================================\n");
        printf("Plik wejściowy: [%s] nie istnieje lub ścieżka jest błędna.\n", input_path);
        printf("Upewnij się, że nazwa pliku i rozszerzenie są poprawne.\n");
        printf("============================================================\n");
        return 3;
    }

    Graph g = { .nodes = NULL, .edges = NULL, .node_count = 0, .edge_count = 0, .width = 1000.0, .height = 1000.0 };

    // --- FAZA 1: WALIDACJA I ODCZYT ---
    // Logika: Najpierw sprawdzamy poprawnosc struktury pliku (Kod 10), potem wczytujemy dane (Kod 2)
    if (validate_and_process(input_path) == -1){
        printf("\n=========================================================================\n");
        printf("   PRZETWARZANIE PRZERWANE: BŁĘDNA STRUKTURA GRAFU    \n");
        printf("=========================================================================\n");
        printf("Powód: Wykryto niedozwolone pętle własne lub inne błędy w strukturze grafu.\n");
        printf("Sprawdź raport powyżej i popraw plik wejściowy.\n");
        printf("=========================================================================\n");
        return 12;
    }

    if (read_graph(&g, input_path) != 0) return 2;

    if (g.edge_count == 0) {
        fprintf(stderr, "Błąd: Plik pusty.\n");
        free_graph(&g);
        return 5;
    }

    // WALIDACJA MINIMALNEJ LICZBY KRAWĘDZI
    // LOGIKA: GRAF MUSI MIEĆ CO NAJMNIEJ 2 KRAWĘDZIE, ABY SYMULACJA MIAŁA SENS
    if (g.edge_count < 2) {
        printf("\n============================================================\n");
        printf("   BŁĄD: ZA MAŁA LICZBA KRAWĘDZI    \n");
        printf("============================================================\n");
        printf("Napotkano %d krawędzi.\n", g.edge_count);
        printf("-> Potrzeba >= 2 krawędzi, aby poprawnie wyznaczyć uklad. \n");
        printf("============================================================\n");
        free_graph(&g);
        return 6; // Wykorzystujemy istniejący kod błędu dla problemów z wierzchołkami
    }

    // --- FAZA 2: WERYFIKACJA STRUKTURY I SPOJNOSCI ---

    // WALIDACJA LICZBY WIERZCHOŁKÓW
    // LOGIKA: GRAF PLANARNY WYMAGA MINIMUM 3 WIERZCHOŁKÓW DO STWORZENIA KONSTRUKCJI GEOMETRYCZNEJ
    if (g.node_count < 3) {
        printf("\n============================================================\n");
        printf("   BŁĄD: ZA MAŁA LICZBA WIERZCHOŁKÓW    \n");
        printf("============================================================\n");
        printf("Napotkano %d wierzchołków.\n", g.node_count);
        printf("-> Potrzeba >= 3 wierzchołków, by wygenerować graf planarny.\n");
        printf("============================================================\n");
        free_graph(&g);
        return 6; 
    }

    // KOMUNIKAT O ROZPOCZĘCIU SPRAWDZANIA SPÓJNOŚCI
    printf("\nSprawdzanie spójności grafu za pomocą algorytmu DFS ...\n");

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
    } else {
        // KOMUNIKAT POTWIERDZAJĄCY SPÓJNOŚĆ
        printf("Sukces: Graf jest spójny. Przechodzę do obliczeń.\n");
    }

    // --- KOMUNIKAT O WYBORZE ALGORYTMU PRZED OBLICZENIAMI ---
    if (!algo_provided) {
        printf("\n>>> INFO: Nie wpisano parametru -a. Automatycznie wybrano algorytm: TUTTE. <<<\n");
        printf(">>> INFO: Aby użyć innego układu, dodaj parametr: -a fruchterman. <<<\n");
    }

    int success = 0; // Flaga sukcesu dla całego procesu

    // --- FAZA 3: OBLICZENIA (LOGIKA WYBORU ALGORYTMU) ---

    if (strcmp(algorithm, "tutte") == 0) {
        printf("\nUruchamiam algorytm Tutte'a...\n");
        compute_tutte_layout(&g); 

        // KOMUNIKAT O SPRAWDZANIU PLANARNOSCI
        printf("\nSprawdzanie planarności wygenerowanego układu (Tutte)...\n");

        if (is_tutte_layout_planar(&g)) {
            printf("Sukces! Graf planarny (Tutte).\n");
            success = 1;
        } else {
            printf("\n============================================================\n");
            printf("   PORAŻKA: WYNIK TUTTE NIE JEST PLANARNY    \n");
            printf("============================================================\n");
            printf("Zapis pliku został zablokowany.\n"); 
        }
    } 
    else if (strcmp(algorithm, "fruchterman") == 0) {
        // Mechanizm wielokrotnych prob: Resetowanie pozycji i ponowna symulacja fizyczna
        int attempts = 0;
        printf("\nUruchamiam symulację Fruchtermana (max 3000 prób)...\n");
        // KOMUNIKAT O SPRAWDZANIU PLANARNOSCI
        printf("\nSprawdzanie planarności wygenerowanego układu (Fruchterman)...\n");

        while (!success && attempts < 3000) {
            attempts++;
            init_random_positions(&g);
            run_fruchterman(&g, 1000, 30.0);
            
            if (is_layout_planar(&g)) {
                success = 1;
                printf("Sukces! Graf planarny (Fruchterman). Wygenerowany po %d probach.\n", attempts);
            }
        }

        // DODANO: KOMUNIKAT O PORAZCE DLA FRUCHTERMANA PO WYCZERPANIU PROB
        if (!success) {
            printf("\n============================================================\n");
            printf("   KOMUNIKAT: NIE ZNALEZIONO UKŁADU PLANARNEGO    \n");
            printf("============================================================\n");
            printf("Po 3000 prób algorytm FR nie wyeliminował przecięć.\n");
            printf("Zapis pliku wyjściowego anulowany. Spróbuj ponownie.\n");
        }
    }

    // --- FAZA 4: ZAPIS I CZYSZCZENIE ---
    // Logika: Jesli osiagnieto uklad bez przeciec lub Tutte skonczyl prace, zapisujemy dane (Kod 8 przy porazce)
    if (success) {
        // KOMUNIKAT POTWIERDZAJACY SCIEZKE ZAPISU
        if (save_graph(&g, output_path, format) == 0) {
            printf("\n------------------------------------------------------------------------------\n");
            printf(">>> SUKCES: Wynik został zapisany do pliku [%s] <<<\n", output_path);
            
            // LOGIKA DYNAMICZNEGO KOMUNIKATU O BRAKU FLAGI -f
            if (format_provided) {
                if (strcmp(format, "bin") == 0) {
                    printf(">>> FORMAT: Plik BINARNY (.bin) <<<\n");
                } else {
                    printf(">>> FORMAT: Plik TEKSTOWY (.txt) <<<\n");
                }
            } else {
                // ZMIANA: KOMUNIKAT WYŚWIETLANY, GDY UŻYTKOWNIK NIE WPISAŁ -f
                printf(">>> INFO: Nie podano parametru -f bin. Zapisano domyślnie jako plik TEKSTOWY (.txt) <<<\n");
            }
        } else {
            fprintf(stderr, "Błąd: Nie udało się zapisać pliku wyjściowego!\n");
        }
    }

    free_graph(&g); 
    // ZWROT KODU 8 PRZY BRAKU SUKCESU (NIEPLANARNOSC)
    return success ? 0 : 8;
}