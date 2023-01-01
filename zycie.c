/*  Program symuluje Conway's Game of Life wykonując polecenia określone
w treści zadania zaliczeniowego.

Autor: Michał Mnich
Wersja: 3.0.0
Data: 30 grudnia 2022  */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIERSZE
#define WIERSZE 22
#endif

#ifndef KOLUMNY
#define KOLUMNY 80
#endif

#define ENTER 10 // Enter w ASCII.
#define SPACJA 32 // Spacja w ASCII.
#define KROPKA 46 // Kropka w ASCII.
#define SLASH 47 // Slash w ASCII.
#define ZERO 48 // Zero w ASCII.
#define ROWNASIE 61 // Równa się w ASCII.

#define ZARAZ_ZYWA 49 // Komórka, która zmieni się z martwej na żywą.
#define ZARAZ_MARTWA 50 // Komórka, która zmieni się z żywej na martwą.
#define ZOSTAJE_ZYWA 51 // Komórka, która pozostanie żywa.
#define ZOSTAJE_MARTWA 52 // Komórka, która pozostanie martwa.
#define ODWIEDZONA_ZYWA 53 // Żywa komórka, która została już odwiedzona.
#define ODWIEDZONA_MARTWA 54 // Martwa komórka, która została już odwiedzona.

/*  Parametry opisujące wymiary bieżącej planszy.  */
struct wymiary {
    int m; // Liczba wierszy.
    int n; // Liczba kolumn.
    int min_w; // Najmniejszy indeks wiersza.
    int min_k; // Najmniejszy indeks kolumny.
    int max_w; // Największy indeks wiersza.
    int max_k; // Największy indeks kolumny.
};
typedef struct wymiary Twymiary;

/*  Parametry opisujące okno (lewy górny róg).  */
struct okno {
    int w; // Wiersz, w którym zaczyna się okno.
    int k; // Kolumna, w której zaczyna się okno.
};
typedef struct okno Tokno;

/*  Lista zawierająca numery kolumn.  */
struct kolumny {
    int k;
    struct kolumny* nast_k;
    struct kolumny* poprz_k;
};
typedef struct kolumny Tkolumny;

/*  Lista zawierająca numery wierszy wraz z listą kolumn w tym wierszu.  */
struct wiersze {
    int w;
    Tkolumny* kolumny; 
    struct wiersze* nast_w;
    struct wiersze* poprz_w;
};
typedef struct wiersze Twiersze;

/*  Zwalnia pamięć zarezerwowaną przez dwuwymiarową tablicę 'tab' 
    o wymiarach 'wymiary'.  */
void zwolnij_2D(char** tab, Twymiary wymiary) {
    int m = wymiary.m;
    for (int i = 0; i < m; i++) {
        free(tab[i]);
    }
    free(tab);
}

/*  Zwalnia pamięć zarezerwowaną przez listę kolumn zaczynajacą się w 'l'.  */
void zwolnij_kolumny(Tkolumny** l) {
    Tkolumny* akt = *l;
    Tkolumny* temp;
    while (akt != NULL) { 
        temp = akt -> nast_k;
        free(akt);
        akt = temp;
    }
    *l = NULL;
}

/*  Zwalnia pamięć zarezerwowaną przez listę wierszy zaczynajacą się w 'l'.  */
void zwolnij_wiersze(Twiersze** l) {
    Twiersze* akt = *l;
    Twiersze* temp;
    while (akt != NULL) { 
        zwolnij_kolumny(&(akt -> kolumny));
        temp = akt -> nast_w;
        free(akt);
        akt = temp;
    }
    *l = NULL;
}

/*  Usuwa element 'l' na liście kolumn.  */
void usun_kolumne(Tkolumny* l) {
    if (l -> poprz_k != NULL) {
        l -> poprz_k -> nast_k = l -> nast_k;
    }
    if (l -> nast_k != NULL) {
        l -> nast_k -> poprz_k = l -> poprz_k;
    }
    free(l);
}

/*  Usuwa element 'l' na liście wierszy.  */
void usun_wiersz(Twiersze* l) {
    if (l -> poprz_w != NULL) {
        l -> poprz_w -> nast_w = l -> nast_w;
    }
    if (l -> nast_w != NULL) {
        l -> nast_w -> poprz_w = l -> poprz_w;
    }
    zwolnij_kolumny(&(l -> kolumny));
    free(l);
}

/*  Czyści strumień wejściowy.  */
void wyczysc_strumien(void) {
    int znak;
    while (((znak = getchar()) != ENTER) && (znak != EOF)); 
}

