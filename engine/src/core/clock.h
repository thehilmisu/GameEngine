#pragma once
#include "definitions.h"

typedef struct kclock {
 f64 start_time;
 f64 elapsed;
} kclock;

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non-started clocks.
void clock_update(kclock* clock);

// Starts the provided clock. Resets elapsed time.
void clock_start(kclock* clock);

// Stops the provided clock. Does not reset elapsed time.
void clock_stop(kclock* clock);
