#include "networking.h"

#define BUFFER_SIZE 4096

#define BUFFER_SIZE 4096

#define BUFFER_SIZE 4096

void fetch_man_page(const char *command_name) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int bytes_received;
    int header_end = 0;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    // Get the server by name
    server = gethostbyname("man.he.net");
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    server_addr.sin_port = htons(80);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting");
        exit(EXIT_FAILURE);
    }

    // Format the GET request with the correct URL for fetching man pages
    snprintf(request, sizeof(request),
             "GET /cgi-bin/man/man2html?query=%s HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n",
             command_name);

    // Send the GET request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    // Debug: Print the request
    printf("Request sent:\n%s\n", request);

    // Receive the response
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';

        // Debug: Print the raw response
        printf("Raw response received:\n%s\n", response);

        // Skip the HTTP headers
        if (!header_end) {
            char *header_end_ptr = strstr(response, "\r\n\r\n");
            if (header_end_ptr) {
                header_end = 1;
                printf("%s", header_end_ptr + 4);
            }
        } else {
            printf("%s", response);
        }
    }

    if (bytes_received < 0) {
        perror("Error receiving response");
    }

    // Close the socket
    close(sockfd);
}