/*  Tworzy atrapę na liście kolumn 'l'.  */
void stworz_atrape_k(Tkolumny** l) {
    *l = (Tkolumny*) malloc(sizeof(Tkolumny));
    assert(*l != NULL);
    (*l) -> nast_k = NULL;
    (*l) -> poprz_k = NULL;
}

/*  Tworzy atrapę na liście wierszy 'l'.  */
void stworz_atrape_w(Twiersze** l) {
    *l = (Twiersze*) malloc(sizeof(Twiersze));
    assert(*l != NULL);
    (*l) -> nast_w = NULL;
    (*l) -> poprz_w = NULL;
    (*l) -> kolumny = NULL;
}

/*  Inicjalizuje tablicę 'tab' tak, aby miała 'm' wierszy i 'n' kolumn i była
    cała wypełniona kropkami.  */
void inicjalizuj(char*** tab, int m, int n) {
    char** pom = (char**) malloc((size_t) m * sizeof(char*));
    assert(pom != NULL);
    for (int i = 0; i < m; i++) {
        pom[i] = (char*) malloc((size_t) n * sizeof(char));
        assert(pom[i] != NULL);
        for (int j = 0; j < n; j++) {
            pom[i][j] = KROPKA;
        }
    }
    *tab = pom;
}

/*  Wstawia kolumnę 'x' za element 'l1' na liście kolumn w danym wierszu.  */
void wstaw_za_kolumne(Tkolumny* l1, int x) {
    Tkolumny* pom;
    pom = (Tkolumny*) malloc(sizeof(Tkolumny));
    assert(pom != NULL);
    pom -> k = x;
    pom -> nast_k = l1 -> nast_k;
    pom -> poprz_k = l1;
    l1 -> nast_k = pom;
    if (pom -> nast_k != NULL) {
        pom -> nast_k -> poprz_k = pom;
    }
}

/*  Wstawia wiersz 'x' z kolumnami reprezentowanymi na liście 'l2' za element 
    'l1' na liście wierszy.  */
void wstaw_za_wiersz(Twiersze* l1, int x, Tkolumny* l2) {
    Twiersze* pom;
    pom = (Twiersze*) malloc(sizeof(Twiersze));
    assert(pom != NULL);
    pom -> w = x;
    pom -> kolumny = l2;
    pom -> nast_w = l1 -> nast_w;
    pom -> poprz_w = l1;
    l1 -> nast_w = pom;
    if (pom -> nast_w != NULL) {
        pom -> nast_w -> poprz_w = pom;
    }
}

/*  Dołącza listę kolumn 'l2' za element 'l1'.  */
void dolacz_na_koniec_k(Tkolumny* l1, Tkolumny* l2) {
    if (l1 != NULL && l2 != NULL) {
        l1 -> nast_k = l2;
        l2 -> poprz_k = l1;
    }
}

/*  Dołącza listę wierszy 'l2' za element 'l1'.  */
void dolacz_na_koniec_w(Twiersze* l1, Twiersze* l2) {
    if (l1 != NULL && l2 != NULL) {
        l1 -> nast_w = l2;
        l2 -> poprz_w = l1;
    }
}

/*  Scala dwie listy z kolumnami 'l1' i 'l2' posortowane rosnąco i przekazuje 
    wskaźnik do wyniku. Zwalnia również pamięć zarezerwowaną przez listy 
    'l1' i 'l2'.  */
Tkolumny* scal_posort_k(Tkolumny* l1, Tkolumny* l2) {
    Tkolumny* do_usuniecia;
    Tkolumny* pom; // Lista wynikowa.
    stworz_atrape_k(&pom);
    Tkolumny* akt_pom = pom;
    // Omijamy atrapy na listach 'l1' i 'l2'.
    Tkolumny* akt1 = l1 -> nast_k;
    Tkolumny* akt2 = l2 -> nast_k;
    // I je zwalniamy.
    free(l1);
    free(l2);
    while (akt1 != NULL && akt2 != NULL) {
        if (akt1 -> k < akt2 -> k) {
            wstaw_za_kolumne(akt_pom, akt1 -> k);
            do_usuniecia = akt1;
            akt1 = akt1 -> nast_k;
            free(do_usuniecia);
        }
        else if (akt1 -> k > akt2 -> k) {
            wstaw_za_kolumne(akt_pom, akt2 -> k);
            do_usuniecia = akt2;
            akt2 = akt2 -> nast_k;
            free(do_usuniecia);
        }
        else { // Jeśli takie same.
            wstaw_za_kolumne(akt_pom, akt1 -> k);
            do_usuniecia = akt1;
            akt1 = akt1 -> nast_k;
            free(do_usuniecia);
            do_usuniecia = akt2;
            akt2 = akt2 -> nast_k;
            free(do_usuniecia);
        }
        akt_pom = akt_pom -> nast_k;
    }
    // Dopisywanie reszty listy, jeśli któraś się skończyła.
    if (akt1 == NULL) {
        dolacz_na_koniec_k(akt_pom, akt2);
    }
    else { // akt2 == NULL
        dolacz_na_koniec_k(akt_pom, akt1);
    }
    return pom;
}

