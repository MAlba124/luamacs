CFLAGS = -Wall -Wextra -O2 -Werror=implicit-function-declaration

default: build

.PHONY: build
build: luamacs.so

luamacs.so:
	$(CC) -c $(CFLAGS) -fpic src/main.c -o luamacs.o -l:liblua.a
	$(CC) -shared luamacs.o -o luamacs.so -l:liblua.a
	@rm luamacs.o

.PHONY: check
check:
	$(CC) $(CFLAGS) -fsyntax-only src/main.c

.PHONY: test
test: build
	./test_emacs.sh

.PHONY: doc
doc:
	@ldoc ./emacs

.PHONY: clean
clean:
	@rm luamacs.so
