#include "../include/session.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_SESSIONS 100
Session sessions[MAX_SESSIONS];
void initialize_sessions() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        sessions[i].active = 0;
    }
}

// We generate sessions randomly
static uint32_t generate_session_id() {
    uint32_t session_id;
    int is_unique;
    do {
        session_id = (uint32_t)rand();
        is_unique = 1;

        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (sessions[i].active && sessions[i].session_id == session_id) {
                is_unique = 0;
                break;
            }
        }
    } while (!is_unique);

    return session_id;
}

Session* create_session(uint8_t permissions){
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!sessions[i].active) {
            sessions[i].session_id = generate_session_id();
            sessions[i].last_active = time(NULL);
            sessions[i].permission = permissions;
            sessions[i].active = 1;
            return &sessions[i];
        }
    }
    return NULL;
}

int is_session_valid(Session* session) {
    if (!session || !session -> active ) return 0;

    time_t current_time = time(NULL);
    return difftime(current_time, session->last_active) <= SESSION_TIMEOUT;
}

void update_session(Session* session) {
    if (session) {
        session->last_active = SESSION_TIMEOUT;
    }
}

void destroy_session(Session* session) {
    if (session) {
        free(session);
    }
}

Session* get_session(uint32_t session_id) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].session_id == session_id && sessions[i].active) {
            return &sessions[i];
        }
    }
    return NULL;
}