/*  Scala dwie listy z wierszami 'l1' i 'l2' posortowane rosnąco i przekazuje 
    wskaźnik do wyniku. Zwalnia również pamięć zarezerwowaną przez listy 
    'l1' i 'l2'.  */
Twiersze* scal_posort_w(Twiersze* l1, Twiersze* l2) {
    Tkolumny* nowe;
    Twiersze* do_usuniecia;
    Twiersze* pom; // Lista wynikowa.
    stworz_atrape_w(&pom);
    Twiersze* akt_pom = pom;
    // Omijamy atrapy na listach 'l1' i 'l2'.
    Twiersze* akt1 = l1 -> nast_w;
    Twiersze* akt2 = l2 -> nast_w;
    // I je zwalniamy.
    free(l1);
    free(l2);
    while (akt1 != NULL && akt2 != NULL) {
        if (akt1 -> w < akt2 -> w) {
            wstaw_za_wiersz(akt_pom, akt1 -> w, akt1 -> kolumny);
            do_usuniecia = akt1;
            akt1 = akt1 -> nast_w;
            free(do_usuniecia);
        }
        else if (akt1 -> w > akt2 -> w) {
            wstaw_za_wiersz(akt_pom, akt2 -> w, akt2 -> kolumny);
            do_usuniecia = akt2;
            akt2 = akt2 -> nast_w;
            free(do_usuniecia);
        }
        else { // Jeśli takie same.
            nowe = scal_posort_k(akt1 -> kolumny, akt2 -> kolumny);
            wstaw_za_wiersz(akt_pom, akt1 -> w, nowe);
            do_usuniecia = akt1;
            akt1 = akt1 -> nast_w;
            free(do_usuniecia);
            do_usuniecia = akt2;
            akt2 = akt2 -> nast_w;
            free(do_usuniecia);
        }
        akt_pom = akt_pom -> nast_w;
    }
    // Dopisywanie reszty listy, jeśli któraś się skonczyła.
    if (akt1 == NULL) {
        dolacz_na_koniec_w(akt_pom, akt2);
    }
    else { // akt2 == NULL
        dolacz_na_koniec_w(akt_pom, akt1);
    }
    return pom;
}

/*  Rozszerza jednowymiarową tablicę typu 'char' do długości 'rozmiar'.  */
void rozszerz_char(char** tab, int rozmiar) {
    *tab = realloc(*tab, (size_t) rozmiar * sizeof(char));
    assert(*tab != NULL);
}

/*  Rozszerza dwuwymiarową tablicę typu 'char' tak, aby miała 
    'rozmiar' wierszy.  */
void rozszerz_char_gw(char ***tab, int rozmiar) {
    *tab = realloc(*tab, (size_t) rozmiar * sizeof(char*));
    assert(*tab != NULL);
}

/*  Dodaje 'ile_trzeba' wierszy na dole planszy wypełnionych kropkami.
    Aktualizuje również wymiary planszy.  */
void rozszerz_dol(char*** tab, Twymiary* wymiary, int ile_trzeba) {
    rozszerz_char_gw(tab, wymiary -> m + ile_trzeba);
    for (int i = wymiary -> m; i < (wymiary -> m) + ile_trzeba; i++) {
        (*tab)[i] = NULL;
        rozszerz_char(*tab + i, wymiary -> n);
        for (int j = 0; j < wymiary -> n; j++) {
            (*tab)[i][j] = KROPKA;
        }
    }
    wymiary -> max_w += ile_trzeba;
    wymiary -> m += ile_trzeba;
}

/*  Dodaje 'ile_trzeba' wierszy na górze planszy wypełnionych kropkami.
    Aktualizuje również wymiary planszy.  */
