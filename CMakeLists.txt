cmake_minimum_required(VERSION 3.0)
project(Drogi C)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

OPTION(-B "" 1)

SET(CMAKE_C_COMPILER /usr/bin/gcc)

# Nie chcemy oglądać komunikatów o procentowym postępie kompilacji.
#set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

# Chcemy zobaczyć polecenia wywołane przez make.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Ustawiamy wspólne opcje kompilowania dla wszystkich wariantów projektu.
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra")
# Domyślne opcje dla wariantów Release i Debug są sensowne.
# Jeśli to konieczne, ustawiamy tu inne.
 set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
 set(CMAKE_C_FLAGS_DEBUG "-g")

# Wskazujemy pliki źródłowe.
set(SOURCE_FILES
    src/city_map.c
    src/city_map.h
    src/global_declarations.h
    src/map.c
    src/map.h
    src/queue.c
    src/queue.h
    src/trie.c
    src/trie.h
    src/trunk.c
    src/trunk.h
    src/road.c
    src/road.h
    src/city.c
    src/city.h
    src/parser.c
    src/parser.h
    src/map_main.c
)

# Wskazujemy plik wykonywalny.
add_executable(map ${SOURCE_FILES})

# Dodajemy obsługę Doxygena: sprawdzamy, czy jest zainstalowany i jeśli tak to:
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # Wskazujemy lokalizację pliku konfiguracyjnego i podajemy jego docelową lokalizację w folderze, gdzie następuje kompilacja.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    # Dodajemy cel doc: użycie make doc spowoduje wywołanie doxygena z odpowiednim plikiem konfiguracyjnym w folderze kompilacji.
    # Na wyjście zostanie wypisany odpowiedni komentarz.
    add_custom_target(doc
        doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif (DOXYGEN_FOUND)
