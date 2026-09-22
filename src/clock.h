#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

typedef struct {
    uint64_t prev_frame_counter;
} wb_clock_t;

void clock_init(wb_clock_t *clock);
double wb_clock_tick(wb_clock_t *clock, double target_interval_sec);

#endif // CLOCK_H