void rozszerz_gore(char*** tab, Twymiary* wymiary, int ile_trzeba) {
    char** pom;
    // Alokowanie tablicy wypełnionej kropkami o nowych wymiarach.
    inicjalizuj(&pom, (wymiary -> m) + ile_trzeba, wymiary -> n);
    // Kopiowanie tablicy 'tab' zostawiając 'ile_trzeba' pustych wierszy u góry.
    for (int i = 0; i < wymiary -> m; i++) {
        for (int j = 0; j < wymiary -> n; j++) {
            pom[i + ile_trzeba][j] = (*tab)[i][j];
        }
    }
    // Przypisanie nowej tablicy.
    zwolnij_2D(*tab, *wymiary);
    *tab = pom;
    wymiary -> min_w -= ile_trzeba;
    wymiary -> m += ile_trzeba;
}

/*  Dodaje 'ile_trzeba' kolumn na lewo planszy wypełnionych kropkami.
    Aktualizuje również wymiary planszy.  */
void rozszerz_lewo(char*** tab, Twymiary* wymiary, int ile_trzeba) {
    char** pom;
    // Alokowanie tablicy wypełnionej kropkami o nowych wymiarach.
    inicjalizuj(&pom, wymiary -> m, (wymiary -> n + ile_trzeba));
    // Kopiowanie tablicy 'tab' zostawiając 'ile_trzeba' pustych kolumn na lewo.
    for (int i = 0; i < wymiary -> m; i++) {
        for (int j = 0; j < wymiary -> n; j++) {
            pom[i][j + ile_trzeba] = (*tab)[i][j];
        }
    }
    // Przypisanie nowej tablicy.
    zwolnij_2D(*tab, *wymiary);
    *tab = pom;
    wymiary -> min_k -= ile_trzeba;
    wymiary -> n += ile_trzeba;
}

/*  Dodaje 'ile_trzeba' kolumn na prawo planszy wypełnionych kropkami.
    Aktualizuje również wymiary planszy.  */
void rozszerz_prawo(char*** tab, Twymiary* wymiary, int ile_trzeba) {
    for (int i = 0; i < wymiary -> m; i++) {
        rozszerz_char(*tab + i, (wymiary -> n) + ile_trzeba);
        for (int j = wymiary -> n; j < (wymiary -> n) + ile_trzeba; j++) {
            (*tab)[i][j] = KROPKA;
        }
    }
    wymiary -> max_k += ile_trzeba;
    wymiary -> n += ile_trzeba;        
}

/*  Jeśli trzeba, rozszerza planszę tak, aby można było wypisać okno.  */
void rozszerz_plansze(char*** tab, int w, int k, Twymiary* wymiary) {
    int ile_trzeba;
    if (w + WIERSZE - 1 > wymiary -> max_w) {
        // Jeśli okno wyszłoby poza dolny zakres.
        ile_trzeba = WIERSZE - ((wymiary -> max_w) + 1 - w);
        rozszerz_dol(tab, wymiary, ile_trzeba);
    }
    if (w < wymiary -> min_w) {
        // Jeśli okno wyszłoby poza górny zakres.
        ile_trzeba = (wymiary -> min_w) - w;
        rozszerz_gore(tab, wymiary, ile_trzeba);
    }
    if (k + KOLUMNY - 1 > wymiary -> max_k) {
        // Jeśli okno wyszłoby poza prawy zakres.
        ile_trzeba = KOLUMNY - ((wymiary -> max_k) + 1 - k);
        rozszerz_prawo(tab, wymiary, ile_trzeba);
    }
    if (k < wymiary -> min_k) {
        // Jeśli okno wyszłoby poza lewy zakres.
        ile_trzeba = (wymiary -> min_k) - k;
        rozszerz_lewo(tab, wymiary, ile_trzeba);
    }
}

/*  Funkcja przekazująca optymalną wartość, o jaką należy poszerzyć planszę.  */
int wiecej(int n) {
    return n / 2 + 26;
}

/*  Rozszerza planszę tak, by była pewność, że podwójne otoczenie 
    (czyli sąsiedzi wszystkich sąsiadów) komórki w wierszu 'w' i kolumnie 'k' 
    jest zaalokowane. Jeśli trzeba, alokuje wiecej wierszy lub kolumn.  */
void rozszerz_otoczenie(char*** tab, int w, int k, Twymiary* wymiary) {
    if (w + 2 > wymiary -> max_w) {
        rozszerz_dol(tab, wymiary, wiecej(wymiary -> m));
    }
    if (w - 2 < wymiary -> min_w) {
        rozszerz_gore(tab, wymiary, wiecej(wymiary -> m));
    }
    if (k + 2 > wymiary -> max_k) {
        rozszerz_prawo(tab, wymiary, wiecej(wymiary -> n));
    }
    if (k - 2 < wymiary -> min_k) {
        rozszerz_lewo(tab, wymiary, wiecej(wymiary -> n));
    }
}

