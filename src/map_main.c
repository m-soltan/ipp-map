#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla"
#pragma clang diagnostic ignored "-Wcast-qual"
#ifdef NDEBUG
#undef NDEBUG
#endif

/* Ten plik włączamy na początku i dwa razy, aby sprawdzić, czy zawiera
 * wszystko, co jest potrzebne. */
/* first include to check for */
#include "map.h"
#include "map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Possible test outcomes */
#define PASS 0
//#define FAIL 1
#define WRONG_TEST 2
#define MEM_PASS 13

/* Liczba elementów tablicy x */
#define SIZE(x) (sizeof(x) / sizeof(x)[0])

/* Sprawdzenie poprawności opisu drogi krajowej **/
#define DK(_map, _dk, _descr)                           \
  do {                                                  \
    char const *_str = getRouteDescription(_map, _dk);  \
    assert(_str);                                       \
    assert(strcmp(_str, _descr) == 0);                  \
    free((void *)_str);                                 \
  } while (0)


/* Lekka konwersja numeru drogi krajowej do napisu */
static int dk2str(unsigned dk, char *str) {
    int i = 3;
    do {
        --i;
        str[i] = dk % 10 + '0';
        dk /= 10;
    } while (dk);
    return i;
}

/** WŁAŚCIWE TESTY **/

/* Tylko tworzy i usuwa strukturę. */
static int empty(void) {
    Map *map = newMap();
    assert(map);
    deleteMap(map);
    return PASS;
}

/* Sprawdza opublikowany przykład użycia. */
static int example(void) {
    Map* m = newMap();
    assert(m);

    assert(addRoad(m, "Alinów", "Bór", 1, 2020));
    assert(addRoad(m, "Bór", "Cielińsk-Niekłańsk", 2, 2020));
    assert(addRoad(m, "Bór", "Dąb Stary", 3, 2020));
    assert(addRoad(m, "Cielińsk-Niekłańsk", "Emiliew", 4, 2020));
    assert(addRoad(m, "Dąb Stary", "Emiliew", 5, 2020));
    assert(addRoad(m, "Emiliew", "Bór", 8, 2020));
    assert(addRoad(m, "Emiliew", "Fraźnik Nowy", 3, 2020));
    assert(!repairRoad(m, "Emiliew", "Cielińsk-Niekłańsk", 2019));
    assert(repairRoad(m, "Emiliew", "Cielińsk-Niekłańsk", 2021));
    assert(!repairRoad(m, "Emiliew", "Alinów", 2020));
    assert(addRoad(m, "Fraźnik Nowy", "Grzegrzewo", 4, 2020));
    assert(addRoad(m, "Alinów", "Grzegrzewo", 10, 2020));
    assert(addRoad(m, "Homilcin", "Grzegrzewo", 5, 2020));
    assert(addRoad(m, "Fraźnik Nowy", "Cielińsk-Niekłańsk", 2, 2020));
    assert(!addRoad(m, "Fraźnik Nowy", "Cielińsk-Niekłańsk", 2, 2020));
    assert(!addRoad(m, "Cielińsk-Niekłańsk", "Fraźnik Nowy", 2, 2020));
    assert(!repairRoad(m, "Emiliew", "Bór", 2018));
    assert(repairRoad(m, "Emiliew", "Cielińsk-Niekłańsk", 2021));
    assert(repairRoad(m, "Emiliew", "Fraźnik Nowy", 2023));
    assert(addRoad(m, "Homilcin", "Cielińsk-Niekłańsk", 3, 2020));
    assert(newRoute(m, 10, "Alinów", "Emiliew"));

    DK(m, 10, "10;Alinów;1;2020;Bór;2;2020;Cielińsk-Niekłańsk;4;2021;Emiliew");

    assert(extendRoute(m, 10, "Homilcin"));

    DK(m, 10, "10;Alinów;1;2020;Bór;2;2020;Cielińsk-Niekłańsk;4;2021;Emiliew"
              ";3;2023;Fraźnik Nowy;4;2020;Grzegrzewo;5;2020;Homilcin");

    deleteMap(m);
    return PASS;
}

/* Testuje sprawdzanie poprawność parametrów funkcji. */
static int params(void) {
    Map *m = newMap();
    assert(m);

    assert(!addRoad(m, "",     "def",  1, 2019));
    assert(!addRoad(m, "\x01", "def",  1, 2019));
    assert(!addRoad(m, "\x1f", "def",  1, 2019));
    assert(!addRoad(m, ";",    "def",  1, 2019));
    assert(!addRoad(m, "abc",  "",     1, 2019));
    assert(!addRoad(m, "abc",  "\x01", 1, 2019));
    assert(!addRoad(m, "abc",  "\x1f", 1, 2019));
    assert(!addRoad(m, "abc",  ";",    1, 2019));
    assert(!addRoad(m, "abc",  "def",  0, 2019));
    assert(!addRoad(m, "abc",  "abc",  0, 2019));
    assert(!addRoad(m, "abc",  "def",  1, 0));

    assert(addRoad(m, "abc", "def", 1, -1));
    assert(!repairRoad(m, "",     "def",  2));
    assert(!repairRoad(m, "\x01", "def",  2));
    assert(!repairRoad(m, "\x1f", "def",  2));
    assert(!repairRoad(m, ";",    "def",  2));
    assert(!repairRoad(m, "abc",  "",     2));
    assert(!repairRoad(m, "abc",  "\x01", 2));
    assert(!repairRoad(m, "abc",  "\xff", 2));
    assert(!repairRoad(m, "abc",  ";",    2));
    assert(!repairRoad(m, "abc",  "def",  0));

    assert(!newRoute(m, 0,    "abc",  "def"));
    assert(!newRoute(m, 1000, "abc",  "def"));
    assert(!newRoute(m, 1,    "",     "def"));
    assert(!newRoute(m, 1,    "\x01", "def"));
    assert(!newRoute(m, 1,    "\x1f", "def"));
    assert(!newRoute(m, 1,    "abc",  ""));
    assert(!newRoute(m, 1,    "abc",  "\x01"));
    assert(!newRoute(m, 1,    "abc",  "\x1f"));
    assert(!newRoute(m, 1,    "abc",  ";"));
    assert(!newRoute(m, 1,    "abc",  "abc"));

    assert(newRoute(m, 999, "abc",  "def"));
    assert(addRoad(m, "def", "ghi", 1, 1));
    assert(!extendRoute(m,    0, "ghi"));
    assert(!extendRoute(m, 1000, "ghi"));
    assert(!extendRoute(m,  999, ""));
    assert(!extendRoute(m,  999, "\x01"));
    assert(!extendRoute(m,  999, "\x1f"));
    assert(!extendRoute(m,  999, ";"));
    assert(!extendRoute(m,  999, "abc"));
    assert(!extendRoute(m,  999, "def"));

    assert(!removeRoad(m, "",     "def"));
    assert(!removeRoad(m, "\x01", "def"));
    assert(!removeRoad(m, "\x1f", "def"));
    assert(!removeRoad(m, ";",    "def"));
    assert(!removeRoad(m, "abc",  ""));
    assert(!removeRoad(m, "abc",  "\x01"));
    assert(!removeRoad(m, "abc",  "\x1f"));
    assert(!removeRoad(m, "abc",  ";"));

    char const* d;

    d = getRouteDescription(m, 0);
    assert(d == NULL || strcmp(d, "") == 0);
    free((void *)d);

    d = getRouteDescription(m, 1000);
    assert(d == NULL || strcmp(d, "") == 0);
    free((void *)d);

    deleteMap(m);
    return PASS;
}

/* Wykonanie niektórych funkcji w tym teście może się nie udać z powodu braku
 * pamięci, ale nie powinno to skutkować załamaniem wykonywania programu. */
static int memory(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "a", "b", 1, 1));
    assert(addRoad(m, "b", "c", 1, 1));
    assert(newRoute(m, 1, "a", "b"));
    assert(extendRoute(m, 1, "c"));

    /* Alokujemy całą dostępną pamięć. */
    for (size_t s = 1024 * 1024 * 1024; s >= 1; s /= 2) {
        void *p;
        do {
            p = malloc(s);
        } while (p);
    }

    while (newMap());

    for (unsigned dk = 2; dk <= 999; ++dk) {
        if (newRoute(m, dk, "a", "c")) {
            char const *str = getRouteDescription(m, dk);
            if (str) {
                static char rd[] = "000;a;1;1;b;1;1;c";
                int i = dk2str(dk, rd);
                assert(strcmp(str, rd + i) == 0);
            }
        }
        else {
            char const *str = getRouteDescription(m, dk);
            if (str) {
                assert(strcmp(str, "") == 0);
            }
        }
    }

    char city1[2], city2[2];
    city1[0] = 'c';
    city2[0] = 'd';
    city1[1] = city2[1] = '\0';

    for (; city2[0] <= 'z'; city1[0] = city2[0], city2[0]++)
        if (addRoad(m, city1, city2, 1, 1))
            if (extendRoute(m, 1, city2) == false)
                break;

    /* To jest test alokacji pamięci – nie zwalniamy jej. */
    return MEM_PASS;
}

/* Test utworzony na podstawie pytania z forum. */
static int forum1(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "poczatek", "a", 10, 2019));
    assert(addRoad(m, "a", "c", 10, 2019));
    assert(addRoad(m, "c", "koniec", 10, 2019));

    assert(newRoute(m, 1, "poczatek", "koniec"));

    assert(addRoad(m, "poczatek", "koniec", 1, 2019));
    assert(addRoad(m, "a", "b", 100, 2019));
    assert(addRoad(m, "b", "c", 100, 2019));
    assert(addRoad(m, "a", "koniec", 1, 2019));

    assert(removeRoad(m, "a", "c"));

    DK(m, 1, "1;poczatek;10;2019;a;100;2019;b;100;2019;c;10;2019;koniec");

    deleteMap(m);
    return PASS;
}

/* Test utworzony na podstawie pytania z forum. */
static int forum2(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "A", "D", 10, 2019));
    assert(addRoad(m, "A", "B", 1, 2019));
    assert(addRoad(m, "B", "C", 1, 2019));
    assert(addRoad(m, "C", "D", 1, 2019));
    assert(newRoute(m, 1, "A", "D"));
    assert(!removeRoad(m, "B", "C"));

    deleteMap(m);
    return PASS;
}

/* Test utworzony na podstawie pytania z forum z dodaną dodatkową drogą krajową
 * na skróty */
static int forum3(void) {
    Map* m = newMap();
    assert(m);

    assert(addRoad(m, "A", "B", 1, 2000));
    assert(addRoad(m, "B", "C", 1, 2000));
    assert(addRoad(m, "C", "D", 1, 2000));
    assert(addRoad(m, "C", "E", 1, 2000));
    assert(addRoad(m, "E", "F", 1, 2000));
    assert(addRoad(m, "E", "D", 7, 2000));

    assert(newRoute(m, 10, "A", "D"));
    assert(extendRoute(m, 10, "F"));

    DK(m, 10, "10;A;1;2000;B;1;2000;C;1;2000;D;7;2000;E;1;2000;F");

    assert(newRoute(m, 11, "A", "F"));

    DK(m, 11, "11;A;1;2000;B;1;2000;C;1;2000;E;1;2000;F");

    deleteMap(m);
    return PASS;
}

