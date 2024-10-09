#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdint.h>
#include "session.h"

#define BUFFER_SIZE 1024

#define SUCCESS 0x01
#define FAILURE 0xFF

// Handle list remote directory operations
void handle_ls_operation(int client_sock, uint8_t *buffer);
// Handle get remote files operations
void handle_get_operation(int client_sock, uint8_t *buffer);
// Handle put remote files operations
void handle_put_operation(int client_sock, uint8_t *buffer);
// handle user operations
void handle_user_operation(int client_sock, uint8_t *buffer);
// Handle delete remote file operations
void handle_delete_operation(int client_sock, uint8_t *buffer);
// Handle make directory remotely operations
void handle_mkdir_operation(int client_sock, uint8_t *buffer);

// Send unknown opcode
void send_response_code(int client_sock, uint8_t return_code, uint32_t content_length);

#endif