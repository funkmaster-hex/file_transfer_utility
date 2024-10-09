#ifndef USER_H
#define USER_H

#define USERNAME_LENGTH 32
#define PASSWORD_LENGTH 32
#define MAX_USERS 10

typedef enum {
    READ,
    READ_WRITE,
    ADMIN
} UserRole;

typedef struct {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    UserRole role;
} User;

void initialize_users();
User* authenticate_user(const char* username, const char* password);
int register_user(const char* username, const char* password, UserRole role);
int delete_user(const char* username);

#endif //USER_H