/* Funkcja getRouteDescription musi przy każdym wywołaniu zaalokować pamięć. */
static int description1(void) {
    Map* m = newMap();
    assert(m);

    assert(addRoad(m, "Aaa", "Bbb", 48, 1998));
    assert(addRoad(m, "Ccc", "Ddd", 84, 1994));
    assert(newRoute(m, 8, "Aaa", "Bbb"));
    assert(newRoute(m, 4, "Ccc", "Ddd"));

    char const *d8 = getRouteDescription(m, 8);
    char const *d4 = getRouteDescription(m, 4);
    assert(d8);
    assert(d4);
    assert(strcmp(d8, "8;Aaa;48;1998;Bbb") == 0);
    assert(strcmp(d4, "4;Ccc;84;1994;Ddd") == 0);
    free((void *)d8);
    free((void *)d4);

    deleteMap(m);
    return PASS;
}

/* Wynikiem funkcji getRouteDescription przy braku
 * drogi krajowej ma być pusty napis. */
static int description2(void) {
    Map* m = newMap();
    assert(m);

    for (unsigned dk = 1; dk <= 999; ++dk)
        DK(m, dk, "");

    deleteMap(m);
    return PASS;
}

/* Odcinki dróg nie będące fragmentami dróg krajowych można zawsze usunąć. */
static int remove1(void) {
    Map* m = newMap();
    assert(m);

    assert(addRoad(m, "a", "b", 1, 1));
    assert(addRoad(m, "a", "c", 1, 1));
    assert(addRoad(m, "a", "d", 1, 1));
    assert(addRoad(m, "a", "e", 1, 1));
    assert(addRoad(m, "b", "c", 1, 1));
    assert(addRoad(m, "b", "d", 1, 1));
    assert(addRoad(m, "b", "e", 1, 1));
    assert(addRoad(m, "c", "d", 1, 1));
    assert(addRoad(m, "c", "e", 1, 1));
    assert(addRoad(m, "d", "e", 1, 1));

    assert(removeRoad(m, "a", "b"));
    assert(removeRoad(m, "a", "c"));
    assert(removeRoad(m, "a", "d"));
    assert(removeRoad(m, "a", "e"));
    assert(removeRoad(m, "b", "c"));
    assert(removeRoad(m, "b", "d"));
    assert(removeRoad(m, "b", "e"));
    assert(removeRoad(m, "c", "d"));
    assert(removeRoad(m, "c", "e"));
    assert(removeRoad(m, "d", "e"));

    deleteMap(m);
    return PASS;
}

/* Nie można utworzyć dwa razy drogi krajowej o tym samym numerze. */
static int route1(void) {
    Map* m = newMap();
    assert(m);

    assert(addRoad(m, "Ćmielów", "Żagań", 1, 1));
    assert(newRoute(m, 1, "Ćmielów", "Żagań"));
    assert(!newRoute(m, 1, "Ćmielów", "Żagań"));

    deleteMap(m);
    return PASS;
}

/* Tworzy 999 dróg krajowych. */
static int route2(void) {
    Map* m = newMap();
    assert(m);

    assert(addRoad(m, "a", "b", 1, 1));
    assert(addRoad(m, "b", "c", 1, 1));

    for (unsigned dk = 1; dk <= 999; ++dk) {
        assert(newRoute(m, dk, "a", "c"));
        char const *str = getRouteDescription(m, dk);
        assert(str);
        static char rd[] = "000;a;1;1;b;1;1;c";
        int i = dk2str(dk, rd);
        assert(strcmp(str, rd + i) == 0);
        free((void *)str);
    }

    deleteMap(m);
    return PASS;
}

/* Sprawdza poprawność tworzenia dróg krajowych z uwzględnieniem roku budowy
 * lub ostatniego remontu, gdy istnieje do wyboru kilka wariantów przebiegu
 * drogi. */
static int route3(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "X", "A", 10, 10));
    assert(removeRoad(m, "X", "A"));
    assert(addRoad(m, "X", "A", 10, 10));
    assert(addRoad(m, "A", "C1", 1, 11));
    assert(addRoad(m, "A", "D1", 1, 10));
    assert(addRoad(m, "C1", "B1", 1, 10));
    assert(addRoad(m, "D1", "B1", 1, 10));
    assert(addRoad(m, "A", "C2", 1, 9));
    assert(addRoad(m, "A", "D2", 1, 10));
    assert(addRoad(m, "C2", "B2", 1, 10));
    assert(addRoad(m, "D2", "B2", 1, 10));
    assert(addRoad(m, "A", "C3", 1, 11));
    assert(addRoad(m, "A", "D3", 1, 10));
    assert(addRoad(m, "C3", "B3", 1, 10));
    assert(addRoad(m, "D3", "B3", 1, 12));
    assert(newRoute(m, 10, "X", "A"));
    assert(newRoute(m, 11, "X", "A"));
    assert(newRoute(m, 12, "X", "A"));

    DK(m, 10, "10;X;10;10;A");
    DK(m, 11, "11;X;10;10;A");
    DK(m, 12, "12;X;10;10;A");

    assert(!extendRoute(m, 10, "B1"));
    assert(extendRoute(m, 11, "B2"));
    assert(!extendRoute(m, 12, "B3"));

    DK(m, 10, "10;X;10;10;A");
    DK(m, 11, "11;X;10;10;A;1;10;D2;1;10;B2");
    DK(m, 12, "12;X;10;10;A");

    assert(addRoad(m, "X", "Y", 10, 10));
    assert(addRoad(m, "A", "Y", 10, 10));
    assert(removeRoad(m, "A", "X"));

    DK(m, 10, "10;X;10;10;Y;10;10;A");
    DK(m, 11, "11;X;10;10;Y;10;10;A;1;10;D2;1;10;B2");
    DK(m, 12, "12;X;10;10;Y;10;10;A");

    deleteMap(m);
    return PASS;
}

/* Tworzy dwie spójne składowe. Jedną X0–X1–X2–…–X100. Drugą X101–X102. Tworzy
 * drogę krajową z X0 do X100. Następnie sprawdza różne konfiguracje poprawnych
 * i niepoprawnych wywołań repairRoad oraz niepoprawnych wywołań addRoad
 * i extendRoute. */
