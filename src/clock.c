
#include "clock.h"

#include <stdint.h>

#define CLOCK_PRECISION_SEC 0.0015

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static uint64_t clock_get_counter(void) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)counter.QuadPart;
}

static uint64_t clock_get_frequency(void) {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return (uint64_t)frequency.QuadPart;
}

static void clock_sleep_ns(uint64_t nanoseconds) {
    /* Sleep has millisecond precision. */
    DWORD milliseconds = (DWORD)(nanoseconds / 1000000ULL);

    if (milliseconds > 0) {
        Sleep(milliseconds);
    }
}

#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)

#include <time.h>

static uint64_t clock_get_counter(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static uint64_t clock_get_frequency(void) { return 1000000000ULL; }

static void clock_sleep_ns(uint64_t nanoseconds) {
    struct timespec request;

    request.tv_sec = (time_t)(nanoseconds / 1000000000ULL);
    request.tv_nsec = (long)(nanoseconds % 1000000000ULL);

    nanosleep(&request, NULL);
}

#else

#error "Unsupported platform"

#endif

static double clock_elapsed_sec(uint64_t since, uint64_t freq) {
    return (double)(clock_get_counter() - since) / (double)freq;
}

void clock_init(wb_clock_t *clock) {
    clock->prev_frame_counter = clock_get_counter();
}

double wb_clock_tick(wb_clock_t *clock, double target_interval_sec) {
    uint64_t freq = clock_get_frequency();
    uint64_t last = clock->prev_frame_counter;

    double time_idle = target_interval_sec - clock_elapsed_sec(last, freq);

    double time_sleep = time_idle - CLOCK_PRECISION_SEC;

    if (time_sleep > 0.0) {
        clock_sleep_ns((uint64_t)(time_sleep * 1e9));
    }

    while (clock_elapsed_sec(last, freq) < target_interval_sec) {
        /* Busy wait for the remaining interval. */
    }

    clock->prev_frame_counter = clock_get_counter();

    return clock_elapsed_sec(last, freq) * 1000.0;
}
