#ifndef SESSION_H
#define SESSION_H

#include <stdint.h>
#include <time.h>

#define SESSION_TIMEOUT 300 // 5 minutes default
#define PERM_READ   0x01
#define PERM_WRITE  0x02
#define PERM_ADMIN  0x04

// Combined
#define PERM_READ_WRITE (PERM_READ | PERM_WRITE)
#define PERM_READ_WRITE_ADMIN (PERM_READ | PERM_WRITE | PERM_ADMIN)

typedef struct {
    uint32_t session_id;
    time_t last_active;
    uint8_t permission;
    int active;
} Session;

Session* create_session(uint8_t permissions);
int is_session_valid(Session* session);
void update_session(Session* session);
void destroy_session(Session* session);
void initialize_sessions();
Session* get_session(uint32_t session_id);

#endif //SESSION_H