static int route4(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "X0", "X1", 1, 1));
    assert(addRoad(m, "X1", "X2", 1, 1));
    assert(addRoad(m, "X2", "X3", 1, 1));
    assert(addRoad(m, "X3", "X4", 1, 1));
    assert(addRoad(m, "X4", "X5", 1, 1));
    assert(addRoad(m, "X5", "X6", 1, 1));
    assert(addRoad(m, "X6", "X7", 1, 1));
    assert(addRoad(m, "X7", "X8", 1, 1));
    assert(addRoad(m, "X8", "X9", 1, 1));
    assert(addRoad(m, "X9", "X10", 1, 1));
    assert(addRoad(m, "X10", "X11", 1, 1));
    assert(addRoad(m, "X11", "X12", 1, 1));
    assert(addRoad(m, "X12", "X13", 1, 1));
    assert(addRoad(m, "X13", "X14", 1, 1));
    assert(addRoad(m, "X14", "X15", 1, 1));
    assert(addRoad(m, "X15", "X16", 1, 1));
    assert(addRoad(m, "X16", "X17", 1, 1));
    assert(addRoad(m, "X17", "X18", 1, 1));
    assert(addRoad(m, "X18", "X19", 1, 1));
    assert(addRoad(m, "X19", "X20", 1, 1));
    assert(addRoad(m, "X20", "X21", 1, 1));
    assert(addRoad(m, "X21", "X22", 1, 1));
    assert(addRoad(m, "X22", "X23", 1, 1));
    assert(addRoad(m, "X23", "X24", 1, 1));
    assert(addRoad(m, "X24", "X25", 1, 1));
    assert(addRoad(m, "X25", "X26", 1, 1));
    assert(addRoad(m, "X26", "X27", 1, 1));
    assert(addRoad(m, "X27", "X28", 1, 1));
    assert(addRoad(m, "X28", "X29", 1, 1));
    assert(addRoad(m, "X29", "X30", 1, 1));
    assert(addRoad(m, "X30", "X31", 1, 1));
    assert(addRoad(m, "X31", "X32", 1, 1));
    assert(addRoad(m, "X32", "X33", 1, 1));
    assert(addRoad(m, "X33", "X34", 1, 1));
    assert(addRoad(m, "X34", "X35", 1, 1));
    assert(addRoad(m, "X35", "X36", 1, 1));
    assert(addRoad(m, "X36", "X37", 1, 1));
    assert(addRoad(m, "X37", "X38", 1, 1));
    assert(addRoad(m, "X38", "X39", 1, 1));
    assert(addRoad(m, "X39", "X40", 1, 1));
    assert(addRoad(m, "X40", "X41", 1, 1));
    assert(addRoad(m, "X41", "X42", 1, 1));
    assert(addRoad(m, "X42", "X43", 1, 1));
    assert(addRoad(m, "X43", "X44", 1, 1));
    assert(addRoad(m, "X44", "X45", 1, 1));
    assert(addRoad(m, "X45", "X46", 1, 1));
    assert(addRoad(m, "X46", "X47", 1, 1));
    assert(addRoad(m, "X47", "X48", 1, 1));
    assert(addRoad(m, "X48", "X49", 1, 1));
    assert(addRoad(m, "X49", "X50", 1, 1));
    assert(addRoad(m, "X50", "X51", 1, 1));
    assert(addRoad(m, "X51", "X52", 1, 1));
    assert(addRoad(m, "X52", "X53", 1, 1));
    assert(addRoad(m, "X53", "X54", 1, 1));
    assert(addRoad(m, "X54", "X55", 1, 1));
    assert(addRoad(m, "X55", "X56", 1, 1));
    assert(addRoad(m, "X56", "X57", 1, 1));
    assert(addRoad(m, "X57", "X58", 1, 1));
    assert(addRoad(m, "X58", "X59", 1, 1));
    assert(addRoad(m, "X59", "X60", 1, 1));
    assert(addRoad(m, "X60", "X61", 1, 1));
    assert(addRoad(m, "X61", "X62", 1, 1));
    assert(addRoad(m, "X62", "X63", 1, 1));
    assert(addRoad(m, "X63", "X64", 1, 1));
    assert(addRoad(m, "X64", "X65", 1, 1));
    assert(addRoad(m, "X65", "X66", 1, 1));
    assert(addRoad(m, "X66", "X67", 1, 1));
    assert(addRoad(m, "X67", "X68", 1, 1));
    assert(addRoad(m, "X68", "X69", 1, 1));
    assert(addRoad(m, "X69", "X70", 1, 1));
    assert(addRoad(m, "X70", "X71", 1, 1));
    assert(addRoad(m, "X71", "X72", 1, 1));
    assert(addRoad(m, "X72", "X73", 1, 1));
    assert(addRoad(m, "X73", "X74", 1, 1));
    assert(addRoad(m, "X74", "X75", 1, 1));
    assert(addRoad(m, "X75", "X76", 1, 1));
    assert(addRoad(m, "X76", "X77", 1, 1));
    assert(addRoad(m, "X77", "X78", 1, 1));
    assert(addRoad(m, "X78", "X79", 1, 1));
    assert(addRoad(m, "X79", "X80", 1, 1));
    assert(addRoad(m, "X80", "X81", 1, 1));
    assert(addRoad(m, "X81", "X82", 1, 1));
    assert(addRoad(m, "X82", "X83", 1, 1));
    assert(addRoad(m, "X83", "X84", 1, 1));
    assert(addRoad(m, "X84", "X85", 1, 1));
    assert(addRoad(m, "X85", "X86", 1, 1));
    assert(addRoad(m, "X86", "X87", 1, 1));
    assert(addRoad(m, "X87", "X88", 1, 1));
    assert(addRoad(m, "X88", "X89", 1, 1));
    assert(addRoad(m, "X89", "X90", 1, 1));
    assert(addRoad(m, "X90", "X91", 1, 1));
    assert(addRoad(m, "X91", "X92", 1, 1));
    assert(addRoad(m, "X92", "X93", 1, 1));
    assert(addRoad(m, "X93", "X94", 1, 1));
    assert(addRoad(m, "X94", "X95", 1, 1));
    assert(addRoad(m, "X95", "X96", 1, 1));
    assert(addRoad(m, "X96", "X97", 1, 1));
    assert(addRoad(m, "X97", "X98", 1, 1));
    assert(addRoad(m, "X98", "X99", 1, 1));
    assert(addRoad(m, "X99", "X100", 1, 1));
    assert(addRoad(m, "X101", "X102", 1, 1));
    assert(newRoute(m, 10, "X0", "X100"));

    DK(m, 10, "10;X0;1;1;X1;1;1;X2;1;1;X3;1;1;X4;1;1;X5;1;1;X6;1;1;X7;1;1;X8;"
              "1;1;X9;1;1;X10;1;1;X11;1;1;X12;1;1;X13;1;1;X14;1;1;X15;1;1;X16"
              ";1;1;X17;1;1;X18;1;1;X19;1;1;X20;1;1;X21;1;1;X22;1;1;X23;1;1;X24"
              ";1;1;X25;1;1;X26;1;1;X27;1;1;X28;1;1;X29;1;1;X30;1;1;X31;1;1;X32"
              ";1;1;X33;1;1;X34;1;1;X35;1;1;X36;1;1;X37;1;1;X38;1;1;X39;1;1;X40;"
              "1;1;X41;1;1;X42;1;1;X43;1;1;X44;1;1;X45;1;1;X46;1;1;X47;1;1;X48;"
              "1;1;X49;1;1;X50;1;1;X51;1;1;X52;1;1;X53;1;1;X54;1;1;X55;1;1;X56;"
              "1;1;X57;1;1;X58;1;1;X59;1;1;X60;1;1;X61;1;1;X62;1;1;X63;1;1;X64;"
              "1;1;X65;1;1;X66;1;1;X67;1;1;X68;1;1;X69;1;1;X70;1;1;X71;1;1;X72;"
              "1;1;X73;1;1;X74;1;1;X75;1;1;X76;1;1;X77;1;1;X78;1;1;X79;1;1;X80;"
              "1;1;X81;1;1;X82;1;1;X83;1;1;X84;1;1;X85;1;1;X86;1;1;X87;1;1;X88;"
              "1;1;X89;1;1;X90;1;1;X91;1;1;X92;1;1;X93;1;1;X94;1;1;X95;1;1;X96;"
              "1;1;X97;1;1;X98;1;1;X99;1;1;X100");

    assert(!addRoad(m, "X0", "X1", 1, 1));
    assert(!addRoad(m, "X1", "X2", 1, 1));
    assert(!addRoad(m, "X2", "X3", 1, 1));
    assert(!addRoad(m, "X3", "X4", 1, 1));
    assert(!addRoad(m, "X4", "X5", 1, 1));
    assert(!addRoad(m, "X5", "X6", 1, 1));
    assert(!addRoad(m, "X6", "X7", 1, 1));
    assert(!addRoad(m, "X7", "X8", 1, 1));
    assert(!addRoad(m, "X8", "X9", 1, 1));
    assert(!addRoad(m, "X9", "X10", 1, 1));
    assert(!addRoad(m, "X10", "X11", 1, 1));
    assert(!addRoad(m, "X11", "X12", 1, 1));
    assert(!addRoad(m, "X12", "X13", 1, 1));
    assert(!addRoad(m, "X13", "X14", 1, 1));
    assert(!addRoad(m, "X14", "X15", 1, 1));
    assert(!addRoad(m, "X15", "X16", 1, 1));
    assert(!addRoad(m, "X16", "X17", 1, 1));
    assert(!addRoad(m, "X17", "X18", 1, 1));
    assert(!addRoad(m, "X18", "X19", 1, 1));
    assert(!addRoad(m, "X19", "X20", 1, 1));
    assert(!addRoad(m, "X20", "X21", 1, 1));
    assert(!addRoad(m, "X21", "X22", 1, 1));
    assert(!addRoad(m, "X22", "X23", 1, 1));
    assert(!addRoad(m, "X23", "X24", 1, 1));
    assert(!addRoad(m, "X24", "X25", 1, 1));
    assert(!addRoad(m, "X25", "X26", 1, 1));
    assert(!addRoad(m, "X26", "X27", 1, 1));
    assert(!addRoad(m, "X27", "X28", 1, 1));
    assert(!addRoad(m, "X28", "X29", 1, 1));
    assert(!addRoad(m, "X29", "X30", 1, 1));
    assert(!addRoad(m, "X30", "X31", 1, 1));
    assert(!addRoad(m, "X31", "X32", 1, 1));
    assert(!addRoad(m, "X32", "X33", 1, 1));
    assert(!addRoad(m, "X33", "X34", 1, 1));
    assert(!addRoad(m, "X34", "X35", 1, 1));
    assert(!addRoad(m, "X35", "X36", 1, 1));
    assert(!addRoad(m, "X36", "X37", 1, 1));
    assert(!addRoad(m, "X37", "X38", 1, 1));
    assert(!addRoad(m, "X38", "X39", 1, 1));
    assert(!addRoad(m, "X39", "X40", 1, 1));
    assert(!addRoad(m, "X40", "X41", 1, 1));
    assert(!addRoad(m, "X41", "X42", 1, 1));
    assert(!addRoad(m, "X42", "X43", 1, 1));
    assert(!addRoad(m, "X43", "X44", 1, 1));
    assert(!addRoad(m, "X44", "X45", 1, 1));
    assert(!addRoad(m, "X45", "X46", 1, 1));
    assert(!addRoad(m, "X46", "X47", 1, 1));
    assert(!addRoad(m, "X47", "X48", 1, 1));
    assert(!addRoad(m, "X48", "X49", 1, 1));
    assert(!addRoad(m, "X49", "X50", 1, 1));
    assert(!addRoad(m, "X50", "X51", 1, 1));
    assert(!addRoad(m, "X51", "X52", 1, 1));
    assert(!addRoad(m, "X52", "X53", 1, 1));
    assert(!addRoad(m, "X53", "X54", 1, 1));
    assert(!addRoad(m, "X54", "X55", 1, 1));
    assert(!addRoad(m, "X55", "X56", 1, 1));
    assert(!addRoad(m, "X56", "X57", 1, 1));
    assert(!addRoad(m, "X57", "X58", 1, 1));
    assert(!addRoad(m, "X58", "X59", 1, 1));
    assert(!addRoad(m, "X59", "X60", 1, 1));
    assert(!addRoad(m, "X60", "X61", 1, 1));
    assert(!addRoad(m, "X61", "X62", 1, 1));
    assert(!addRoad(m, "X62", "X63", 1, 1));
    assert(!addRoad(m, "X63", "X64", 1, 1));
    assert(!addRoad(m, "X64", "X65", 1, 1));
    assert(!addRoad(m, "X65", "X66", 1, 1));
    assert(!addRoad(m, "X66", "X67", 1, 1));
    assert(!addRoad(m, "X67", "X68", 1, 1));
    assert(!addRoad(m, "X68", "X69", 1, 1));
    assert(!addRoad(m, "X69", "X70", 1, 1));
    assert(!addRoad(m, "X70", "X71", 1, 1));
    assert(!addRoad(m, "X71", "X72", 1, 1));
    assert(!addRoad(m, "X72", "X73", 1, 1));
    assert(!addRoad(m, "X73", "X74", 1, 1));
    assert(!addRoad(m, "X74", "X75", 1, 1));
    assert(!addRoad(m, "X75", "X76", 1, 1));
    assert(!addRoad(m, "X76", "X77", 1, 1));
    assert(!addRoad(m, "X77", "X78", 1, 1));
    assert(!addRoad(m, "X78", "X79", 1, 1));
    assert(!addRoad(m, "X79", "X80", 1, 1));
    assert(!addRoad(m, "X80", "X81", 1, 1));
    assert(!addRoad(m, "X81", "X82", 1, 1));
    assert(!addRoad(m, "X82", "X83", 1, 1));
    assert(!addRoad(m, "X83", "X84", 1, 1));
    assert(!addRoad(m, "X84", "X85", 1, 1));
    assert(!addRoad(m, "X85", "X86", 1, 1));
    assert(!addRoad(m, "X86", "X87", 1, 1));
    assert(!addRoad(m, "X87", "X88", 1, 1));
    assert(!addRoad(m, "X88", "X89", 1, 1));
    assert(!addRoad(m, "X89", "X90", 1, 1));
    assert(!addRoad(m, "X90", "X91", 1, 1));
    assert(!addRoad(m, "X91", "X92", 1, 1));
    assert(!addRoad(m, "X92", "X93", 1, 1));
    assert(!addRoad(m, "X93", "X94", 1, 1));
    assert(!addRoad(m, "X94", "X95", 1, 1));
    assert(!addRoad(m, "X95", "X96", 1, 1));
    assert(!addRoad(m, "X96", "X97", 1, 1));
    assert(!addRoad(m, "X97", "X98", 1, 1));
    assert(!addRoad(m, "X98", "X99", 1, 1));
    assert(!addRoad(m, "X99", "X100", 1, 1));
    assert(repairRoad(m, "X0", "X1", 1));
    assert(repairRoad(m, "X1", "X2", 1));
    assert(repairRoad(m, "X2", "X3", 1));
    assert(repairRoad(m, "X3", "X4", 1));
    assert(repairRoad(m, "X4", "X5", 1));
    assert(repairRoad(m, "X5", "X6", 1));
    assert(repairRoad(m, "X6", "X7", 1));
    assert(repairRoad(m, "X7", "X8", 1));
    assert(repairRoad(m, "X8", "X9", 1));
    assert(repairRoad(m, "X9", "X10", 1));
    assert(repairRoad(m, "X10", "X11", 1));
    assert(repairRoad(m, "X11", "X12", 1));
    assert(repairRoad(m, "X12", "X13", 1));
    assert(repairRoad(m, "X13", "X14", 1));
    assert(repairRoad(m, "X14", "X15", 1));
    assert(repairRoad(m, "X15", "X16", 1));
    assert(repairRoad(m, "X16", "X17", 1));
    assert(repairRoad(m, "X17", "X18", 1));
    assert(repairRoad(m, "X18", "X19", 1));
    assert(repairRoad(m, "X19", "X20", 1));
    assert(repairRoad(m, "X20", "X21", 1));
    assert(repairRoad(m, "X21", "X22", 1));
    assert(repairRoad(m, "X22", "X23", 1));
    assert(repairRoad(m, "X23", "X24", 1));
    assert(repairRoad(m, "X24", "X25", 1));
    assert(repairRoad(m, "X25", "X26", 1));
    assert(repairRoad(m, "X26", "X27", 1));
    assert(repairRoad(m, "X27", "X28", 1));
    assert(repairRoad(m, "X28", "X29", 1));
    assert(repairRoad(m, "X29", "X30", 1));
    assert(repairRoad(m, "X30", "X31", 1));
    assert(repairRoad(m, "X31", "X32", 1));
    assert(repairRoad(m, "X32", "X33", 1));
    assert(repairRoad(m, "X33", "X34", 1));
    assert(repairRoad(m, "X34", "X35", 1));
    assert(repairRoad(m, "X35", "X36", 1));
    assert(repairRoad(m, "X36", "X37", 1));
    assert(repairRoad(m, "X37", "X38", 1));
    assert(repairRoad(m, "X38", "X39", 1));
    assert(repairRoad(m, "X39", "X40", 1));
    assert(repairRoad(m, "X40", "X41", 1));
    assert(repairRoad(m, "X41", "X42", 1));
    assert(repairRoad(m, "X42", "X43", 1));
    assert(repairRoad(m, "X43", "X44", 1));
    assert(repairRoad(m, "X44", "X45", 1));
    assert(repairRoad(m, "X45", "X46", 1));
    assert(repairRoad(m, "X46", "X47", 1));
    assert(repairRoad(m, "X47", "X48", 1));
    assert(repairRoad(m, "X48", "X49", 1));
    assert(repairRoad(m, "X49", "X50", 1));
    assert(repairRoad(m, "X50", "X51", 1));
    assert(repairRoad(m, "X51", "X52", 1));
    assert(repairRoad(m, "X52", "X53", 1));
    assert(repairRoad(m, "X53", "X54", 1));
    assert(repairRoad(m, "X54", "X55", 1));
    assert(repairRoad(m, "X55", "X56", 1));
    assert(repairRoad(m, "X56", "X57", 1));
    assert(repairRoad(m, "X57", "X58", 1));
    assert(repairRoad(m, "X58", "X59", 1));
    assert(repairRoad(m, "X59", "X60", 1));
    assert(repairRoad(m, "X60", "X61", 1));
    assert(repairRoad(m, "X61", "X62", 1));
    assert(repairRoad(m, "X62", "X63", 1));
    assert(repairRoad(m, "X63", "X64", 1));
    assert(repairRoad(m, "X64", "X65", 1));
    assert(repairRoad(m, "X65", "X66", 1));
    assert(repairRoad(m, "X66", "X67", 1));
    assert(repairRoad(m, "X67", "X68", 1));
    assert(repairRoad(m, "X68", "X69", 1));
    assert(repairRoad(m, "X69", "X70", 1));
    assert(repairRoad(m, "X70", "X71", 1));
    assert(repairRoad(m, "X71", "X72", 1));
    assert(repairRoad(m, "X72", "X73", 1));
    assert(repairRoad(m, "X73", "X74", 1));
    assert(repairRoad(m, "X74", "X75", 1));
    assert(repairRoad(m, "X75", "X76", 1));
    assert(repairRoad(m, "X76", "X77", 1));
    assert(repairRoad(m, "X77", "X78", 1));
    assert(repairRoad(m, "X78", "X79", 1));
    assert(repairRoad(m, "X79", "X80", 1));
    assert(repairRoad(m, "X80", "X81", 1));
    assert(repairRoad(m, "X81", "X82", 1));
    assert(repairRoad(m, "X82", "X83", 1));
    assert(repairRoad(m, "X83", "X84", 1));
    assert(repairRoad(m, "X84", "X85", 1));
    assert(repairRoad(m, "X85", "X86", 1));
    assert(repairRoad(m, "X86", "X87", 1));
    assert(repairRoad(m, "X87", "X88", 1));
    assert(repairRoad(m, "X88", "X89", 1));
    assert(repairRoad(m, "X89", "X90", 1));
    assert(repairRoad(m, "X90", "X91", 1));
    assert(repairRoad(m, "X91", "X92", 1));
    assert(repairRoad(m, "X92", "X93", 1));
    assert(repairRoad(m, "X93", "X94", 1));
    assert(repairRoad(m, "X94", "X95", 1));
    assert(repairRoad(m, "X95", "X96", 1));
    assert(repairRoad(m, "X96", "X97", 1));
    assert(repairRoad(m, "X97", "X98", 1));
    assert(repairRoad(m, "X98", "X99", 1));
    assert(repairRoad(m, "X99", "X100", 1));
    assert(repairRoad(m, "X0", "X1", 2));
    assert(repairRoad(m, "X1", "X2", 2));
    assert(repairRoad(m, "X2", "X3", 2));
    assert(repairRoad(m, "X3", "X4", 2));
    assert(repairRoad(m, "X4", "X5", 2));
    assert(repairRoad(m, "X5", "X6", 2));
    assert(repairRoad(m, "X6", "X7", 2));
    assert(repairRoad(m, "X7", "X8", 2));
    assert(repairRoad(m, "X8", "X9", 2));
    assert(repairRoad(m, "X9", "X10", 2));
    assert(repairRoad(m, "X10", "X11", 2));
    assert(repairRoad(m, "X11", "X12", 2));
    assert(repairRoad(m, "X12", "X13", 2));
    assert(repairRoad(m, "X13", "X14", 2));
    assert(repairRoad(m, "X14", "X15", 2));
    assert(repairRoad(m, "X15", "X16", 2));
    assert(repairRoad(m, "X16", "X17", 2));
    assert(repairRoad(m, "X17", "X18", 2));
    assert(repairRoad(m, "X18", "X19", 2));
    assert(repairRoad(m, "X19", "X20", 2));
    assert(repairRoad(m, "X20", "X21", 2));
    assert(repairRoad(m, "X21", "X22", 2));
    assert(repairRoad(m, "X22", "X23", 2));
    assert(repairRoad(m, "X23", "X24", 2));
    assert(repairRoad(m, "X24", "X25", 2));
    assert(repairRoad(m, "X25", "X26", 2));
    assert(repairRoad(m, "X26", "X27", 2));
    assert(repairRoad(m, "X27", "X28", 2));
    assert(repairRoad(m, "X28", "X29", 2));
    assert(repairRoad(m, "X29", "X30", 2));
    assert(repairRoad(m, "X30", "X31", 2));
    assert(repairRoad(m, "X31", "X32", 2));
    assert(repairRoad(m, "X32", "X33", 2));
    assert(repairRoad(m, "X33", "X34", 2));
    assert(repairRoad(m, "X34", "X35", 2));
    assert(repairRoad(m, "X35", "X36", 2));
    assert(repairRoad(m, "X36", "X37", 2));
    assert(repairRoad(m, "X37", "X38", 2));
    assert(repairRoad(m, "X38", "X39", 2));
    assert(repairRoad(m, "X39", "X40", 2));
    assert(repairRoad(m, "X40", "X41", 2));
    assert(repairRoad(m, "X41", "X42", 2));
    assert(repairRoad(m, "X42", "X43", 2));
    assert(repairRoad(m, "X43", "X44", 2));
    assert(repairRoad(m, "X44", "X45", 2));
    assert(repairRoad(m, "X45", "X46", 2));
    assert(repairRoad(m, "X46", "X47", 2));
    assert(repairRoad(m, "X47", "X48", 2));
    assert(repairRoad(m, "X48", "X49", 2));
    assert(repairRoad(m, "X49", "X50", 2));
    assert(repairRoad(m, "X50", "X51", 2));
    assert(repairRoad(m, "X51", "X52", 2));
    assert(repairRoad(m, "X52", "X53", 2));
    assert(repairRoad(m, "X53", "X54", 2));
    assert(repairRoad(m, "X54", "X55", 2));
    assert(repairRoad(m, "X55", "X56", 2));
    assert(repairRoad(m, "X56", "X57", 2));
    assert(repairRoad(m, "X57", "X58", 2));
    assert(repairRoad(m, "X58", "X59", 2));
    assert(repairRoad(m, "X59", "X60", 2));
    assert(repairRoad(m, "X60", "X61", 2));
    assert(repairRoad(m, "X61", "X62", 2));
    assert(repairRoad(m, "X62", "X63", 2));
    assert(repairRoad(m, "X63", "X64", 2));
    assert(repairRoad(m, "X64", "X65", 2));
    assert(repairRoad(m, "X65", "X66", 2));
    assert(repairRoad(m, "X66", "X67", 2));
    assert(repairRoad(m, "X67", "X68", 2));
    assert(repairRoad(m, "X68", "X69", 2));
    assert(repairRoad(m, "X69", "X70", 2));
    assert(repairRoad(m, "X70", "X71", 2));
    assert(repairRoad(m, "X71", "X72", 2));
    assert(repairRoad(m, "X72", "X73", 2));
    assert(repairRoad(m, "X73", "X74", 2));
    assert(repairRoad(m, "X74", "X75", 2));
    assert(repairRoad(m, "X75", "X76", 2));
    assert(repairRoad(m, "X76", "X77", 2));
    assert(repairRoad(m, "X77", "X78", 2));
    assert(repairRoad(m, "X78", "X79", 2));
    assert(repairRoad(m, "X79", "X80", 2));
    assert(repairRoad(m, "X80", "X81", 2));
    assert(repairRoad(m, "X81", "X82", 2));
    assert(repairRoad(m, "X82", "X83", 2));
    assert(repairRoad(m, "X83", "X84", 2));
    assert(repairRoad(m, "X84", "X85", 2));
    assert(repairRoad(m, "X85", "X86", 2));
    assert(repairRoad(m, "X86", "X87", 2));
    assert(repairRoad(m, "X87", "X88", 2));
    assert(repairRoad(m, "X88", "X89", 2));
    assert(repairRoad(m, "X89", "X90", 2));
    assert(repairRoad(m, "X90", "X91", 2));
    assert(repairRoad(m, "X91", "X92", 2));
    assert(repairRoad(m, "X92", "X93", 2));
    assert(repairRoad(m, "X93", "X94", 2));
    assert(repairRoad(m, "X94", "X95", 2));
    assert(repairRoad(m, "X95", "X96", 2));
    assert(repairRoad(m, "X96", "X97", 2));
    assert(repairRoad(m, "X97", "X98", 2));
    assert(repairRoad(m, "X98", "X99", 2));
    assert(repairRoad(m, "X99", "X100", 2));
    assert(!extendRoute(m, 10, "X0"));
    assert(!extendRoute(m, 10, "X1"));
    assert(!extendRoute(m, 10, "X2"));
    assert(!extendRoute(m, 10, "X3"));
    assert(!extendRoute(m, 10, "X4"));
    assert(!extendRoute(m, 10, "X5"));
    assert(!extendRoute(m, 10, "X6"));
    assert(!extendRoute(m, 10, "X7"));
    assert(!extendRoute(m, 10, "X8"));
    assert(!extendRoute(m, 10, "X9"));
    assert(!extendRoute(m, 10, "X10"));
    assert(!extendRoute(m, 10, "X11"));
    assert(!extendRoute(m, 10, "X12"));
    assert(!extendRoute(m, 10, "X13"));
    assert(!extendRoute(m, 10, "X14"));
    assert(!extendRoute(m, 10, "X15"));
    assert(!extendRoute(m, 10, "X16"));
    assert(!extendRoute(m, 10, "X17"));
    assert(!extendRoute(m, 10, "X18"));
    assert(!extendRoute(m, 10, "X19"));
    assert(!extendRoute(m, 10, "X20"));
    assert(!extendRoute(m, 10, "X21"));
    assert(!extendRoute(m, 10, "X22"));
    assert(!extendRoute(m, 10, "X23"));
    assert(!extendRoute(m, 10, "X24"));
    assert(!extendRoute(m, 10, "X25"));
    assert(!extendRoute(m, 10, "X26"));
    assert(!extendRoute(m, 10, "X27"));
    assert(!extendRoute(m, 10, "X28"));
    assert(!extendRoute(m, 10, "X29"));
    assert(!extendRoute(m, 10, "X30"));
    assert(!extendRoute(m, 10, "X31"));
    assert(!extendRoute(m, 10, "X32"));
    assert(!extendRoute(m, 10, "X33"));
    assert(!extendRoute(m, 10, "X34"));
    assert(!extendRoute(m, 10, "X35"));
    assert(!extendRoute(m, 10, "X36"));
    assert(!extendRoute(m, 10, "X37"));
    assert(!extendRoute(m, 10, "X38"));
    assert(!extendRoute(m, 10, "X39"));
    assert(!extendRoute(m, 10, "X40"));
    assert(!extendRoute(m, 10, "X41"));
    assert(!extendRoute(m, 10, "X42"));
    assert(!extendRoute(m, 10, "X43"));
    assert(!extendRoute(m, 10, "X44"));
    assert(!extendRoute(m, 10, "X45"));
    assert(!extendRoute(m, 10, "X46"));
    assert(!extendRoute(m, 10, "X47"));
    assert(!extendRoute(m, 10, "X48"));
    assert(!extendRoute(m, 10, "X49"));
    assert(!extendRoute(m, 10, "X50"));
    assert(!extendRoute(m, 10, "X51"));
    assert(!extendRoute(m, 10, "X52"));
    assert(!extendRoute(m, 10, "X53"));
    assert(!extendRoute(m, 10, "X54"));
    assert(!extendRoute(m, 10, "X55"));
    assert(!extendRoute(m, 10, "X56"));
    assert(!extendRoute(m, 10, "X57"));
    assert(!extendRoute(m, 10, "X58"));
    assert(!extendRoute(m, 10, "X59"));
    assert(!extendRoute(m, 10, "X60"));
    assert(!extendRoute(m, 10, "X61"));
    assert(!extendRoute(m, 10, "X62"));
    assert(!extendRoute(m, 10, "X63"));
    assert(!extendRoute(m, 10, "X64"));
    assert(!extendRoute(m, 10, "X65"));
    assert(!extendRoute(m, 10, "X66"));
    assert(!extendRoute(m, 10, "X67"));
    assert(!extendRoute(m, 10, "X68"));
    assert(!extendRoute(m, 10, "X69"));
    assert(!extendRoute(m, 10, "X70"));
    assert(!extendRoute(m, 10, "X71"));
    assert(!extendRoute(m, 10, "X72"));
    assert(!extendRoute(m, 10, "X73"));
    assert(!extendRoute(m, 10, "X74"));
    assert(!extendRoute(m, 10, "X75"));
    assert(!extendRoute(m, 10, "X76"));
    assert(!extendRoute(m, 10, "X77"));
    assert(!extendRoute(m, 10, "X78"));
    assert(!extendRoute(m, 10, "X79"));
    assert(!extendRoute(m, 10, "X80"));
    assert(!extendRoute(m, 10, "X81"));
    assert(!extendRoute(m, 10, "X82"));
    assert(!extendRoute(m, 10, "X83"));
    assert(!extendRoute(m, 10, "X84"));
    assert(!extendRoute(m, 10, "X85"));
    assert(!extendRoute(m, 10, "X86"));
    assert(!extendRoute(m, 10, "X87"));
    assert(!extendRoute(m, 10, "X88"));
    assert(!extendRoute(m, 10, "X89"));
    assert(!extendRoute(m, 10, "X90"));
    assert(!extendRoute(m, 10, "X91"));
    assert(!extendRoute(m, 10, "X92"));
    assert(!extendRoute(m, 10, "X93"));
    assert(!extendRoute(m, 10, "X94"));
    assert(!extendRoute(m, 10, "X95"));
    assert(!extendRoute(m, 10, "X96"));
    assert(!extendRoute(m, 10, "X97"));
    assert(!extendRoute(m, 10, "X98"));
    assert(!extendRoute(m, 10, "X99"));
    assert(!extendRoute(m, 10, "X100"));
    assert(!extendRoute(m, 10, "X101"));

    DK(m, 10, "10;X0;1;2;X1;1;2;X2;1;2;X3;1;2;X4;1;2;X5;1;2;X6;1;2;X7;1;2;X8;"
              "1;2;X9;1;2;X10;1;2;X11;1;2;X12;1;2;X13;1;2;X14;1;2;X15;1;2;X16;"
              "1;2;X17;1;2;X18;1;2;X19;1;2;X20;1;2;X21;1;2;X22;1;2;X23;1;2;X24;"
              "1;2;X25;1;2;X26;1;2;X27;1;2;X28;1;2;X29;1;2;X30;1;2;X31;1;2;X32;"
              "1;2;X33;1;2;X34;1;2;X35;1;2;X36;1;2;X37;1;2;X38;1;2;X39;1;2;X40;"
              "1;2;X41;1;2;X42;1;2;X43;1;2;X44;1;2;X45;1;2;X46;1;2;X47;1;2;X48;"
              "1;2;X49;1;2;X50;1;2;X51;1;2;X52;1;2;X53;1;2;X54;1;2;X55;1;2;X56;"
              "1;2;X57;1;2;X58;1;2;X59;1;2;X60;1;2;X61;1;2;X62;1;2;X63;1;2;X64;"
              "1;2;X65;1;2;X66;1;2;X67;1;2;X68;1;2;X69;1;2;X70;1;2;X71;1;2;X72;"
              "1;2;X73;1;2;X74;1;2;X75;1;2;X76;1;2;X77;1;2;X78;1;2;X79;1;2;X80;"
              "1;2;X81;1;2;X82;1;2;X83;1;2;X84;1;2;X85;1;2;X86;1;2;X87;1;2;X88;"
              "1;2;X89;1;2;X90;1;2;X91;1;2;X92;1;2;X93;1;2;X94;1;2;X95;1;2;X96;"
              "1;2;X97;1;2;X98;1;2;X99;1;2;X100");

    assert(!addRoad(m, "X1", "X0", 1, 1));
    assert(!addRoad(m, "X2", "X1", 1, 1));
    assert(!addRoad(m, "X3", "X2", 1, 1));
    assert(!addRoad(m, "X4", "X3", 1, 1));
    assert(!addRoad(m, "X5", "X4", 1, 1));
    assert(!addRoad(m, "X6", "X5", 1, 1));
    assert(!addRoad(m, "X7", "X6", 1, 1));
    assert(!addRoad(m, "X8", "X7", 1, 1));
    assert(!addRoad(m, "X9", "X8", 1, 1));
    assert(!addRoad(m, "X10", "X9", 1, 1));
    assert(!addRoad(m, "X11", "X10", 1, 1));
    assert(!addRoad(m, "X12", "X11", 1, 1));
    assert(!addRoad(m, "X13", "X12", 1, 1));
    assert(!addRoad(m, "X14", "X13", 1, 1));
    assert(!addRoad(m, "X15", "X14", 1, 1));
    assert(!addRoad(m, "X16", "X15", 1, 1));
    assert(!addRoad(m, "X17", "X16", 1, 1));
    assert(!addRoad(m, "X18", "X17", 1, 1));
    assert(!addRoad(m, "X19", "X18", 1, 1));
    assert(!addRoad(m, "X20", "X19", 1, 1));
    assert(!addRoad(m, "X21", "X20", 1, 1));
    assert(!addRoad(m, "X22", "X21", 1, 1));
    assert(!addRoad(m, "X23", "X22", 1, 1));
    assert(!addRoad(m, "X24", "X23", 1, 1));
    assert(!addRoad(m, "X25", "X24", 1, 1));
    assert(!addRoad(m, "X26", "X25", 1, 1));
    assert(!addRoad(m, "X27", "X26", 1, 1));
    assert(!addRoad(m, "X28", "X27", 1, 1));
    assert(!addRoad(m, "X29", "X28", 1, 1));
    assert(!addRoad(m, "X30", "X29", 1, 1));
    assert(!addRoad(m, "X31", "X30", 1, 1));
    assert(!addRoad(m, "X32", "X31", 1, 1));
    assert(!addRoad(m, "X33", "X32", 1, 1));
    assert(!addRoad(m, "X34", "X33", 1, 1));
    assert(!addRoad(m, "X35", "X34", 1, 1));
    assert(!addRoad(m, "X36", "X35", 1, 1));
    assert(!addRoad(m, "X37", "X36", 1, 1));
    assert(!addRoad(m, "X38", "X37", 1, 1));
    assert(!addRoad(m, "X39", "X38", 1, 1));
    assert(!addRoad(m, "X40", "X39", 1, 1));
    assert(!addRoad(m, "X41", "X40", 1, 1));
    assert(!addRoad(m, "X42", "X41", 1, 1));
    assert(!addRoad(m, "X43", "X42", 1, 1));
    assert(!addRoad(m, "X44", "X43", 1, 1));
    assert(!addRoad(m, "X45", "X44", 1, 1));
    assert(!addRoad(m, "X46", "X45", 1, 1));
    assert(!addRoad(m, "X47", "X46", 1, 1));
    assert(!addRoad(m, "X48", "X47", 1, 1));
    assert(!addRoad(m, "X49", "X48", 1, 1));
    assert(!addRoad(m, "X50", "X49", 1, 1));
    assert(!addRoad(m, "X51", "X50", 1, 1));
    assert(!addRoad(m, "X52", "X51", 1, 1));
    assert(!addRoad(m, "X53", "X52", 1, 1));
    assert(!addRoad(m, "X54", "X53", 1, 1));
    assert(!addRoad(m, "X55", "X54", 1, 1));
    assert(!addRoad(m, "X56", "X55", 1, 1));
    assert(!addRoad(m, "X57", "X56", 1, 1));
    assert(!addRoad(m, "X58", "X57", 1, 1));
    assert(!addRoad(m, "X59", "X58", 1, 1));
    assert(!addRoad(m, "X60", "X59", 1, 1));
    assert(!addRoad(m, "X61", "X60", 1, 1));
    assert(!addRoad(m, "X62", "X61", 1, 1));
    assert(!addRoad(m, "X63", "X62", 1, 1));
    assert(!addRoad(m, "X64", "X63", 1, 1));
    assert(!addRoad(m, "X65", "X64", 1, 1));
    assert(!addRoad(m, "X66", "X65", 1, 1));
    assert(!addRoad(m, "X67", "X66", 1, 1));
    assert(!addRoad(m, "X68", "X67", 1, 1));
    assert(!addRoad(m, "X69", "X68", 1, 1));
    assert(!addRoad(m, "X70", "X69", 1, 1));
    assert(!addRoad(m, "X71", "X70", 1, 1));
    assert(!addRoad(m, "X72", "X71", 1, 1));
    assert(!addRoad(m, "X73", "X72", 1, 1));
    assert(!addRoad(m, "X74", "X73", 1, 1));
    assert(!addRoad(m, "X75", "X74", 1, 1));
    assert(!addRoad(m, "X76", "X75", 1, 1));
    assert(!addRoad(m, "X77", "X76", 1, 1));
    assert(!addRoad(m, "X78", "X77", 1, 1));
    assert(!addRoad(m, "X79", "X78", 1, 1));
    assert(!addRoad(m, "X80", "X79", 1, 1));
    assert(!addRoad(m, "X81", "X80", 1, 1));
    assert(!addRoad(m, "X82", "X81", 1, 1));
    assert(!addRoad(m, "X83", "X82", 1, 1));
    assert(!addRoad(m, "X84", "X83", 1, 1));
    assert(!addRoad(m, "X85", "X84", 1, 1));
    assert(!addRoad(m, "X86", "X85", 1, 1));
    assert(!addRoad(m, "X87", "X86", 1, 1));
    assert(!addRoad(m, "X88", "X87", 1, 1));
    assert(!addRoad(m, "X89", "X88", 1, 1));
    assert(!addRoad(m, "X90", "X89", 1, 1));
    assert(!addRoad(m, "X91", "X90", 1, 1));
    assert(!addRoad(m, "X92", "X91", 1, 1));
    assert(!addRoad(m, "X93", "X92", 1, 1));
    assert(!addRoad(m, "X94", "X93", 1, 1));
    assert(!addRoad(m, "X95", "X94", 1, 1));
    assert(!addRoad(m, "X96", "X95", 1, 1));
    assert(!addRoad(m, "X97", "X96", 1, 1));
    assert(!addRoad(m, "X98", "X97", 1, 1));
    assert(!addRoad(m, "X99", "X98", 1, 1));
    assert(!addRoad(m, "X100", "X99", 1, 1));
    assert(!repairRoad(m, "X1", "X0", 1));
    assert(!repairRoad(m, "X2", "X1", 1));
    assert(!repairRoad(m, "X3", "X2", 1));
    assert(!repairRoad(m, "X4", "X3", 1));
    assert(!repairRoad(m, "X5", "X4", 1));
    assert(!repairRoad(m, "X6", "X5", 1));
    assert(!repairRoad(m, "X7", "X6", 1));
    assert(!repairRoad(m, "X8", "X7", 1));
    assert(!repairRoad(m, "X9", "X8", 1));
    assert(!repairRoad(m, "X10", "X9", 1));
    assert(!repairRoad(m, "X11", "X10", 1));
    assert(!repairRoad(m, "X12", "X11", 1));
    assert(!repairRoad(m, "X13", "X12", 1));
    assert(!repairRoad(m, "X14", "X13", 1));
    assert(!repairRoad(m, "X15", "X14", 1));
    assert(!repairRoad(m, "X16", "X15", 1));
    assert(!repairRoad(m, "X17", "X16", 1));
    assert(!repairRoad(m, "X18", "X17", 1));
    assert(!repairRoad(m, "X19", "X18", 1));
    assert(!repairRoad(m, "X20", "X19", 1));
    assert(!repairRoad(m, "X21", "X20", 1));
    assert(!repairRoad(m, "X22", "X21", 1));
    assert(!repairRoad(m, "X23", "X22", 1));
    assert(!repairRoad(m, "X24", "X23", 1));
    assert(!repairRoad(m, "X25", "X24", 1));
    assert(!repairRoad(m, "X26", "X25", 1));
    assert(!repairRoad(m, "X27", "X26", 1));
    assert(!repairRoad(m, "X28", "X27", 1));
    assert(!repairRoad(m, "X29", "X28", 1));
    assert(!repairRoad(m, "X30", "X29", 1));
    assert(!repairRoad(m, "X31", "X30", 1));
    assert(!repairRoad(m, "X32", "X31", 1));
    assert(!repairRoad(m, "X33", "X32", 1));
    assert(!repairRoad(m, "X34", "X33", 1));
    assert(!repairRoad(m, "X35", "X34", 1));
    assert(!repairRoad(m, "X36", "X35", 1));
    assert(!repairRoad(m, "X37", "X36", 1));
    assert(!repairRoad(m, "X38", "X37", 1));
    assert(!repairRoad(m, "X39", "X38", 1));
    assert(!repairRoad(m, "X40", "X39", 1));
    assert(!repairRoad(m, "X41", "X40", 1));
    assert(!repairRoad(m, "X42", "X41", 1));
    assert(!repairRoad(m, "X43", "X42", 1));
    assert(!repairRoad(m, "X44", "X43", 1));
    assert(!repairRoad(m, "X45", "X44", 1));
    assert(!repairRoad(m, "X46", "X45", 1));
    assert(!repairRoad(m, "X47", "X46", 1));
    assert(!repairRoad(m, "X48", "X47", 1));
    assert(!repairRoad(m, "X49", "X48", 1));
    assert(!repairRoad(m, "X50", "X49", 1));
    assert(!repairRoad(m, "X51", "X50", 1));
    assert(!repairRoad(m, "X52", "X51", 1));
    assert(!repairRoad(m, "X53", "X52", 1));
    assert(!repairRoad(m, "X54", "X53", 1));
    assert(!repairRoad(m, "X55", "X54", 1));
    assert(!repairRoad(m, "X56", "X55", 1));
    assert(!repairRoad(m, "X57", "X56", 1));
    assert(!repairRoad(m, "X58", "X57", 1));
    assert(!repairRoad(m, "X59", "X58", 1));
    assert(!repairRoad(m, "X60", "X59", 1));
    assert(!repairRoad(m, "X61", "X60", 1));
    assert(!repairRoad(m, "X62", "X61", 1));
    assert(!repairRoad(m, "X63", "X62", 1));
    assert(!repairRoad(m, "X64", "X63", 1));
    assert(!repairRoad(m, "X65", "X64", 1));
    assert(!repairRoad(m, "X66", "X65", 1));
    assert(!repairRoad(m, "X67", "X66", 1));
    assert(!repairRoad(m, "X68", "X67", 1));
    assert(!repairRoad(m, "X69", "X68", 1));
    assert(!repairRoad(m, "X70", "X69", 1));
    assert(!repairRoad(m, "X71", "X70", 1));
    assert(!repairRoad(m, "X72", "X71", 1));
    assert(!repairRoad(m, "X73", "X72", 1));
    assert(!repairRoad(m, "X74", "X73", 1));
    assert(!repairRoad(m, "X75", "X74", 1));
    assert(!repairRoad(m, "X76", "X75", 1));
    assert(!repairRoad(m, "X77", "X76", 1));
    assert(!repairRoad(m, "X78", "X77", 1));
    assert(!repairRoad(m, "X79", "X78", 1));
    assert(!repairRoad(m, "X80", "X79", 1));
    assert(!repairRoad(m, "X81", "X80", 1));
    assert(!repairRoad(m, "X82", "X81", 1));
    assert(!repairRoad(m, "X83", "X82", 1));
    assert(!repairRoad(m, "X84", "X83", 1));
    assert(!repairRoad(m, "X85", "X84", 1));
    assert(!repairRoad(m, "X86", "X85", 1));
    assert(!repairRoad(m, "X87", "X86", 1));
    assert(!repairRoad(m, "X88", "X87", 1));
    assert(!repairRoad(m, "X89", "X88", 1));
    assert(!repairRoad(m, "X90", "X89", 1));
    assert(!repairRoad(m, "X91", "X90", 1));
    assert(!repairRoad(m, "X92", "X91", 1));
    assert(!repairRoad(m, "X93", "X92", 1));
    assert(!repairRoad(m, "X94", "X93", 1));
    assert(!repairRoad(m, "X95", "X94", 1));
    assert(!repairRoad(m, "X96", "X95", 1));
    assert(!repairRoad(m, "X97", "X96", 1));
    assert(!repairRoad(m, "X98", "X97", 1));
    assert(!repairRoad(m, "X99", "X98", 1));
    assert(!repairRoad(m, "X100", "X99", 1));
    assert(repairRoad(m, "X1", "X0", 2));
    assert(repairRoad(m, "X2", "X1", 2));
    assert(repairRoad(m, "X3", "X2", 2));
    assert(repairRoad(m, "X4", "X3", 2));
    assert(repairRoad(m, "X5", "X4", 2));
    assert(repairRoad(m, "X6", "X5", 2));
    assert(repairRoad(m, "X7", "X6", 2));
    assert(repairRoad(m, "X8", "X7", 2));
    assert(repairRoad(m, "X9", "X8", 2));
    assert(repairRoad(m, "X10", "X9", 2));
    assert(repairRoad(m, "X11", "X10", 2));
    assert(repairRoad(m, "X12", "X11", 2));
    assert(repairRoad(m, "X13", "X12", 2));
    assert(repairRoad(m, "X14", "X13", 2));
    assert(repairRoad(m, "X15", "X14", 2));
    assert(repairRoad(m, "X16", "X15", 2));
    assert(repairRoad(m, "X17", "X16", 2));
    assert(repairRoad(m, "X18", "X17", 2));
    assert(repairRoad(m, "X19", "X18", 2));
    assert(repairRoad(m, "X20", "X19", 2));
    assert(repairRoad(m, "X21", "X20", 2));
    assert(repairRoad(m, "X22", "X21", 2));
    assert(repairRoad(m, "X23", "X22", 2));
    assert(repairRoad(m, "X24", "X23", 2));
    assert(repairRoad(m, "X25", "X24", 2));
    assert(repairRoad(m, "X26", "X25", 2));
    assert(repairRoad(m, "X27", "X26", 2));
    assert(repairRoad(m, "X28", "X27", 2));
    assert(repairRoad(m, "X29", "X28", 2));
    assert(repairRoad(m, "X30", "X29", 2));
    assert(repairRoad(m, "X31", "X30", 2));
    assert(repairRoad(m, "X32", "X31", 2));
    assert(repairRoad(m, "X33", "X32", 2));
    assert(repairRoad(m, "X34", "X33", 2));
    assert(repairRoad(m, "X35", "X34", 2));
    assert(repairRoad(m, "X36", "X35", 2));
    assert(repairRoad(m, "X37", "X36", 2));
    assert(repairRoad(m, "X38", "X37", 2));
    assert(repairRoad(m, "X39", "X38", 2));
    assert(repairRoad(m, "X40", "X39", 2));
    assert(repairRoad(m, "X41", "X40", 2));
    assert(repairRoad(m, "X42", "X41", 2));
    assert(repairRoad(m, "X43", "X42", 2));
    assert(repairRoad(m, "X44", "X43", 2));
    assert(repairRoad(m, "X45", "X44", 2));
    assert(repairRoad(m, "X46", "X45", 2));
    assert(repairRoad(m, "X47", "X46", 2));
    assert(repairRoad(m, "X48", "X47", 2));
    assert(repairRoad(m, "X49", "X48", 2));
    assert(repairRoad(m, "X50", "X49", 2));
    assert(repairRoad(m, "X51", "X50", 2));
    assert(repairRoad(m, "X52", "X51", 2));
    assert(repairRoad(m, "X53", "X52", 2));
    assert(repairRoad(m, "X54", "X53", 2));
    assert(repairRoad(m, "X55", "X54", 2));
    assert(repairRoad(m, "X56", "X55", 2));
    assert(repairRoad(m, "X57", "X56", 2));
    assert(repairRoad(m, "X58", "X57", 2));
    assert(repairRoad(m, "X59", "X58", 2));
    assert(repairRoad(m, "X60", "X59", 2));
    assert(repairRoad(m, "X61", "X60", 2));
    assert(repairRoad(m, "X62", "X61", 2));
    assert(repairRoad(m, "X63", "X62", 2));
    assert(repairRoad(m, "X64", "X63", 2));
    assert(repairRoad(m, "X65", "X64", 2));
    assert(repairRoad(m, "X66", "X65", 2));
    assert(repairRoad(m, "X67", "X66", 2));
    assert(repairRoad(m, "X68", "X67", 2));
    assert(repairRoad(m, "X69", "X68", 2));
    assert(repairRoad(m, "X70", "X69", 2));
    assert(repairRoad(m, "X71", "X70", 2));
    assert(repairRoad(m, "X72", "X71", 2));
    assert(repairRoad(m, "X73", "X72", 2));
    assert(repairRoad(m, "X74", "X73", 2));
    assert(repairRoad(m, "X75", "X74", 2));
    assert(repairRoad(m, "X76", "X75", 2));
    assert(repairRoad(m, "X77", "X76", 2));
    assert(repairRoad(m, "X78", "X77", 2));
    assert(repairRoad(m, "X79", "X78", 2));
    assert(repairRoad(m, "X80", "X79", 2));
    assert(repairRoad(m, "X81", "X80", 2));
    assert(repairRoad(m, "X82", "X81", 2));
    assert(repairRoad(m, "X83", "X82", 2));
    assert(repairRoad(m, "X84", "X83", 2));
    assert(repairRoad(m, "X85", "X84", 2));
    assert(repairRoad(m, "X86", "X85", 2));
    assert(repairRoad(m, "X87", "X86", 2));
    assert(repairRoad(m, "X88", "X87", 2));
    assert(repairRoad(m, "X89", "X88", 2));
    assert(repairRoad(m, "X90", "X89", 2));
    assert(repairRoad(m, "X91", "X90", 2));
    assert(repairRoad(m, "X92", "X91", 2));
    assert(repairRoad(m, "X93", "X92", 2));
    assert(repairRoad(m, "X94", "X93", 2));
    assert(repairRoad(m, "X95", "X94", 2));
    assert(repairRoad(m, "X96", "X95", 2));
    assert(repairRoad(m, "X97", "X96", 2));
    assert(repairRoad(m, "X98", "X97", 2));
    assert(repairRoad(m, "X99", "X98", 2));
    assert(repairRoad(m, "X100", "X99", 2));
    assert(!extendRoute(m, 10, "X0"));
    assert(!extendRoute(m, 10, "X1"));
    assert(!extendRoute(m, 10, "X2"));
    assert(!extendRoute(m, 10, "X3"));
    assert(!extendRoute(m, 10, "X4"));
    assert(!extendRoute(m, 10, "X5"));
    assert(!extendRoute(m, 10, "X6"));
    assert(!extendRoute(m, 10, "X7"));
    assert(!extendRoute(m, 10, "X8"));
    assert(!extendRoute(m, 10, "X9"));
    assert(!extendRoute(m, 10, "X10"));
    assert(!extendRoute(m, 10, "X11"));
    assert(!extendRoute(m, 10, "X12"));
    assert(!extendRoute(m, 10, "X13"));
    assert(!extendRoute(m, 10, "X14"));
    assert(!extendRoute(m, 10, "X15"));
    assert(!extendRoute(m, 10, "X16"));
    assert(!extendRoute(m, 10, "X17"));
    assert(!extendRoute(m, 10, "X18"));
    assert(!extendRoute(m, 10, "X19"));
    assert(!extendRoute(m, 10, "X20"));
    assert(!extendRoute(m, 10, "X21"));
    assert(!extendRoute(m, 10, "X22"));
    assert(!extendRoute(m, 10, "X23"));
    assert(!extendRoute(m, 10, "X24"));
    assert(!extendRoute(m, 10, "X25"));
    assert(!extendRoute(m, 10, "X26"));
    assert(!extendRoute(m, 10, "X27"));
    assert(!extendRoute(m, 10, "X28"));
    assert(!extendRoute(m, 10, "X29"));
    assert(!extendRoute(m, 10, "X30"));
    assert(!extendRoute(m, 10, "X31"));
    assert(!extendRoute(m, 10, "X32"));
    assert(!extendRoute(m, 10, "X33"));
    assert(!extendRoute(m, 10, "X34"));
    assert(!extendRoute(m, 10, "X35"));
    assert(!extendRoute(m, 10, "X36"));
    assert(!extendRoute(m, 10, "X37"));
    assert(!extendRoute(m, 10, "X38"));
    assert(!extendRoute(m, 10, "X39"));
    assert(!extendRoute(m, 10, "X40"));
    assert(!extendRoute(m, 10, "X41"));
    assert(!extendRoute(m, 10, "X42"));
    assert(!extendRoute(m, 10, "X43"));
    assert(!extendRoute(m, 10, "X44"));
    assert(!extendRoute(m, 10, "X45"));
    assert(!extendRoute(m, 10, "X46"));
    assert(!extendRoute(m, 10, "X47"));
    assert(!extendRoute(m, 10, "X48"));
    assert(!extendRoute(m, 10, "X49"));
    assert(!extendRoute(m, 10, "X50"));
    assert(!extendRoute(m, 10, "X51"));
    assert(!extendRoute(m, 10, "X52"));
    assert(!extendRoute(m, 10, "X53"));
    assert(!extendRoute(m, 10, "X54"));
    assert(!extendRoute(m, 10, "X55"));
    assert(!extendRoute(m, 10, "X56"));
    assert(!extendRoute(m, 10, "X57"));
    assert(!extendRoute(m, 10, "X58"));
    assert(!extendRoute(m, 10, "X59"));
    assert(!extendRoute(m, 10, "X60"));
    assert(!extendRoute(m, 10, "X61"));
    assert(!extendRoute(m, 10, "X62"));
    assert(!extendRoute(m, 10, "X63"));
    assert(!extendRoute(m, 10, "X64"));
    assert(!extendRoute(m, 10, "X65"));
    assert(!extendRoute(m, 10, "X66"));
    assert(!extendRoute(m, 10, "X67"));
    assert(!extendRoute(m, 10, "X68"));
    assert(!extendRoute(m, 10, "X69"));
    assert(!extendRoute(m, 10, "X70"));
    assert(!extendRoute(m, 10, "X71"));
    assert(!extendRoute(m, 10, "X72"));
    assert(!extendRoute(m, 10, "X73"));
    assert(!extendRoute(m, 10, "X74"));
    assert(!extendRoute(m, 10, "X75"));
    assert(!extendRoute(m, 10, "X76"));
    assert(!extendRoute(m, 10, "X77"));
    assert(!extendRoute(m, 10, "X78"));
    assert(!extendRoute(m, 10, "X79"));
    assert(!extendRoute(m, 10, "X80"));
    assert(!extendRoute(m, 10, "X81"));
    assert(!extendRoute(m, 10, "X82"));
    assert(!extendRoute(m, 10, "X83"));
    assert(!extendRoute(m, 10, "X84"));
    assert(!extendRoute(m, 10, "X85"));
    assert(!extendRoute(m, 10, "X86"));
    assert(!extendRoute(m, 10, "X87"));
    assert(!extendRoute(m, 10, "X88"));
    assert(!extendRoute(m, 10, "X89"));
    assert(!extendRoute(m, 10, "X90"));
    assert(!extendRoute(m, 10, "X91"));
    assert(!extendRoute(m, 10, "X92"));
    assert(!extendRoute(m, 10, "X93"));
    assert(!extendRoute(m, 10, "X94"));
    assert(!extendRoute(m, 10, "X95"));
    assert(!extendRoute(m, 10, "X96"));
    assert(!extendRoute(m, 10, "X97"));
    assert(!extendRoute(m, 10, "X98"));
    assert(!extendRoute(m, 10, "X99"));
    assert(!extendRoute(m, 10, "X100"));
    assert(!extendRoute(m, 10, "X101"));

    DK(m, 10, "10;X0;1;2;X1;1;2;X2;1;2;X3;1;2;X4;1;2;X5;1;2;X6;1;2;X7;1;2;X8;"
              "1;2;X9;1;2;X10;1;2;X11;1;2;X12;1;2;X13;1;2;X14;1;2;X15;1;2;X16;"
              "1;2;X17;1;2;X18;1;2;X19;1;2;X20;1;2;X21;1;2;X22;1;2;X23;1;2;X24;"
              "1;2;X25;1;2;X26;1;2;X27;1;2;X28;1;2;X29;1;2;X30;1;2;X31;1;2;X32;"
              "1;2;X33;1;2;X34;1;2;X35;1;2;X36;1;2;X37;1;2;X38;1;2;X39;1;2;X40;"
              "1;2;X41;1;2;X42;1;2;X43;1;2;X44;1;2;X45;1;2;X46;1;2;X47;1;2;X48;"
              "1;2;X49;1;2;X50;1;2;X51;1;2;X52;1;2;X53;1;2;X54;1;2;X55;1;2;X56;"
              "1;2;X57;1;2;X58;1;2;X59;1;2;X60;1;2;X61;1;2;X62;1;2;X63;1;2;X64;"
              "1;2;X65;1;2;X66;1;2;X67;1;2;X68;1;2;X69;1;2;X70;1;2;X71;1;2;X72;"
              "1;2;X73;1;2;X74;1;2;X75;1;2;X76;1;2;X77;1;2;X78;1;2;X79;1;2;X80;"
              "1;2;X81;1;2;X82;1;2;X83;1;2;X84;1;2;X85;1;2;X86;1;2;X87;1;2;X88;"
              "1;2;X89;1;2;X90;1;2;X91;1;2;X92;1;2;X93;1;2;X94;1;2;X95;1;2;X96;"
              "1;2;X97;1;2;X98;1;2;X99;1;2;X100");

    deleteMap(m);
    return PASS;
}

