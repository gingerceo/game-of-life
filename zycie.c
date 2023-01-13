/*  Program symuluje Conway's Game of Life wykonując polecenia określone
w treści zadania zaliczeniowego.

Autor: Michał Mnich
Wersja: 3.0.0
Data: 10 stycznia 2023  */

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
#define ROWNASIE 61 // Równa się w ASCII.

#define ZYWA 48 // Reprezentacja żywej komórki na planszy.
#define MARTWA 0 // Reprezentacja martwej komórki na planszy.
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

/*  Parametry opisujące bieżące okno (lewy górny róg).  */
struct okno {
    int w; // Wiersz, w którym zaczyna się okno.
    int k; // Kolumna, w której zaczyna się okno.
};
typedef struct okno Tokno;

/*  Lista zawierająca numery kolumn.  */
struct kolumny {
    int k;
    struct kolumny* nast_k;
};
typedef struct kolumny Tkolumny;

/*  Lista zawierająca numery wierszy wraz z listą kolumn w tym wierszu.  */
struct wiersze {
    int w;
    Tkolumny* kolumny; 
    struct wiersze* nast_w;
};
typedef struct wiersze Twiersze;

/*  Funkcja przekazująca optymalną wartość, o jaką należy poszerzyć planszę.  */
int wiecej(int n) {
    return n / 2 + 40;
}

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
}

/*  Tworzy atrapę na liście wierszy 'l'.  */
void stworz_atrape_w(Twiersze** l) {
    *l = (Twiersze*) malloc(sizeof(Twiersze));
    assert(*l != NULL);
    (*l) -> nast_w = NULL;
    (*l) -> kolumny = NULL;
}

/*  Inicjalizuje tablicę 'tab' tak, aby miała 'm' wierszy i 'n' kolumn.  
    Wypełnia ją zerami (martwymi komórkami). */