/*  Wypisuje okno zadane przez 'okno' o 'WIERSZE' wierszach 
    i 'KOLUMNY' kolumnach.  */
void wypisz_okno(char** tab, Tokno okno, Twymiary wymiary) {
    int w = okno.w, k = okno.k;
    int min_w = wymiary.min_w, min_k = wymiary.min_k;
    for (int i = w - min_w; i < w - min_w + WIERSZE; i++) {
        for (int j = k - min_k; j < k - min_k + KOLUMNY; j++) {
            putchar(tab[i][j]);
        }
        putchar(ENTER);
    }
    for (int i = 0; i < KOLUMNY; i++) {
        putchar(ROWNASIE);
    }
    putchar(ENTER);
}

/*  Wczytuje żywe wiersze i żywe kolumny w początkowej generacji do listy 
    'zywe_wiersze'.  */
void wczytaj_generacje(Twiersze* zywe_wiersze) {
    Tkolumny* kol_nowe;
    Tkolumny* akt_k;
    int w, k;
    char znak;
    int koniec = 0; // Flaga oznaczająca koniec wczytywania.
    while (!koniec) {
        getchar(); // Pomiń pierwszy slash.
        znak = (char) getchar(); // Podejrzyj znak po slashu.
        ungetc(znak, stdin);
        if (znak == ENTER) {
            koniec = 1;
        }
        else {
            scanf("%d", &w);
            // Dodajemy wczytany wiersz do listy żywych wierszy.
            wstaw_za_wiersz(zywe_wiersze, w, NULL);
            // Tworzymy atrapę na liście kolumn w dodanym wierszu.
            stworz_atrape_k(&(zywe_wiersze -> nast_w -> kolumny));
            kol_nowe = zywe_wiersze -> nast_w -> kolumny;
            akt_k = kol_nowe;
            // Wczytywanie żywych kolumn w wierszu 'w'.
            do {
                scanf("%d%c", &k, &znak);
                wstaw_za_kolumne(akt_k, k);
                akt_k = akt_k -> nast_k;
            } while (znak == SPACJA);
            // Jeśli dodaliśmy jakieś kolumny, idziemy dalej.
            if (akt_k != kol_nowe) {
                zywe_wiersze = zywe_wiersze -> nast_w;
            }
            // Jeśli nie, usuwamy dodany wiersz.
            else {
                usun_wiersz(zywe_wiersze -> nast_w);
            }
        }
    }
}

/*  Wypisuje stan aktualnej generacji.  */
void zrzut(Twiersze* zywe_wiersze) {
    Twiersze* akt_w;
    Tkolumny* akt_k;
    // Omijamy atrapę na liście żywych wierszy.
    akt_w = zywe_wiersze -> nast_w;
    while (akt_w != NULL) {
        printf("%c%d", SLASH, akt_w -> w);
        // Omijamy atrapę na liście żywych kolumn.
        akt_k = akt_w -> kolumny -> nast_k;
        while (akt_k != NULL) {
            printf(" %d", akt_k -> k);
            akt_k = akt_k -> nast_k;
        }
        putchar(ENTER);
        akt_w = akt_w -> nast_w;
    }
    putchar(SLASH);
    putchar(ENTER);
}

/*  Ustawia pola planszy reprezentowane na liście 'wiersze' na znak 'jak'.  */
void ustaw(char*** tab, Twymiary* wymiary, Twiersze* wiersze, char jak) {
    Twiersze* akt_w;
    Tkolumny* akt_k;
    // Omijamy atrapę na liście żywych wierszy.
    akt_w = wiersze -> nast_w;
    while (akt_w != NULL) {
        // Omijamy atrapę na liście żywych kolumn.
        akt_k = akt_w -> kolumny -> nast_k;
        while (akt_k != NULL) {
            // Każda komórka musi mieć podwójne sąsiedztwo.
            rozszerz_otoczenie(tab, akt_w -> w, akt_k -> k, wymiary);
            (*tab)[(akt_w -> w) - (wymiary -> min_w)]
                  [(akt_k -> k) - (wymiary -> min_k)] = jak;
            akt_k = akt_k -> nast_k;
        }
        akt_w = akt_w -> nast_w;
    }
}

