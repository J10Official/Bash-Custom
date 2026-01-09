#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

// Constants
#define BUFFER_SIZE 4096

// Function prototypes
void fetch_man_page(const char *command_name);

#endif // NETWORKING_H