/* Sprawdza różne sytuacje dla klikunastu miast. */
static int route5(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "A", "B", 1, 20));
    assert(addRoad(m, "B", "C", 2, 20));
    assert(addRoad(m, "B", "D", 3, 20));
    assert(addRoad(m, "C", "E", 4, 20));
    assert(addRoad(m, "D", "E", 5, 20));
    assert(addRoad(m, "E", "B", 8, 20));
    assert(addRoad(m, "E", "F", 3, 20));
    assert(!repairRoad(m, "E", "C", 19));
    assert(repairRoad(m, "E", "C", 21));
    assert(!repairRoad(m, "E", "A", 22));
    assert(addRoad(m, "F", "G", 4, 20));
    assert(addRoad(m, "A", "G", 10, 20));
    assert(addRoad(m, "H", "G", 3, 20));
    assert(addRoad(m, "F", "C", 2, 20));
    assert(!addRoad(m, "F", "C", 2, 20));
    assert(!addRoad(m, "C", "F", 2, 20));
    assert(!repairRoad(m, "E", "B", 18));
    assert(repairRoad(m, "E", "C", 22));
    assert(repairRoad(m, "E", "F", 23));
    assert(addRoad(m, "H", "C", 3, 20));
    assert(addRoad(m, "G", "K", 1, 20));
    assert(addRoad(m, "G", "J", 9, 20));
    assert(addRoad(m, "G", "I", 8, 20));
    assert(addRoad(m, "H", "I", 1, 20));
    assert(addRoad(m, "H", "J", 10, 20));
    assert(addRoad(m, "H", "K", 11, 20));
    assert(addRoad(m, "K", "J", 1, 20));
    assert(addRoad(m, "J", "I", 1, 20));
    assert(newRoute(m, 10, "A", "E"));

    DK(m, 10, "10;A;1;20;B;2;20;C;4;22;E");

    assert(extendRoute(m, 10, "H"));

    DK(m, 10, "10;A;1;20;B;2;20;C;4;22;E;3;23;F;4;20;G;3;20;H");

    assert(removeRoad(m, "H", "G"));

    DK(m, 10, "10;A;1;20;B;2;20;C;4;22;E;3;23;F;4;20;G;1;20;K;1;20;J;1;20;I;1;20;H");

    assert(!removeRoad(m, "J", "K"));

    DK(m, 10, "10;A;1;20;B;2;20;C;4;22;E;3;23;F;4;20;G;1;20;K;1;20;J;1;20;I;1;20;H");

    deleteMap(m);
    return PASS;
}

