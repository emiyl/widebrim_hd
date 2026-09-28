#ifndef GDS_STATE_H
#define GDS_STATE_H

#include <stdbool.h>
#include <stddef.h>

#define GDS_IF_BRANCH_STACK_SIZE 32U

typedef struct {
    bool condition_result;
    bool if_branch_taken[GDS_IF_BRANCH_STACK_SIZE];
    size_t if_branch_depth;
} gds_state_t;

void gds_state_init(gds_state_t *state);
void gds_state_reset(gds_state_t *state);

#endif // GDS_STATE_H
