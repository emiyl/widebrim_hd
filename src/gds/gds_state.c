#include "gds_state.h"

void gds_state_init(gds_state_t *state) { gds_state_reset(state); }
void gds_state_reset(gds_state_t *state) {
    size_t i;

    state->condition_result = false;
    for (i = 0U; i < GDS_IF_BRANCH_STACK_SIZE; ++i) {
        state->if_branch_taken[i] = false;
    }
    state->if_branch_depth = 0U;
}
