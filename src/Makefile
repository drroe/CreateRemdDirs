
SOURCES=main.cpp FileRoutines.cpp Messages.cpp RemdDirs.cpp TextFile.cpp ReplicaDimension.cpp Groups.cpp StringRoutines.cpp CheckRuns.cpp

OBJECTS=$(SOURCES:.cpp=.o)

CXXFLAGS=-Wall

install: CreateRemdDirs
	/bin/mv CreateRemdDirs ../../

uninstall: clean
	/bin/rm -f ../../CreateRemdDirs

all: CreateRemdDirs

CreateRemdDirs: $(OBJECTS)
	$(CXX) -o CreateRemdDirs $(OBJECTS)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	/bin/rm -f $(OBJECTS) CreateRemdDirs

debug: clean
	$(MAKE) install CXXFLAGS='-Wall -g'

findDepend: FindDepend.o
	$(CXX) -o findDepend FindDepend.o

depend: findDepend
	./findDepend $(SOURCES) > depends

# Dependencies
include depends