/* Nie można usunąć odcinka drogi, bo próba utworzenia objazdu spowodowałaby
 * powstanie pętli. */
static int route6(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "A", "B", 1, 2000));
    assert(addRoad(m, "C", "A", 100, 2000));
    assert(addRoad(m, "D", "A", 100, 2000));
    assert(addRoad(m, "E", "B", 100, 2000));
    assert(addRoad(m, "F", "B", 100, 2000));

    assert(newRoute(m, 1, "C", "E"));
    assert(newRoute(m, 2, "D", "F"));

    assert(addRoad(m, "A", "G", 10, 2000));
    assert(addRoad(m, "G", "D", 10, 2000));
    assert(addRoad(m, "D", "C", 10, 2000));
    assert(addRoad(m, "C", "E", 10, 2000));
    assert(addRoad(m, "E", "F", 10, 2000));
    assert(addRoad(m, "F", "H", 10, 2000));
    assert(addRoad(m, "H", "B", 10, 2000));

    assert(!removeRoad(m, "A", "B"));

    deleteMap(m);
    return PASS;
}

/* Nie można usunąć odcinka drogi, bo nie ma jednoznacznego objazdu. */
static int route7(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "A", "B", 1, 2));
    assert(addRoad(m, "B", "C", 1, 2));
    assert(addRoad(m, "C", "D", 1, 2));

    assert(newRoute(m, 1, "A", "D"));

    assert(addRoad(m, "B", "E", 1, 2));
    assert(addRoad(m, "E", "C", 1, 2));
    assert(addRoad(m, "B", "F", 1, 2));
    assert(addRoad(m, "F", "C", 1, 2));

    assert(!removeRoad(m, "B", "C"));

    deleteMap(m);
    return PASS;
}

