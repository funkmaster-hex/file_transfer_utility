// operations.c
#include "../include/operations.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for htonl function

void send_response_code(int client_sock, uint8_t return_code, uint32_t content_length) {
    uint8_t response[8];
    response[0] = return_code;
    response[1] = 0; // reserved
    *(uint32_t *)(response + 2) = htonl(content_length); // Convert to network byte order

    printf("Sending response to client: ");
    for (size_t i = 0; i < sizeof(response); i++) {
        printf("%02x ", response[i]);
    }
    printf("\n");
    send(client_sock, response, sizeof(response), 0);
}