/*  Przechodzi po liście 'zywe_wiersze' i dopisuje do listy 'l' komórki, które 
    są o 'offset' powyżej lub poniżej rozważanej komórki na liście 
    'zywe_wiersze'. Jeśli komórka została dopisana do listy, jest zmieniana 
    na ODWIEDZONA_ZYWA lub ODWIEDZONA_MARTWA, aby nie liczyć tych samych 
    komórek kilka razy.  */
void wczytaj_otoczenie(
    char*** tab, 
    Twiersze* l, 
    int offset, 
    Twiersze* zywe_wiersze,
    Twymiary wymiary) {

    Twiersze* akt_l = l;
    Tkolumny* akt_k;
    Tkolumny *kol_nowe, *akt_n;
    int wiersz, kolumna;
    // Omijamy atrapę na liście żywych wierszy.
    Twiersze* akt_w = zywe_wiersze -> nast_w;
    while (akt_w != NULL) {
        wiersz = (akt_w -> w) - wymiary.min_w + offset;
        // Omijamy atrapę na liście żywych kolumn.
        akt_k = akt_w -> kolumny -> nast_k;
        // Dodajemy rozważany wiersz do listy.
        wstaw_za_wiersz(akt_l, (akt_w -> w) + offset, NULL);
        // Tworzymy atrapę na liście kolumn w dodanym wierszu.
        stworz_atrape_k(&(akt_l -> nast_w -> kolumny));
        kol_nowe = akt_l -> nast_w -> kolumny;
        akt_n = kol_nowe;
        while (akt_k != NULL) {
            for (int i = -1; i < 2; i++) {
                kolumna = (akt_k -> k) - wymiary.min_k + i;
                switch ((*tab)[wiersz][kolumna]) {
                    case ZERO:
                        wstaw_za_kolumne(akt_n, (akt_k -> k) + i);
                        akt_n = akt_n -> nast_k;
                        (*tab)[wiersz][kolumna] = ODWIEDZONA_ZYWA;
                        break;
                    case KROPKA:
                        wstaw_za_kolumne(akt_n, (akt_k -> k) + i);
                        akt_n = akt_n -> nast_k;
                        (*tab)[wiersz][kolumna] = ODWIEDZONA_MARTWA;
                        break;
                }
            }
            akt_k = akt_k -> nast_k;
        }
        // Jeśli dodaliśmy jakieś kolumny, idziemy dalej.
        if (akt_n != kol_nowe) {
            akt_l = akt_l -> nast_w;
        }
        // Jeśli nie, usuwamy dodany wiersz.
        else {
            usun_wiersz(akt_l -> nast_w);
        }
        akt_w = akt_w -> nast_w;
    }
}

/*  Tworzy listę komórek, które trzeba koniecznie sprawdzić przy szukaniu 
    następnej generacji i przekazuje wskaźnik do wyniku.  */
Twiersze* wczytaj_do_sprawdzenia(
    char*** tab, 
    Twiersze* zywe_wiersze, 
    Twymiary wymiary) {

    Twiersze *gora, *srodek, *dol, *temp;
    stworz_atrape_w(&gora);
    stworz_atrape_w(&srodek);
    stworz_atrape_w(&dol);
    wczytaj_otoczenie(tab, gora, -1, zywe_wiersze, wymiary);
    wczytaj_otoczenie(tab, srodek, 0, zywe_wiersze, wymiary);
    wczytaj_otoczenie(tab, dol, 1, zywe_wiersze, wymiary);
    temp = scal_posort_w(gora, srodek);
    return scal_posort_w(temp, dol);
}

/*  Sprawdza, czy komórka w wierszu 'w' i kolumnie 'k' jest zaalokowana. 
    Zwraca '1' jeśli tak, '0' jeśli nie.  */
int czy_w_zakresie(Twymiary wymiary, int w, int k) {
    return ((wymiary.min_w <= w) && (w <= wymiary.max_w) &&
            (wymiary.min_k <= k) && (k <= wymiary.max_k));
}

/*  Sprawdza, czy komórka w wierszu 'w' i kolumnie 'k' (na tablicy 'tab') 
    zmieni status w następnej generacji. Jeśli stanie się żywa, zmienia jej 
    wartość na 'ZARAZ_ZYWA'. Jeśli stanie się martwa, zmienia jej wartość na 
    'ZARAZ_MARTWA'. Jeśli się nie zmieni, zmienia jej wartość na 
    'ZOSTAJE_ZYWA' lub 'ZOSTAJE_MARTWA'.  */