/* Tworzy więcej niż jedną mapę, kasuje mapy, tworzy je na nowo. */
static int maps(void) {
    static const size_t map_count = 99;

    Map *m[map_count];

    for (size_t i = 0; i < map_count; ++i) {
        m[i] = newMap();
        assert(m[i]);
    }

    for (size_t j = 0; j < 3; ++j) {
        assert(addRoad(m[0], "Nowy Dwór Wielkopolski", "Stara Skrobia", 7, 1984));
        assert(addRoad(m[0], "Nowy Dwór Wielkopolski", "Nowa Wieś", 6, 1985));
        assert(addRoad(m[0], "Stara Skrobia", "Znań Zachód", 15, 1985));
        assert(addRoad(m[0], "Nowa Wieś", "Znań Zachód", 13, 1984));

        assert(addRoad(m[map_count - 1], "Nowy Dwór Wielkopolski", "Stara Skrobia", 6, 1984));
        assert(addRoad(m[map_count - 1], "Nowy Dwór Wielkopolski", "Nowa Wieś", 7, 1985));
        assert(addRoad(m[map_count - 1], "Stara Skrobia", "Znań Zachód", 13, 1985));
        assert(addRoad(m[map_count - 1], "Nowa Wieś", "Znań Zachód", 15, 1984));

        assert(newRoute(m[0], 999, "Nowy Dwór Wielkopolski", "Znań Zachód"));
        assert(newRoute(m[map_count - 1], 999, "Nowy Dwór Wielkopolski", "Znań Zachód"));

        DK(m[0], 999, "999;Nowy Dwór Wielkopolski;6;1985;Nowa Wieś;13;1984;Znań Zachód");

        DK(m[map_count - 1], 999, "999;Nowy Dwór Wielkopolski;6;1984;Stara Skrobia;13;1985;Znań Zachód");

        deleteMap(m[0]);
        deleteMap(m[map_count - 1]);
        m[0] = newMap();
        m[map_count - 1] = newMap();
        assert(m[0]);
        assert(m[map_count - 1]);
    }

    for (size_t i = 0; i < map_count; ++i) {
        deleteMap(m[i]);
    }

    return PASS;
}

