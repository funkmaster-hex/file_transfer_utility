#include "../include/operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <arpa/inet.h>

#define TMP_DIRECTORY ""
#define BUFFER_SIZE 1024

void handle_ls_operation(int client_sock, uint8_t *buffer) {
    printf("Received list remote directory operation \n");

    // Print the buffer contents
    printf("Buffer contents (%d bytes):\n", BUFFER_SIZE);
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Extract values from the buffer
    uint16_t directory_name_length;
    uint32_t session_id, current_position;

    memcpy(&directory_name_length, buffer + 2, 2);
    directory_name_length = ntohs(directory_name_length);  // Convert to host byte order

    memcpy(&session_id, buffer + 4, 4);
    session_id = ntohl(session_id);  // Convert to host byte order

    memcpy(&current_position, buffer + 8, 4);
    current_position = ntohl(current_position);  // Convert to host byte order

    printf("Session ID: %u\n", session_id);
    printf("Directory name length: %u\n", directory_name_length);
    printf("Current position: %u\n", current_position);

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
        current_Session->permission != PERM_READ &&
        current_Session->permission != PERM_READ_WRITE &&
        current_Session->permission != PERM_ADMIN &&
        current_Session->permission != PERM_READ_WRITE_ADMIN) {
        // If permissions are not READ, WRITE, READ_WRITE, or ADMIN, return code 0x03
        send_response_code(client_sock, 0x03, 0);
        return;
        }
    // Extract the directory name
    char directory_name[directory_name_length + 1];
    if (directory_name_length > 0) {
        memcpy(directory_name, buffer + 12, directory_name_length);
    }
    directory_name[directory_name_length] = '\0';  // Null-terminate

    printf("Directory name: '%s'\n", directory_name);

    // Construct the full directory path
    char full_path[BUFFER_SIZE];
    if (directory_name_length == 0) {
        snprintf(full_path, sizeof(full_path), "%s", TMP_DIRECTORY);
    } else {
        snprintf(full_path, sizeof(full_path), "%s%s", TMP_DIRECTORY, directory_name);
    }

    printf("Directory path: '%s'\n", full_path);

    // Open the directory
    DIR *dir = opendir(full_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        send_response_code(client_sock, 0xFF, 0);  // Directory open failure
        return;
    }

    // Prepare to build the response content
    uint8_t response_content[BUFFER_SIZE - 12];
    uint32_t content_position = 0;
    uint32_t total_content_length = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        uint8_t entry_type = (entry->d_type == DT_DIR) ? 0x02 : 0x01;  // 0x02 for directory, 0x01 for file
        size_t name_len = strlen(entry->d_name);

        // Log each entry being added
        printf("Adding entry: '%s' (Type: %s)\n", entry->d_name, entry_type == 0x02 ? "Directory" : "File");

        // Check if there's enough space in the buffer for this entry
        if (content_position + 1 + name_len + 1 > sizeof(response_content)) {
            printf("Buffer is full, stopping entry addition.\n");
            break;
        }

        // Add the entry to the response content
        response_content[content_position++] = entry_type;
        memcpy(&response_content[content_position], entry->d_name, name_len);
        content_position += name_len;
        response_content[content_position++] = '\0';  // Null-terminate the name

        total_content_length += 1 + name_len + 1;  // Update the total content length
    }
    closedir(dir);

    // Prepare and send the response header
    send_response_code(client_sock, 0x01, total_content_length);

	// Log the directory listing bytes
    printf("Directory listing bytes (%u bytes):\n", content_position);
    for (size_t i = 0; i < content_position; i++) {
        printf("%02x ", response_content[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");


    // Send the actual directory listing content if any
    if (total_content_length > 0) {
        ssize_t sent_bytes = send(client_sock, response_content, content_position, 0);
        if (sent_bytes < 0) {
            perror("Failed to send directory listing content");
        } else {
            printf("Directory listing sent to client. Total content length: %u, Sent content length: %u\n", total_content_length, content_position);
        }
    }
}


