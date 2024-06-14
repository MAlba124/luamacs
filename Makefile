CFLAGS = -Wall -Wextra -O2 -Werror=implicit-function-declaration

default: build

.PHONY: build

build:
	$(CC) -c $(CFLAGS) -fpic src/main.c -o luamacs.o -l:liblua.a
	$(CC) -shared luamacs.o -o luamacs.so -l:liblua.a
	@rm luamacs.o

.PHONY: test

test: build
	./test_emacs.sh
