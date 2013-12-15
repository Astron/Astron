#pragma once
#include <stdint.h>

typedef uint64_t channel_t;
typedef uint32_t doid_t;
typedef uint32_t zone_t;

#define CHANNEL_MAX ((channel_t)(-1))
#define DOID_MAX ((doid_t)(-1))
#define ZONE_MAX ((zone_t)(-1))
#define ZONE_BITS (sizeof(zone_t)*8)

// Type constants
#define INVALID_CHANNEL 0
#define INVALID_DO_ID 0
#define CONTROL_MESSAGE 1