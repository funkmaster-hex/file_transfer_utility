#include "../include/operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h> // for htonl function
#include <fcntl.h>
#include <errno.h>

// Function to create and send the response
void send_full_response(int client_sock, uint8_t return_code, uint32_t content_length, uint8_t *content, size_t content_size) {
    size_t header_size = 8; // 1 byte for return_code, 1 byte for reserved, 4 bytes for content_length, and 2 bytes for padding
    size_t total_size = header_size + content_size;

    uint8_t *response = (uint8_t *)malloc(total_size);
    if (response == NULL) {
        perror("Failed to allocate memory for response");
        return;
        //TODO: Add some logging  - level ERROR
    }

    // Fill the header
    response[0] = return_code;
    response[1] = 0; // reserved
    *(uint32_t *)(response + 2) = htonl(content_length); // Convert to network byte order

    // Copy the content
    memcpy(response + header_size, content, content_size);

    // Send the response
    printf("Sending response to client (%zu bytes):\n", total_size);
    for (size_t i = 0; i < total_size; i++) {
        printf("%02x ", response[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
    send(client_sock, response, total_size, 0);
    if (errno=-1){
        printf("Sending Error: %s\n", strerror(errno));
        free(response);
        //TODO: Add some logging  - level ERROR
    }
    free(response);
}

void handle_get_operation(int client_sock, uint8_t *buffer) {
    // Print the buffer contents for debugging
    //buffer is limited to 1024 - safe
    printf("Buffer contents (%d bytes):\n", BUFFER_SIZE);
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Extract opcode, reserved, session ID, and filename length
    // safe here because of prior limits
    uint8_t reserved;
    uint32_t session_id;
    uint16_t filename_length;

    memcpy(&reserved, buffer + 1, 1);
    memcpy(&session_id, buffer + 2, 4);
    session_id = ntohl(session_id);  // Convert to host byte order
    memcpy(&filename_length, buffer + 6, 2);
    filename_length = ntohs(filename_length);  // Convert to host byte order

    // Extract filename
    char filename[256];  // Assuming max filename length is max 256 bytes
    memcpy(filename, buffer + 8, filename_length);
    filename[filename_length] = '\0';  // Null-terminate the string

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

    // Check if the session permission is valid - this code might be useless but lets leave it here
    if (current_Session->permission != PERM_READ &&
    current_Session->permission != PERM_WRITE &&
        current_Session->permission != PERM_READ_WRITE &&
        current_Session->permission != PERM_ADMIN &&
        current_Session->permission != PERM_READ_WRITE_ADMIN) {
        // If permissions are not READ, WRITE, READ_WRITE, or ADMIN, return code 0x03
        send_response_code(client_sock, 0x03, 0);
        //TODO: Add some logging  - level ERROR
        return;
        }

    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error"); //error check the cwd command
        send_full_response(client_sock, 0x02, 0, NULL, 0);  // Error code for failed to get current directory
        //TODO: Add some logging  - level ERROR
        return;
    }

    char filepath[1024];
    // Safely construct the filepath, ensuring we do not exceed buffer size
    int ret = snprintf(filepath, sizeof(filepath), "%s/%s", cwd, filename);
    if (ret < 0 || (size_t)ret >= sizeof(filepath)) {  // Corrected the comparison
        fprintf(stderr, "Error: Path length exceeds buffer size or snprintf failed.\n");
        send_full_response(client_sock, 0x02, 0, NULL, 0);  // Error code for path length issue3
        //TODO: Add some logging  - level ERROR
        return;
    }

    // Check if file exists and can be opened
    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("File not found");
        send_full_response(client_sock, 0x02, 0, NULL, 0);  // Error code for file not found
        //TODO: Add some logging  - level ERROR
        return;
    }

    // Open the file for reading
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        send_full_response(client_sock, 0x02, 0, NULL, 0);  // Error code for failed to open file
        //TODO: Add some logging  - level ERROR
        return;
    }

    // Get the file size
    uint32_t file_size = st.st_size;

    // Allocate buffer for the entire response (header + file content)
    uint8_t *file_buffer = (uint8_t *)malloc(file_size);
    if (file_buffer == NULL) {
        perror("Failed to allocate memory for file buffer");
        close(fd);
        send_full_response(client_sock, 0x02, 0, NULL, 0);  // Error code for memory allocation failure
        //TODO: Add some logging  - level ERROR    
        return;
    }

    // Read the file into buffer
    ssize_t bytes_read = read(fd, file_buffer, file_size);
    if (bytes_read != file_size) {
        perror("Error reading file");
        free(file_buffer);
        close(fd);
        send_full_response(client_sock, 0x02, 0, NULL, 0);  // Error code for file read failure
        //TODO: Add some logging  - level ERROR
        return;
    }

    // Close the file
    close(fd);

    // Send the full response (header + content)
    send_full_response(client_sock, 0x01, file_size, file_buffer, file_size);

    // Clean up
    free(file_buffer);
    printf("File '%s' sent successfully.\n", filename);
    //TODO: Add some logging  - level INFO
}
