# Main makefile

all:
	cd src && $(MAKE)

install:
	cd src && $(MAKE) install

clean:
	cd src && $(MAKE) clean
	cd test && $(MAKE) clean

uninstall:
	cd src && $(MAKE) uninstall
	cd test && $(MAKE) clean

test::
	cd test && $(MAKE) test
