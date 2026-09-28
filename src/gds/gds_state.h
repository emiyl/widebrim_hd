#ifndef GDS_STATE_H
#define GDS_STATE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool condition_result;
} gds_state_t;

void gds_state_init(gds_state_t *state);
void gds_state_reset(gds_state_t *state);

#endif // GDS_STATE_H
