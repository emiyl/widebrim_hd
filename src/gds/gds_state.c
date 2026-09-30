#include "gds_state.h"

void gds_state_init(gds_state_t *state) { gds_state_reset(state); }
void gds_state_reset(gds_state_t *state) {
    if (state == NULL) {
        return;
    }

    state->condition_result = false;
    state->skip_next_else_depth = 0U;
    state->skipping_else_block = false;
}
