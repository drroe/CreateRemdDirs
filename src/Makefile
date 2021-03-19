include ../config.h

SOURCES=main.cpp FileRoutines.cpp Messages.cpp RemdDirs.cpp TextFile.cpp ReplicaDimension.cpp Groups.cpp StringRoutines.cpp CheckRuns.cpp Submit.cpp MdinFile.cpp Manager.cpp System.cpp Run.cpp Run_Single.cpp NC.cpp Simulation.cpp

OBJECTS=$(SOURCES:.cpp=.o)

install: CreateRemdDirs ../bin
	/bin/mv CreateRemdDirs ../bin/

../bin:
	mkdir ../bin

uninstall: clean
	/bin/rm -f ../bin/CreateRemdDirs

all: CreateRemdDirs

CreateRemdDirs: $(OBJECTS) ../external/readline/libreadline.a
	$(CXX) -o CreateRemdDirs $(OBJECTS) $(LDFLAGS) ../external/readline/libreadline.a

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(DIRECTIVES) -o $@ $<

../external/readline/libreadline.a:
	cd ../external && $(MAKE) libreadline

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
