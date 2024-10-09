#include "../include/operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>

void handle_put_operation(int client_sock, uint8_t *buffer) {

    printf("Buffer contents (%d bytes):\n", 1024);
    for (size_t i = 0; i < 1024; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Extract Opcode
    uint8_t opcode = buffer[0];
    printf("Opcode: %02x\n", opcode);

    // Extract Overwrite Flag
    uint8_t overwrite_flag = buffer[1];
    printf("Overwrite Flag: %02x\n", overwrite_flag);

    // Extract Filename Length
    uint16_t filename_length;
    memcpy(&filename_length, &buffer[2], sizeof(filename_length));
    filename_length = ntohs(filename_length);
    printf("Filename Length: %u\n", filename_length);

    // Extract Session ID
    uint32_t session_id;
    memcpy(&session_id, &buffer[4], sizeof(session_id));
    session_id = ntohl(session_id);
    printf("Session ID: %u\n", session_id);
    // Fetch session information safely
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
    // Extract File Content Length
    uint32_t file_content_length;
    memcpy(&file_content_length, &buffer[8], sizeof(file_content_length));
    file_content_length = ntohl(file_content_length);
    printf("File Content Length: %u\n", file_content_length);

    // Extract Filename
    char filename[filename_length + 1];
    memcpy(filename, &buffer[12], filename_length);
    filename[filename_length] = '\0';  // Null-terminate the filename
    printf("Filename: %s\n", filename);

    // Extract File Content
    char *file_content = (char *)&buffer[12 + filename_length];
    printf("File Content as String: %.5s\n", file_content);

    // Determine file save mode
    int file_flags = O_WRONLY | O_CREAT;
    if (overwrite_flag == 0x00) {
        file_flags |= O_EXCL;  // Fail if file exists
    } else {
        file_flags |= O_TRUNC;  // Overwrite if file exists
    }

    // Attempt to open or create the file
    int fd = open(filename, file_flags, S_IRUSR | S_IWUSR);
    uint8_t response_code;

    if (fd == -1) {
        // File could not be opened/created
        perror("Error opening file");
        response_code = 0x05;  // Indicating failure to create/open file
    } else {
        // Write file content to the file
        ssize_t bytes_written = write(fd, file_content, file_content_length);
        if (bytes_written != file_content_length) {
            perror("Error writing to file");
            response_code = 0x05;  // Indicating failure to write file content
        } else {
            printf("File '%s' saved successfully.\n", filename);
            response_code = 0x01;  // Indicating success
        }
        close(fd);
    }
    send_response_code(client_sock, response_code, file_content_length);
}
