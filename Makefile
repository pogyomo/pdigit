build:
	gcc main.c -o pdigit

install: build
	mv pdigit /usr/local/bin/