/* Sprawdza lata ujemne. */
static int years(void) {
    Map *m = newMap();
    assert(m);

    assert(addRoad(m, "Forum Romanum", "Amfiteatrum Flavium", 3, -768));
    assert(addRoad(m, "Forum Romanum", "Thermae Antoninianae", 3, -767));
    assert(addRoad(m, "Forum Romanum", "Circus Maximus", 3, -800));
    assert(addRoad(m, "Amfiteatrum Flavium", "Porta Metrovia", 3, -767));
    assert(addRoad(m, "Thermae Antoninianae", "Porta Metrovia", 3, -766));
    assert(addRoad(m, "Circus Maximus", "Porta Metrovia", 3, 1));

    assert(newRoute(m, 3, "Forum Romanum", "Porta Metrovia"));

    DK(m, 3, "3;Forum Romanum;3;-767;Thermae Antoninianae;3;-766;Porta Metrovia");

    deleteMap(m);
    return PASS;
}

/* Tworzy mapę z długimi i niepermamentnymi nazwami miast. */
static int cities(void) {
    static const size_t city_name_len = 255;

    Map *m = newMap();
    assert(m);

    char city1[city_name_len + 1], city2[city_name_len + 1];
    city1[city_name_len] = '\0';
    city2[city_name_len] = '\0';

    for (char c = 'a'; c < 'z'; ++c) {
        for (size_t i = 0; i < city_name_len; ++i) {
            city1[i] = c;
            city2[i] = c + 1;
        }
        assert(addRoad(m, city1, city2, 7, 9));
    }

    for (size_t i = 0; i < city_name_len; ++i) {
        city1[i] = 'a';
        city2[i] = 'z';
    }
    assert(newRoute(m, 1, city1, city2));

    for (size_t i = 0; i < city_name_len; ++i) {
        city1[i] = '\0';
        city2[i] = '\0';
    }

    size_t j = 0;
    char rd[26 * city_name_len + 25 * 5 + 3];
    rd[j++] = '1';
    rd[j++] = ';';
    for (char c = 'a'; c < 'z'; ++c) {
        for (size_t i = 0; i < city_name_len; ++i)
            rd[j++] = c;
        rd[j++] = ';';
        rd[j++] = '7';
        rd[j++] = ';';
        rd[j++] = '9';
        rd[j++] = ';';
    }
    for (size_t i = 0; i < city_name_len; ++i)
        rd[j++] = 'z';
    rd[j] = '\0';

    DK(m, 1, rd);

    deleteMap(m);
    return PASS;
}

