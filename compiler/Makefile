FLAGS = -std=c++17 
.PHONY = all clean cleanall

all: comp

comp: comp.y comp.l
	bison -d -o build/comp.tab.cpp comp.y
	g++ $(FLAGS) -c build/comp.tab.cpp -o build/comp.tab.o
	flex -o build/compl.cpp comp.l
	g++ $(FLAGS) -c build/compl.cpp -o build/compl.o
	g++ $(FLAGS) -c functions.cpp -o build/utils.o
	g++ $(FLAGS)  -o kompilator build/compl.o build/comp.tab.o build/utils.o -lfl
	rm -f build/*.cpp build/*.hpp build/*.o
	
clean:
	rm -f build/*.cpp build/*.hpp build/*.o

cleanall: clean
	rm -f comp
