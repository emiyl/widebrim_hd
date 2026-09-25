#include "gds_state.h"

void gds_state_init(gds_state_t *state) { gds_state_reset(state); }
void gds_state_reset(gds_state_t *state) { state->if_condition = false; }
