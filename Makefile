# Main makefile

all: config.h
	cd src && $(MAKE)

install: config.h
	cd src && $(MAKE) install

config.h:
	@(echo "config.h not present; please run configure." ; exit 1)

clean:
	cd src && $(MAKE) clean
	cd test && $(MAKE) clean

uninstall:
	cd src && $(MAKE) uninstall
	cd test && $(MAKE) clean
	/bin/rm config.h

test::
	cd test && $(MAKE) all 
