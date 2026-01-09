# Compiler
CC = gcc

# Source files
SRCS = command_handler.c directory.c log.c neonate.c networking.c processlist.c proclore.c seek.c signals.c utils.c main.c alias.c

# Executable name
EXEC = shell

# Default target
all: $(EXEC)

# Link object files to create the executable
$(EXEC):
	$(CC) -o $(EXEC) $(SRCS)

# Clean up the executable
clean:
	rm -f $(EXEC)

# Phony targets
.PHONY: all clean