void sprawdz(char*** tab, int w, int k, Twymiary wymiary) {
    char komorka; // Rozważana komórka.
    int ile_zywych = 0;
    // Zliczanie żywych sąsiadów.
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            // Zawsze będzie w zakresie.
            assert(czy_w_zakresie(
            wymiary, w + i + wymiary.min_w, k + j + wymiary.min_k));

            komorka = (*tab)[w + i][k + j];
            if (komorka == ZERO ||
                komorka == ZARAZ_MARTWA ||
                komorka == ODWIEDZONA_ZYWA ||
                komorka == ZOSTAJE_ZYWA) {

                ile_zywych++;
            }
        }
    }
    switch ((*tab)[w][k]) {
        case ODWIEDZONA_ZYWA:
            ile_zywych--; // Wtedy policzyliśmy o jedną żywą za dużo.
            if (ile_zywych != 2 && ile_zywych != 3) {
                (*tab)[w][k] = ZARAZ_MARTWA;
            }
            else {
                (*tab)[w][k] = ZOSTAJE_ZYWA;
            }
            break;
        case ODWIEDZONA_MARTWA:
            if (ile_zywych == 3) {
                (*tab)[w][k] = ZARAZ_ZYWA;
            }
            else {
                (*tab)[w][k] = ZOSTAJE_MARTWA;
            }
            break;
        default: assert(0); // Błąd.
    }
}

/*  Idzie po liście 'do_sprawdzenia' i dopisuje komórki, które powinny być żywe
    w następnej generacji do listy 'zywe_wiersze_next', oraz komórki, które 
    powinny być martwe do listy 'martwe_wiersze_next'.  */
void oblicz_zywe_i_martwe(
    char*** tab, 
    Twiersze* do_sprawdzenia, 
    Twiersze* zywe_wiersze_next, 
    Twiersze* martwe_wiersze_next,
    Twymiary wymiary) {

    int wiersz, kolumna; // Miejsca komórek na tablicy 'tab'.
    // Omijamy atrapę na liście wierszy do sprawdzenia.
    Twiersze* akt_w = do_sprawdzenia -> nast_w;
    Tkolumny* akt_k;
    Tkolumny *kol_nowe_z, *kol_nowe_m;
    Tkolumny *akt_z, *akt_m;
    while (akt_w != NULL) {
        // Dodajemy rozważany wiersz do obu list.
        wstaw_za_wiersz(zywe_wiersze_next, akt_w -> w, NULL);
        wstaw_za_wiersz(martwe_wiersze_next, akt_w -> w, NULL);
        // Tworzymy atrapę na listach kolumn w obu wierszach.
        stworz_atrape_k(&(zywe_wiersze_next -> nast_w -> kolumny));
        stworz_atrape_k(&(martwe_wiersze_next -> nast_w -> kolumny));
        // Inicjalizacja wskaźników pomocniczych.
        kol_nowe_z = zywe_wiersze_next -> nast_w -> kolumny;
        kol_nowe_m = martwe_wiersze_next -> nast_w -> kolumny;
        akt_z = kol_nowe_z;
        akt_m = kol_nowe_m;
        wiersz = (akt_w -> w) - wymiary.min_w;
        // Omijamy atrapę na liście kolumn do sprawdzenia.
        akt_k = akt_w -> kolumny -> nast_k;
        while(akt_k != NULL) {
            kolumna = (akt_k -> k) - wymiary.min_k;
            // Zawsze będzie w zakresie.
            assert(czy_w_zakresie(wymiary, akt_w -> w, akt_k -> k));
            sprawdz(tab, wiersz, kolumna, wymiary);
            switch ((*tab)[wiersz][kolumna]) {
                case ZARAZ_ZYWA: case ZOSTAJE_ZYWA:
                    wstaw_za_kolumne(akt_z, akt_k -> k);
                    akt_z = akt_z -> nast_k;
                    break;
                case ZARAZ_MARTWA: case ZOSTAJE_MARTWA:
                    wstaw_za_kolumne(akt_m, akt_k -> k);
                    akt_m = akt_m -> nast_k;
                    break;
                default: assert(0); // Błąd.
            }
            akt_k = akt_k -> nast_k;
        }
        // Jeśli dodaliśmy jakieś kolumny, idziemy dalej.
        if (akt_z != kol_nowe_z) {
            zywe_wiersze_next = zywe_wiersze_next -> nast_w;
        }
        // Jeśli nie, usuwamy dodany wiersz.
        else {
            usun_wiersz(zywe_wiersze_next -> nast_w);
        }
        // To samo z drugą listą.
        if (akt_m != kol_nowe_m) {
            martwe_wiersze_next = martwe_wiersze_next -> nast_w;
        }
        else {
            usun_wiersz(martwe_wiersze_next -> nast_w);
        }
        akt_w = akt_w -> nast_w;
    }
}

