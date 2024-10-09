#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "../include/operations.h"

#define MAX_DIRECTORY_NAME 1024
#define TMP_DIRECTORY ""
#define BUFFER_SIZE 1024

void handle_mkdir_operation(int client_sock, uint8_t *buffer) {
    // Print the buffer contents for debugging we can use it to see the request buffer
    printf("Buffer contents (%d bytes):\n", BUFFER_SIZE);
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Extract the session ID bytes (bytes 4 to 7)
    uint32_t session_id;
    memcpy(&session_id, &buffer[4], sizeof(session_id));

    // Convert from network byte order to host byte order
    session_id = ntohl(session_id);

    // Print the session ID
    printf("Session ID: %u\n", session_id);

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

    // when we display the buffer contents, we see that the directory name starts at offset 12
    size_t name_start_pos = 12;  // Start position of the directory name
	// Then we get the name length
    uint16_t dirname_len = ntohs(*(uint16_t*)&buffer[2]);
    printf("Extracted length: %d\n", dirname_len);

    // Extract the directory name from the buffer
    char dirname[dirname_len + 1];
    memset(dirname, 0, sizeof(dirname));

    // Print only the bytes representing the directory name
	// This is will help us verify the directory name we set and see if it creates
    printf("Directory Name Bytes (%hu bytes):\n", dirname_len);
    for (size_t i = 0; i < dirname_len; i++) {
        printf("%02x ", buffer[name_start_pos + i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Copy the directory name from the buffer
    memcpy(dirname, &buffer[name_start_pos], dirname_len);
    dirname[dirname_len] = '\0';

    printf("Extracted Directory Name: %s\n", dirname);

    // Construct the full path for the new directory by concatenating the base directory with the directory name...I used this if we were to create a folder to hold all the directories
    char full_path[MAX_DIRECTORY_NAME + strlen(TMP_DIRECTORY) + 1];
    snprintf(full_path, sizeof(full_path), "%s%s", TMP_DIRECTORY, dirname);
    printf("Full directory path: %s\n", full_path);

    // Attempt to create the directory
    printf("Attempting to create directory: %s\n", full_path);
    if (mkdir(full_path, 0755) == 0) {
        // Send success response code
        send_response_code(client_sock, 0x01, 0);  // Success
        printf("Directory created: %s\n", full_path);
    } else {
        perror("mkdir");  // Print the error to understand why mkdir failed
        // Send failure response code
        send_response_code(client_sock, 0xFF, 0);  // Failure
        printf("Failed to create directory: %s\n", full_path);
    }
}
