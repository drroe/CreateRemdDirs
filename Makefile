# Main makefile

all: config.h externallib
	cd src && $(MAKE)

install: config.h externallib
	cd src && $(MAKE) install

externallib: config.h
	cd external && $(MAKE) all

config.h:
	@(echo "config.h not present; please run configure." ; exit 1)

cleansource:
	cd external && $(MAKE) clean
	cd src && $(MAKE) clean

clean: cleansource
	cd test && $(MAKE) clean

uninstall:
	cd src && $(MAKE) uninstall
	cd test && $(MAKE) clean
	/bin/rm config.h

test::
	cd test && $(MAKE) all 
