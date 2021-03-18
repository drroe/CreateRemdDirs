include ../config.h

SOURCES=main.cpp FileRoutines.cpp Messages.cpp RemdDirs.cpp TextFile.cpp ReplicaDimension.cpp Groups.cpp StringRoutines.cpp CheckRuns.cpp Submit.cpp MdinFile.cpp Manager.cpp

OBJECTS=$(SOURCES:.cpp=.o)

install: CreateRemdDirs ../bin
	/bin/mv CreateRemdDirs ../bin/

../bin:
	mkdir ../bin

uninstall: clean
	/bin/rm -f ../bin/CreateRemdDirs

all: CreateRemdDirs

CreateRemdDirs: $(OBJECTS)
	$(CXX) -o CreateRemdDirs $(OBJECTS) $(LDFLAGS)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(DIRECTIVES) -o $@ $<

clean:
	/bin/rm -f $(OBJECTS) FindDepend.o CreateRemdDirs

debug: clean
	$(MAKE) install CXXFLAGS='-Wall -g'

findDepend: FindDepend.o
	$(CXX) -o findDepend FindDepend.o

depend: findDepend
	./findDepend $(SOURCES) > depends

# Dependencies
include depends
