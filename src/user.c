#include "../include/user.h"
#include <string.h>
#include <stdlib.h>

static User users[MAX_USERS];
static int user_count = 0;

void initialize_users() {
    // Create a default admin user
    strcpy(users[0].username, "admin");
    strcpy(users[0].password, "password");
    users[0].role = ADMIN;
    user_count = 1;
}

User* authenticate_user(const char* username, const char* password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            return &users[i];
            }
    }
    return NULL;
}

int register_user(const char* username, const char* password, UserRole role) {
    if (user_count >= MAX_USERS) return -1;

    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    users[user_count].role = role;
    user_count++;
    return 0;
}

int delete_user(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // Shift users down
            for (int j = i; j < user_count - 1; j++) {
                users[j] = users[j + 1];
            }
            user_count--;
            return 0;
        }
    }
    return -1;
}