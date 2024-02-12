# Compiler and flags
CC=gcc
CFLAGS=-Wall -g

# All targets
all: output/client output/server

# Ensure the output directory exists
_output_dir:
	@mkdir -p output

# Compile client.c into binary output/client
output/client: client.c _output_dir
	$(CC) $(CFLAGS) client.c -o output/client

# Compile server.c into binary output/server
output/server: server.c _output_dir
	$(CC) $(CFLAGS) server.c -o output/server

# Clean up binaries
.PHONY: clean
clean:
	rm -rf output
