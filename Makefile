export CPLUS_INCLUDE_PATH = /usr/X11/include
export LIBRARY_PATH = /usr/X11/lib

CXX = clang++

objects = Animal.o BioSim.o main.o Map.o filename.o random.o read_parameters.o skip_comment.o

BioSim : $(objects)
	$(CXX) -lpng -o BioSim $(objects)
BioSim-no-png : $(objects)
	$(CXX) -o BioSim $(objects)
documentation : Doxyfile
	doxygen >/dev/null

Animal.o: Animal.cpp Animal.h toolbox/random.h Map.h prefix.h

BioSim.o: BioSim.cpp BioSim.h Animal.h Map.h toolbox/skip_comment.h toolbox/random.h toolbox/filename.h prefix.h

main.o: main.cpp toolbox/random.h BioSim.h Animal.h toolbox/read_parameters.h Map.h prefix.h

Map.o: Map.cpp Map.h toolbox/read_parameters.h Animal.h prefix.h

filename.o: toolbox/filename.cpp toolbox/filename.h
	$(CXX) -c toolbox/filename.cpp

random.o: toolbox/random.cpp toolbox/random.h
	$(CXX) -c toolbox/random.cpp

read_parameters.o: toolbox/read_parameters.cpp toolbox/read_parameters.h toolbox/skip_comment.h
	$(CXX) -c toolbox/read_parameters.cpp

skip_comment.o: toolbox/skip_comment.cpp toolbox/skip_comment.h
	$(CXX) -c toolbox/skip_comment.cpp

install: BioSim documentation
	cp BioSim /usr/bin/BioSim
	rm -rf /usr/share/doc/BioSim
	cp -R doc/html /usr/share/doc/BioSim

clean:
	rm -f BioSim $(objects)
