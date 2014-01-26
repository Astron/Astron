#pragma once
#include <stdint.h>
#ifdef ASTRON_128BIT_CHANNELS
#include "util/uint128.h"
typedef uint128_t channel_t;
typedef uint64_t doid_t;
typedef uint64_t zone_t;
#else
typedef uint64_t channel_t;
typedef uint32_t doid_t;
typedef uint32_t zone_t;
#endif

#define CHANNEL_MAX ((channel_t)(-1))
#define DOID_MAX ((doid_t)(-1))
#define ZONE_MAX ((zone_t)(-1))
#define ZONE_BITS (sizeof(zone_t)*8)

// Type constants
#define INVALID_CHANNEL channel_t(0)
#define INVALID_DO_ID doid_t(0)
#define CONTROL_MESSAGE 1
