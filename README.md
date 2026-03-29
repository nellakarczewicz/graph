*Graph Layout Engine (C Edition)*

Zaawansowane narzędzie konsolowe do automatycznego rozmieszczania wierzchołków grafu w przestrzeni 2D. Program implementuje dwa odmienne podejścia: matematyczną metodę barycentryczną oraz fizyczny model siłowy.


*Główne Funkcjonalności*

    Hybrydowy Parser I/O: Inteligentna obsługa plików tekstowych z automatyczną korektą separatorów (, vs ;) oraz kropki dziesiętnej.

    Silnik Walidacji: Przed obliczeniami program przeprowadza pełną analizę strukturalną grafu, generując raport spójności i poprawności danych.

    Gwarancja Planarności (Tutte): Implementacja zapewniająca brak przecięć krawędzi dla grafów trójspójnych poprzez rozwiązanie układu równań liniowych.

    Fizyczna Optymalizacja (Fruchterman-Reingold): Iteracyjne wyznaczanie pozycji wierzchołków z mechanizmem "Retry Logic" (do 3000 prób) w celu znalezienia ułożenia planarnego.

*Instrukcja Kompilacji*

Projekt wykorzystuje plik Makefile do automatyzacji procesu budowania.
make        # Kompilacja całego projektu
make clean  # Usunięcie plików obiektowych i binariów

*Uruchomienie i Składnia*

Bash

./program -i <wejscie.txt> -o <wyjscie> -a <algorytm> [-f <format>]

Parametry:

    -i : Plik wejściowy w formacie Nazwa;ID_U;ID_V;Waga.

    -o : Ścieżka do pliku wynikowego.

    -a : Wybór algorytmu: tutte (matematyczny) lub fruchterman (siłowy).

    -f : Format zapisu: txt (czytelny dla człowieka) lub bin (szybki zapis binarny).

*Diagnostyka i Kody Błędów*

Program zwraca specyficzne kody wyjścia w przypadku napotkania problemów z danymi:

Kod	    Znaczenie  	        Przyczyna

6   	Błąd rozmiaru   	Algorytm Tutte'a wymaga minimum 3 wierzchołków.
7   	Brak planarności    Złamanie wzoru Eulera (E>3V−6) – graf zbyt gęsty.
8   	Błąd optymalizacji  Nie udało się rozplątać grafu po 3000 próbach (Fruchterman).
9   	Brak spójności      Graf jest rozbity na osobne komponenty.
10  	Błąd krytyczny      Niepoprawny format danych w pliku wejściowym.

*Struktura Modułowa Projektu*

Projekt został podzielony na niezależne moduły, co zapewnia separację logiki obliczeniowej od operacji wejścia/wyjścia.
1. Rdzeń Danych (graph.h)

    Definiuje kluczowe struktury: Node (wierzchołek z pozycją i wektorem siły), Edge (krawędź z wagą) oraz Graph (kontener danych).

    Zapewnia spójność danych pomiędzy wszystkimi algorytmami.

2. Moduł Komunikacji i Analizy (io.c, io.h)

    Parser: Obsługuje wczytywanie list krawędzi z automatycznym mapowaniem ID wierzchołków na ciągłe indeksy tablicy.

    Walidator: Funkcja validate_and_process wykonuje wstępny skan pliku, raportując poprawki formatowania i błędy krytyczne.

    Analizator Grafu: Zawiera implementację algorytmu DFS sprawdzającego spójność (is_graph_connected) oraz weryfikację teoretycznej planarności na podstawie wzoru Eulera (is_potentially_planar).

3. Moduł Metody Barycentrycznej (layout_tutte.c, layout_tutte.h)

    Implementuje algorytm Tutte’a, który rozmieszcza wierzchołki zewnętrzne na obwodzie koła, a wewnętrzne w ich baricentrum (średniej arytmetycznej sąsiadów).

    Gwarantuje deterministyczne i planarne ułożenie dla grafów spełniających założenia teoretyczne.

4. Moduł Modelu Siłowego (fruchterman.c, fruchterman.h)

    Realizuje dynamiczną symulację fizyczną, w której wierzchołki odpychają się, a krawędzie przyciągają.

    Geometria Obliczeniowa: Zawiera funkcję is_layout_planar, która wykorzystuje algorytm orientacji punktów (CCW) do detekcji przecięć krawędzi w czasie rzeczywistym.

5. Moduł Sterujący (main.c)

    Zarządza cyklem życia programu: od obsługi argumentów CLI (getopt), przez proces walidacji, aż po pętlę optymalizacji (do 3000 prób dla algorytmów stochastycznych).