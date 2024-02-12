# Compiler and flags
CC=gcc
CFLAGS=-Wall -g -O2

# All targets
all: output/client output/server _clean_object_files

# Ensure the output directory exists
_output_dir:
	mkdir -p output

# Object files
OBJ=util.o

# Compile util.c into util.o
util.o: util.c util.h _output_dir
	$(CC) $(CFLAGS) -c util.c -o output/util.o

# Compile client.c into binary output/client
output/client: client.c $(OBJ) _output_dir
	$(CC) $(CFLAGS) client.c -o output/client

# Compile server.c into binary output/server
output/server: server.c $(OBJ) _output_dir
	$(CC) $(CFLAGS) server.c -o output/server


# Clean up all
.PHONY: clean _clean_object_files
clean:
	rm -rf output

# Clean up object files
_clean_object_files:
	rm -rf output/*.o