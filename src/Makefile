
SOURCES=main.cpp FileRoutines.cpp Messages.cpp RemdDirs.cpp TextFile.cpp ReplicaDimension.cpp Groups.cpp

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

# Dependencies
main.o: main.cpp RemdDirs.h ReplicaDimension.h FileRoutines.h Messages.h TextFile.h
FileRoutines.o: FileRoutines.cpp FileRoutines.h Messages.h
Messages.o: Messages.cpp Messages.h
RemdDirs.o: RemdDirs.cpp RemdDirs.h ReplicaDimension.h Messages.h FileRoutines.h TextFile.h Groups.h
TextFile.o: TextFile.cpp TextFile.h Messages.h
ReplicaDimension.o: ReplicaDimension.cpp ReplicaDimension.h TextFile.h FileRoutines.h Messages.h
Groups.o: Groups.cpp Groups.h Messages.h TextFile.h