void inicjalizuj(char*** tab, int m, int n) {
    char** pom = (char**) malloc((size_t) m * sizeof(char*));
    assert(pom != NULL);
    for (int i = 0; i < m; i++) {
        pom[i] = (char*) calloc((size_t) n, sizeof(char));
        assert(pom[i] != NULL);
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
    l1 -> nast_k = pom;
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
    l1 -> nast_w = pom;
}

/*  Dołącza listę kolumn 'l2' za element 'l1'.  */
void dolacz_na_koniec_k(Tkolumny* l1, Tkolumny* l2) {
    if (l1 != NULL && l2 != NULL) {
        l1 -> nast_k = l2;
    }
}

/*  Dołącza listę wierszy 'l2' za element 'l1'.  */
void dolacz_na_koniec_w(Twiersze* l1, Twiersze* l2) {
    if (l1 != NULL && l2 != NULL) {
        l1 -> nast_w = l2;
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

/*  Ustawia pola planszy reprezentowane na liście 'wiersze' na znak 'jak'.  */
void ustaw(char*** tab, Twymiary wymiary, Twiersze* wiersze, char jak) {
    Twiersze* akt_w;
    Tkolumny* akt_k;
    // Omijamy atrapę na liście żywych wierszy.
    akt_w = wiersze -> nast_w;
    while (akt_w != NULL) {
        // Omijamy atrapę na liście żywych kolumn.
        akt_k = akt_w -> kolumny -> nast_k;
        while (akt_k != NULL) {
            (*tab)[(akt_w -> w) - (wymiary.min_w)]
                  [(akt_k -> k) - (wymiary.min_k)] = jak;
            akt_k = akt_k -> nast_k;
        }
        akt_w = akt_w -> nast_w;
    }
}

/*  Rozszerza tablicę 'tab' tak, aby miała o 'm' więcej wierszy i o 'n' więcej 
    kolumn oraz zeruje wszystkie jej pola.  */
void rozszerz_i_wyzeruj(char*** tab, int m, int n, Twymiary wymiary) {
    zwolnij_2D(*tab, wymiary);
    char** pom;
    inicjalizuj(&pom, wymiary.m + m, wymiary.n + n);
    *tab = pom;
}

/*  Jeśli trzeba, rozszerza tablicę 'tab' w dowolnym kierunku. Aktualizuje
    również parametry planszy w zmiennej 'wymiary'.  Jeśli tablica została
    wyzerowana, przekazuje 1, w przecwinym razie przekazuje 0.  */
int rozszerz_tab(
    char*** tab, int dol, int gora, int prawo, int lewo, Twymiary* wymiary) {

    if (dol != 0 || gora != 0 || prawo != 0 || lewo != 0) {
        rozszerz_i_wyzeruj(tab, dol + gora, prawo + lewo, *wymiary);
        wymiary -> max_w += dol;
        wymiary -> min_w -= gora;
        wymiary -> max_k += prawo;
        wymiary -> min_k -= lewo;
        wymiary -> m += dol + gora;
        wymiary -> n += prawo + lewo;
    }
    return dol != 0 || gora != 0 || prawo != 0 || lewo != 0;
}

/*  Jeśli trzeba, rozszerza planszę tak, aby można było wypisać okno.  */
void rozszerz_okno(
    char*** tab, int w, int k, Twymiary* wymiary, Twiersze* zywe_wiersze) {

    // O ile trzeba rozszerzyć.
    int dol = 0;
    int gora = 0;
    int prawo = 0;
    int lewo = 0;
    if (w + WIERSZE - 1 > wymiary -> max_w) {
        // Jeśli okno wyszłoby poza dolny zakres.
        dol = WIERSZE - ((wymiary -> max_w) + 1 - w);
    }
    if (w < wymiary -> min_w) {
        // Jeśli okno wyszłoby poza górny zakres.
        gora = (wymiary -> min_w) - w;
    }
    if (k + KOLUMNY - 1 > wymiary -> max_k) {
        // Jeśli okno wyszłoby poza prawy zakres.
        prawo = KOLUMNY - ((wymiary -> max_k) + 1 - k);
    }
    if (k < wymiary -> min_k) {
        // Jeśli okno wyszłoby poza lewy zakres.
        lewo = (wymiary -> min_k) - k;
    }
    if (rozszerz_tab(tab, dol, gora, prawo, lewo, wymiary)) {
        ustaw(tab, *wymiary, zywe_wiersze, ZYWA);
    }
}

/*  Rozszerza planszę tak, by była pewność, że podwójne otoczenie 
    (czyli sąsiedzi wszystkich sąsiadów) komórek reprezentowanych na liście 
    'wiersze' jest zaalokowane. 
    Jeśli trzeba, alokuje wiecej wierszy lub kolumn.  */
void rozszerz_otoczenie(char*** tab, Twymiary* wymiary, Twiersze* wiersze) {
    int dol, gora, prawo, lewo;
    int w, k;
    Twiersze* akt_w;
    Tkolumny* akt_k;
    // Omijamy atrapę na liście żywych wierszy.
    akt_w = wiersze -> nast_w;
    while (akt_w != NULL) {
        // Omijamy atrapę na liście żywych kolumn.
        akt_k = akt_w -> kolumny -> nast_k;
        while (akt_k != NULL) {
            dol = 0;
            gora = 0;
            prawo = 0;
            lewo = 0;
            w = akt_w -> w;
            k = akt_k -> k;
            if (w + 2 > wymiary -> max_w) {
                dol = wiecej(wymiary -> m);
            }
            if (w - 2 < wymiary -> min_w) {
                gora = wiecej(wymiary -> m);
            }
            if (k + 2 > wymiary -> max_k) {
                prawo = wiecej(wymiary -> n);
            }
            if (k - 2 < wymiary -> min_k) {
                lewo = wiecej(wymiary -> n);
            }
            rozszerz_tab(tab, dol, gora, prawo, lewo, wymiary);
            akt_k = akt_k -> nast_k;
        }
        akt_w = akt_w -> nast_w;
    }
    ustaw(tab, *wymiary, wiersze, ZYWA);
}

/*  Wypisuje okno zadane przez 'okno' o 'WIERSZE' wierszach 
    i 'KOLUMNY' kolumnach.  */
void wypisz_okno(char** tab, Tokno okno, Twymiary wymiary) {
    int w = okno.w, k = okno.k;
    int min_w = wymiary.min_w, min_k = wymiary.min_k;
    for (int i = w - min_w; i < w - min_w + WIERSZE; i++) {
        for (int j = k - min_k; j < k - min_k + KOLUMNY; j++) {
            switch (tab[i][j]) {
                case ZYWA:
                    putchar(ZYWA);
                    break;
                case MARTWA:
                    putchar(KROPKA);
                    break;
                default: assert(0); // Błąd
            }
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
                zwolnij_wiersze(&(zywe_wiersze -> nast_w));
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

/*  Sprawdza, czy komórka na polu 'stara[w][k]' powinna być żywa w następnej 
    generacji. Jeśli tak, funkcja zwraca wartość '1', jeśli nie, zwraca '0'.
    Co więcej, jeśli komórka powinna być żywa, zaznacza ten fakt na planszy, 
    aby uniknąć niepotrzebnego ponownego sprawdzania. */
int sprawdz(char** stara, int w, int k) {
    char komorka; // Rozważana komórka.
    int ile_zywych = 0;
    // Zliczanie żywych sąsiadów.
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            komorka = stara[w + i][k + j];
            if (komorka == ZYWA || komorka == ODWIEDZONA_ZYWA) {
                ile_zywych++;
            }
        }
    }
    switch (stara[w][k]) {
        case ZYWA:
            ile_zywych--; // Wtedy policzyliśmy o jedną żywą za dużo.
            stara[w][k] = ODWIEDZONA_ZYWA;
            if (ile_zywych == 2 || ile_zywych == 3) {
                return 1;
            }
            break;
        case MARTWA:
            if (ile_zywych == 3) {
                stara[w][k] = ODWIEDZONA_MARTWA;
                return 1;
            }
            break;
        default: assert(0); // Błąd.
    }
    return 0;
}

/*  Sprawdza, czy komórki znajdujące się o 'offset' powyżej lub ponizej żywych 
    komórek w wierszu 'akt_w' powinny być żywe w następnej generacji. 
    Jeśli komórka powinna być żywa, jest dopisywana za element 'z' na listę.  */
void sprawdz_otoczenie(
    char** stara, 
    int offset, 
    Twiersze* akt_w,
    Twiersze* z,
    Twymiary wymiary) {

    char komorka; // Rozważana komórka.
    // Dodajemy rozważany wiersz do listy żywych komórek w następnej generacji.
    wstaw_za_wiersz(z, (akt_w -> w) + offset, NULL);
    // Tworzymy atrapę listy kolumn w tym wierszu.
    stworz_atrape_k(&(z -> nast_w -> kolumny));
    // Inicjalizacja wskaźników pomocniczych.
    Tkolumny* kol_nowe_z = z -> nast_w -> kolumny;
    Tkolumny* akt_z = kol_nowe_z;
    int wiersz, kolumna;
    wiersz = (akt_w -> w) - wymiary.min_w + offset;
    // Omijamy atrapę na liście żywych kolumn.
    Tkolumny* akt_k = akt_w -> kolumny -> nast_k;
    while (akt_k != NULL) {
        for (int i = -1; i < 2; i++) {
            kolumna = (akt_k -> k) - wymiary.min_k + i;
            komorka = stara[wiersz][kolumna];
            if (komorka == ZYWA || komorka == MARTWA) {
                if (sprawdz(stara, wiersz, kolumna)) {
                    wstaw_za_kolumne(akt_z, (akt_k -> k) + i);
                    akt_z = akt_z -> nast_k;
                }
            }
        }
        akt_k = akt_k -> nast_k;
    }
    // Jeśli nie dodaliśmy żadnej kolumny, to zwalniamy niepotrzebny wiersz.
    if (akt_z == kol_nowe_z) {
        zwolnij_wiersze(&(z -> nast_w));
    }
}

/*  Sprawdza, czy za wskaźnik 'l' zostało coś dodane. Jeśli tak, przesuwa go
    tak, aby pokazywał na dodany element.  */
void przesun(Twiersze** l) {
    if ((*l) -> nast_w != NULL) {
        *l = (*l) -> nast_w;
    }
}

/*  Tworzy listę żywych komórek w następnej generacji i przekazuje
    wskaźnik do wyniku.  */
Twiersze* nowe_zywe(char** stara, Twiersze* zywe_wiersze, Twymiary wymiary) {
    Twiersze *gora, *srodek, *dol, *temp;
    stworz_atrape_w(&gora);
    stworz_atrape_w(&srodek);
    stworz_atrape_w(&dol);
    Twiersze* akt_g = gora;
    Twiersze* akt_s = srodek;
    Twiersze* akt_d = dol;
    // Omijamy atrapę na liście żywych wierszy.
    Twiersze* akt_w = zywe_wiersze -> nast_w;
    while (akt_w != NULL) {
        sprawdz_otoczenie(stara, -1, akt_w, akt_g, wymiary);
        sprawdz_otoczenie(stara, 0, akt_w, akt_s, wymiary);
        sprawdz_otoczenie(stara, 1, akt_w, akt_d, wymiary);
        przesun(&akt_g);
        przesun(&akt_s);
        przesun(&akt_d);
        akt_w = akt_w -> nast_w;
    }
    temp = scal_posort_w(gora, srodek);
    return scal_posort_w(temp, dol);
}

/*  Aktualizuje listę żywych wierszy w następnej generacji i odpowiednio zmienia
    pola na planszy (rozszerza ją w miarę potrzeb).  */
void nastepna_generacja(
    char*** stara, 
    Twiersze** zywe_wiersze, 
    Twymiary* wymiary) {

    Twiersze* zywe_wiersze_next = nowe_zywe(*stara, *zywe_wiersze, *wymiary);
    ustaw(stara, *wymiary, *zywe_wiersze, MARTWA);
    rozszerz_otoczenie(stara, wymiary, zywe_wiersze_next);
    zwolnij_wiersze(zywe_wiersze);
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
            rozszerz_otoczenie(tab, wymiary, *zywe_wiersze);
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
                rozszerz_okno(tab, a, b, wymiary, *zywe_wiersze);
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
    // Główna pętla programu.
    while (!wejscie(&plansza, &okno, &wymiary, &zywe_wiersze)) {
        wypisz_okno(plansza, okno, wymiary);
        wyczysc_strumien();
    }
    // Zwalnianie pamięci.
    zwolnij_2D(plansza, wymiary);
    zwolnij_wiersze(&zywe_wiersze);
    return 0;
}
