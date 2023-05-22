Autor:          Dominik Kaczmarek

Załączone pliki:
comp.l -
    skaner odpowiedzialny za analizę leksykalną pliku wejściowego; (flex)

comp.y - 
    parsuje plik zwrócony przez skaner; (bison)

functions.cpp/functions.hpp -
    zawierają funkcje (np. add, sub) oraz struktury (przechowywujące potrzebne danę) używane w pliku 'comp.y'.
    Dzięki nim kod w 'comp.y' jest bardziej czytelny.

smieci (folder) -
    wykorzystany w procesie budowania aplikacji. Pusty zarówno przed jak i po zbudowaniu kompilatora.
    Można go zignorować.

Makefile -
    plik budujący kompilator z plików 'comp.l', 'comp.y', 'functions.cpp', 'functions.hpp'

Uruchomienie kompilatora:
    1.  Znajdując się w folderze '.../261757' wpisać w terminal komendę 'make' (zbudowanie kompilatora).
        W aktualnym folderze pojawi się plik o nazwie 'kompilator'
    2.  Poniższa komenda kompiluje program napisany w języku 'gębalang'.

            kompilator <nazwa pliku wejściowego> <nazwa pliku wyjściowego>
