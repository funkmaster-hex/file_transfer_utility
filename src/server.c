#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "../include/operations.h"
#include "../include/user.h"
#include "../include/session.h"

void handle_client(int client_sock) {
    uint8_t buffer[1024];
    int received_bytes = 0;

    // Process client requests in a loop
    while ((received_bytes = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        if (received_bytes <= 0) {
            printf("Client disconnected or error receiving data\n");
            break;
        }

        uint8_t opcode = buffer[0]; //cant be larger than 1 byte -safe
        printf("Received opcode: %02x\n", opcode);

        uint32_t session_id = 0;
        Session* session = NULL; //initialize to 0

        // Retrieve session ID from the request if the opcode isn't for login (0x01)
        if (opcode != 0x01) {
            if (received_bytes < 12) {
                printf("Insufficient data for session ID\n");
                send_response_code(client_sock, FAILURE, 0);
                continue;  // Continue to wait for further data
            }
        }

        // Handle requests based on opcode
        switch (opcode) {
        case 0x01:
            printf("Handling user operation (Login)\n");
            handle_user_operation(client_sock, buffer);
            break;
        case 0x02:
            printf("Handling delete remote file\n");
            handle_delete_operation(client_sock, buffer);
            break;
        case 0x03:
            printf("Handling list remote directory\n");
            handle_ls_operation(client_sock, buffer);
            break;
        case 0x04:
            printf("Handling get remote file\n");
            handle_get_operation(client_sock, buffer);
            break;
        case 0x06:
            printf("Handling put remote file\n");
            handle_put_operation(client_sock, buffer);
            break;
        case 0x09:
            printf("Handling remote mkdir operation.\n");
            handle_mkdir_operation(client_sock, buffer);
            break;
        default:
            printf("Unknown opcode received\n");
            send_response_code(client_sock, FAILURE, session_id);
            break;
        }

        if (session) {
            update_session(session);
        }
    }

    // Attempt to close the socket and check for success
    if (close(client_sock) == -1) {
        perror("Failed to close socket");
    }
}


// We need to add abit of concurrency using threads in order to handle multiple clients
void *client_handler_thread(void *client_socket) {
    int sock = *(int*)client_socket;
    free(client_socket);

    // Then we create sessions for each client
    Session* session = create_session(PERM_READ);

    if (session == NULL) {
        printf("Max sessions reached, closing connection.\n");
        close(sock);
        pthread_exit(NULL);
    }
    handle_client(sock);

    //destroy_session(session);

    close(sock);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    int session_timeout = SESSION_TIMEOUT;
    char *directory = NULL;
    int port = 0;
    

    //Parse command -line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            session_timeout = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            directory = strdup(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Usage: %s [-t timeout] [-d directory] [-p port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (port == 0 || directory == NULL){
        fprintf(stderr, "Error: Port and directory are required.\n");
        fprintf(stderr, "Usage: %s [-t timeout] [-d directory] [-p port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Starting server...\n");
    printf("Session timeout: %d seconds\n", session_timeout);
    printf("Directory: %s\n", directory);
    printf("Port: %d\n", port);

    // Initialize users
    initialize_users();
    initialize_sessions();

    // Below just sets up the server.
    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    int opt = 1;

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Bind socket to the port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port =htons(port);

    if ( bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    } 

    // Listen for incoming connections
    if(listen(server_sock, 3) < 0){
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", port);

    while (1) {
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if(*client_sock < 0) {
            perror("accept failed");
            free(client_sock);
            continue;
        }

        printf("Client conected from IP: %s and Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler_thread, (void*)client_sock) < 0) {
            perror("could not create thread");
            free(client_sock);
        }
        // clean up memory to avoid memory leaks
        pthread_detach(client_thread);
    }

    close(server_sock);
    return 0;
}