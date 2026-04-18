#include <stdlib.h>
#include <stdio.h>

#include "state.h"

RelayState* state_new() {
    RelayState *state = NULL;

    if ((state = calloc(sizeof(RelayState), 1)) == NULL) {
        fprintf(stderr, "Unable to allocate memory for RelayState struct\n");
        return NULL;
    }

    if ((state->brightState = calloc(sizeof(BrightState), 1)) == NULL) {
        fprintf(stderr, "Unable to allocate memory for BrightState struct\n");
        return NULL;
    }

    // Set timestamp to current time
    state->brightState->lastTimestamp = time(NULL);

    return state;
}

void state_free(RelayState *state) {
    if (state->brightState != NULL) {
        free(state->brightState);
        state->brightState = NULL;
    }

    if (state != NULL) {
        free(state);
        state = NULL;
    }
}