/*  Oblicza następną generację i odpowiednio zmienia planszę.  */
void nastepna_generacja(
    char*** tab, 
    Twiersze** zywe_wiersze,
    Twymiary* wymiary) {

    // Lista komórek, które należy sprawdzić przy szukaniu następnej generacji.
    Twiersze* do_sprawdzenia;
    // Lista wszystkich żywych komórek w następnej generacji.
    Twiersze* zywe_wiersze_next;
    // Lista wszystkich martwych komórek w następnej generacji.
    Twiersze* martwe_wiersze_next;
    // Tworzenie atrap.
    stworz_atrape_w(&zywe_wiersze_next);
    stworz_atrape_w(&martwe_wiersze_next);
    do_sprawdzenia = wczytaj_do_sprawdzenia(tab, *zywe_wiersze, *wymiary);
    oblicz_zywe_i_martwe(
    tab, do_sprawdzenia, zywe_wiersze_next, martwe_wiersze_next, *wymiary);
    // Ustawianie planszy.
    ustaw(tab, wymiary, zywe_wiersze_next, ZERO);
    ustaw(tab, wymiary, martwe_wiersze_next, KROPKA);
    // Zwalnianie pamięci.
    zwolnij_wiersze(zywe_wiersze);
    zwolnij_wiersze(&do_sprawdzenia);
    zwolnij_wiersze(&martwe_wiersze_next);
    // Zaktualizowanie listy żywych wierszy.
    *zywe_wiersze = zywe_wiersze_next;
}

/*  Kieruje tym, co ma się wydarzyć w programie w zależności od otrzymanego
    wejścia. Jeśli trzeba wyjść z programu, przekazuje 1, w przeciwnym
    przypadku przekazuje 0.  */
int wejscie(
    char*** tab, 
    Tokno* okno, 
    Twymiary* wymiary, 
    Twiersze** zywe_wiersze) {

    int pierwszy = getchar();
    ungetc(pierwszy, stdin);
    switch (pierwszy) {
        case KROPKA: // Zakończ program.
            return 1;
            break;
        case SLASH: // Wczytaj początkową generację.
            wczytaj_generacje(*zywe_wiersze);
            ustaw(tab, wymiary, *zywe_wiersze, ZERO);
            break;
        case ENTER: // Obliczenie kolejnej generacji.
            nastepna_generacja(tab, zywe_wiersze, wymiary);
            break;
        default:;
            int a, b;
            char c;
            scanf("%d%c", &a, &c);
            if (c == SPACJA) { // Przesuwanie okna.
                scanf("%d", &b);
                okno -> w = a;
                okno -> k = b;
                rozszerz_plansze(tab, a, b, wymiary);
            }
            else if (c == ENTER) {
                ungetc(ENTER, stdin);
                if (a == 0) { // Zrzut bieżącej generacji.
                    zrzut(*zywe_wiersze);
                }
                else { // Obliczenie 'a' kolejnych generacji.
                    for (int i = 0; i < a; i++) {
                        nastepna_generacja(tab, zywe_wiersze, wymiary);
                    }
                }
            }
    }
    return 0;
}

int main(void) {
    
    // Inicjalizacja planszy.
    char** plansza;
    inicjalizuj(&plansza, WIERSZE, KOLUMNY);
    // Inicjalizacja zmiennych.
    //
    // Lista wszystkich żywych komórek w bieżącej generacji.
    Twiersze* zywe_wiersze;
    // Tworzenie atrapy.
    stworz_atrape_w(&zywe_wiersze);
    // Bieżące okno.
    Tokno okno;
    okno.w = 1;
    okno.k = 1;
    // Bieżące wymiary planszy.
    Twymiary wymiary;
    wymiary.m = WIERSZE;
    wymiary.n = KOLUMNY;
    wymiary.min_w = 1;
    wymiary.min_k = 1;
    wymiary.max_w = WIERSZE;
    wymiary.max_k = KOLUMNY;

    while (!wejscie(&plansza, &okno, &wymiary, &zywe_wiersze)) {
        wypisz_okno(plansza, okno, wymiary);
        wyczysc_strumien();
    }
    zwolnij_2D(plansza, wymiary);
    zwolnij_wiersze(&zywe_wiersze);
    
    return 0;
}
