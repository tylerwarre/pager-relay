#ifndef STATE_H
#define STATE_H

#define __USE_XOPEN // Required for strptime
#include <time.h>

typedef struct BrightState {
    time_t lastTimestamp;
    u_int8_t unread;
} BrightState;

typedef struct RelayState {
    BrightState *brightState;
} RelayState;

RelayState* state_new();
void state_free(RelayState *state);

#endif