#define CITY_COUNT 1000

/* Tworzy mapę z CITY_COUNT miastami i CITY_COUNT/2 - 1 drogami krajowymi
 * przechodzącymi przez jeden wspólny krótki odcinek drogi. Dodaje odcinki dróg,
 * aby umożliwić objazdy. Usuwa ten krótki odcinek drogi. */
static int long1(void) {
    static char city[CITY_COUNT][4];

    unsigned i = 0;
    for (char c2 = 'a'; c2 <= 'z'; ++c2) {
        for (char c1 = 'a'; c1 <= 'z'; ++c1) {
            for (char c0 = 'A'; c0 <= 'Z'; ++c0) {
                city[i][0] = c0;
                city[i][1] = c1;
                city[i][2] = c2;
                city[i][3] = '\0';
                ++i;
                if (i == CITY_COUNT)
                    goto end;
            }
        }
    }
    end:;

    Map *m = newMap();
    assert(m);

    assert(addRoad(m, city[0], city[1], 1, 996));
    for (i = 2; i < CITY_COUNT; i += 2) {
        assert(addRoad(m, city[0], city[i], 11, 996));
        assert(addRoad(m, city[1], city[i + 1], 11, 996));
    }

    for (i = 2; i < CITY_COUNT; i += 2) {
        assert(newRoute(m, i, city[i], city[i + 1]));
    }

    assert(addRoad(m, city[2], city[3], 2, 996));
    assert(addRoad(m, city[4], city[5], 3, 996));
    for (i = 2; i < CITY_COUNT - 5; i += 2) {
        assert(addRoad(m, city[i], city[i + 4], 7, 996));
        assert(addRoad(m, city[i + 1], city[i + 5], 7, 996));
    }
    assert(addRoad(m, city[CITY_COUNT - 4], city[CITY_COUNT - 3], 4, 996));
    assert(addRoad(m, city[CITY_COUNT - 2], city[CITY_COUNT - 1], 5, 996));

    assert(removeRoad(m, city[0], city[1]));

    DK(m, 2, "2;Caa;11;996;Aaa;11;996;Eaa;3;996;Faa;11;996;Baa;11;996;Daa");
    for (i = 4; i < CITY_COUNT; i += 2) {
        static char rd[] = "000;___;11;996;Aaa;11;996;Caa;2;996;Daa;11;996;Baa;11;996;___";
        int j = dk2str(i, rd);
        memcpy(rd + 4, city[i], 3);
        memcpy(rd + sizeof (rd) - 4, city[i + 1], 3);
        DK(m, i, rd + j);
    }

    deleteMap(m);
    return PASS;
}

/* TODO */
static int long2(void) {
    Map* m = newMap();
    assert(m);

    /* TODO */

    deleteMap(m);
    return PASS;
}

/** URUCHAMIANIE TESTÓW **/

typedef struct {
    char const *name;
    int (*function)(void);
} test_list_t;

#define TEST(t) {#t, t}

static const test_list_t test_list[] = {
        TEST(empty),
        TEST(example),
        TEST(params),
        TEST(memory),
        TEST(forum1),
        TEST(forum2),
        TEST(forum3),
        TEST(description1),
        TEST(description2),
        TEST(remove1),
        TEST(route1),
        TEST(route2),
        TEST(route3),
        TEST(route4),
        TEST(route5),
        TEST(route6),
        TEST(route7),
        TEST(maps),
        TEST(years),
        TEST(cities),
        TEST(long1),
        TEST(long2),
};

int main(int argc, char *argv[]) {
    if (argc != 2)
        return WRONG_TEST;

    for (size_t i = 0; i < SIZE(test_list); ++i)
        if (strcmp(argv[1], test_list[i].name) == 0)
            return test_list[i].function();

    return WRONG_TEST;
}

#pragma clang diagnostic pop
