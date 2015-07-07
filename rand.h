#ifndef RAND_H_
#define RAND_H_

#include <stdint.h>

#define R_MAX UINT32_MAX

typedef struct {
    uint64_t curval;
} rand_s;

static inline uint32_t next_rand(rand_s *state) {
    state->curval ^= state->curval >> 12; // a
    state->curval ^= state->curval << 25; // b
    state->curval ^= state->curval >> 27; // c
    state->curval |= 1;
	return (state->curval * 2685821657736338717) >> 32;
}

static inline uint32_t next_rand_to(rand_s *state, uint32_t max) {
    return next_rand(state) % max;
}

static inline void seed_rand(rand_s *state, uint64_t seed) {
    state->curval = seed;
}


#endif
