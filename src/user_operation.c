#include "../include/operations.h"
#include "../include/session.h"
#include "../include/user.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

void handle_user_operation(int client_sock, uint8_t *buffer) {
    printf("Buffer contents:\n");
    for (size_t i = 0; i < 32; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    uint8_t opcode = buffer[0]; // Extract the opcode
    uint8_t flag = buffer[1]; // Extract the user operation flag
    uint16_t username_len = ntohs(*(uint16_t*)&buffer[4]);
    uint16_t password_len = ntohs(*(uint16_t*)&buffer[6]);

    printf("Opcode: %u\n", opcode);
    printf("Flag: %u\n", flag);
    printf("Extracted username length: %u\n", username_len);
    printf("Extracted password length: %u\n", password_len);

    if (username_len > USERNAME_LENGTH || password_len > PASSWORD_LENGTH) {
        printf("Invalid username or password length\n");
        send_response_code(client_sock, 0xFF, 0);  // Failure
        return;
    }

    char username[username_len + 1];
    char password[password_len + 1];

    memcpy(username, &buffer[12], username_len);
    memcpy(password, &buffer[12 + username_len], password_len);

    username[username_len] = '\0';
    password[password_len] = '\0';

    printf("Username: %s\n", username);
    printf("Password: %s\n", password);

    if (opcode == 0x01) { // Assuming 0x01 is the opcode for user operations
        if (flag == 0x00) {
            // Handle login operation
            User* user = authenticate_user(username, password);
            if (user != NULL) {
                Session* session = create_session(user->role);
                if (session) {
                    send_response_code(client_sock, 0x01, session->session_id);
                    printf("Login successful, session ID: %u\n", session->session_id);
                } else {
                    send_response_code(client_sock, 0xFF, 0);
                    printf("Failed to create session\n");
                }
            } else {
                send_response_code(client_sock, 0xFF, 0);  // Failure
                printf("Invalid credentials\n");
            }
        } else if (flag == 0x01 || flag == 0x02 || flag == 0x03) {
            // Handle create user operation
            UserRole role = READ;
            if (flag == 0x02) {
                role = READ_WRITE;
            } else if (flag == 0x03) {
                role = ADMIN;
            }

            int result = register_user(username, password, role);
            if (result == 0) {
                send_response_code(client_sock, 0x01, 0);  // Success
                printf("User created successfully\n");
            } else {
                send_response_code(client_sock, 0xFF, 0);  // Failure
                printf("Failed to create user\n");
            }
        } else if (flag == 0xff) {
            if (delete_user(username) == 0) {
                // Send success response
                send_response_code(client_sock, 0x00, 0);  // Success
                printf("User deleted successfully\n");
            } else {
                // Send failure response
                send_response_code(client_sock, 0xFF, 0);  // Failure
                printf("User deletion failed\n");
            }
        } else {
            // Handle other operations, like delete user
            printf("Unknown operation flag: %u\n", flag);
            send_response_code(client_sock, 0xFF, 0);  // Failure
        }
    } else {
        // Handle other opcodes
        printf("Unknown opcode: %u\n", opcode);
        send_response_code(client_sock, 0xFF, 0);  // Failure
    }
}
