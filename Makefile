BIN_FILES = cliente servidor

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
CPPFLAGS = -I$(INSTALL_PATH)/include
LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread -lrt

all: $(BIN_FILES) libclaves.so
.PHONY: all

cliente: cliente.o libclaves.so
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

servidor: servidor.o libclaves.so
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

libclaves.so: claves.o
	$(CC) -shared -o $@ $^

clean:
	rm -f $(BIN_FILES) *.o libclaves.so
.PHONY: clean
