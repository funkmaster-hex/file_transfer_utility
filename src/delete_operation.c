#include "../include/operations.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TMP_DIRECTORY ""

void handle_delete_operation(int client_sock, uint8_t *buffer) {
    printf("Received delete operation request\n");

    // Print the buffer contents for debugging
    printf("Buffer contents (%d bytes):\n", 1024);
    for (size_t i = 0; i < 1024; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Extract the filename length (2 bytes)
    uint16_t filename_length = ntohs(*(uint16_t *)&buffer[2]);

    // Check if filename_length is within valid range
    if (filename_length > 1024 - 8) {  // Ensure it fits in buffer
        printf("Filename length too large\n");
        send_response_code(client_sock, 0xff, 0);
        return;
    }

    // Extract the session ID (4 bytes)
    uint32_t session_id = ntohl(*(uint32_t *)&buffer[4]);

    // Extract the filename (filename_length bytes)
    char raw_filename[filename_length + 1];
    memcpy(raw_filename, &buffer[8], filename_length);
    raw_filename[filename_length] = '\0';  // Null-terminate the filename

    // Locate the '{#' part in the filename
    char *truncated_position = strstr(raw_filename, "{#");
    if (truncated_position != NULL) {
        *truncated_position = '\0';  // Truncate the filename at '{#'
    }

    // Final extracted filename
    char *filename = raw_filename;

    printf("Filename Length: %d\n", filename_length);
    printf("Session ID: %d\n", session_id);
    printf("Filename: %s\n", filename);

    Session* current_Session = get_session(session_id);
    if (current_Session == NULL) {
        printf("Invalid session\n");
        send_response_code(client_sock, 0x02, 0);
        return;
    }

    printf("Session ID: %u, Session permissions: %d, isActive: %d, last Active: %ld\n",
           current_Session->session_id, current_Session->permission, current_Session->active, current_Session->last_active);

    // Check if the session permission is valid
    if (current_Session->permission != PERM_WRITE &&
        current_Session->permission != PERM_READ_WRITE &&
        current_Session->permission != PERM_ADMIN &&
        current_Session->permission != PERM_READ_WRITE_ADMIN) {
        // If permissions are not READ, WRITE, READ_WRITE, or ADMIN, return code 0x03
        send_response_code(client_sock, 0x03, 0);
        return;
        }

    // Prepare the full path for deletion
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s%s", TMP_DIRECTORY, filename);

    // Attempt to delete the file or directory
    int delete_status = remove(filepath);
    if (delete_status == 0) {
        printf("Successfully deleted: %s\n", filepath);
        // Send success response
        send_response_code(client_sock, 0x01, 0);
    } else {
        perror("Failed to delete file");
        // Send failure response due to delete error
        send_response_code(client_sock, 0xff, 0);
    }
}
