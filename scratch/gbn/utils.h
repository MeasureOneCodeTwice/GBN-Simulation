#pragma once

#include <stdint.h>

void incrementSequenceNumber(uint32_t &number);
bool sequenceNumberInWindow(uint32_t sequenceNumber, uint32_t windowStart, uint32_t windowEnd);
