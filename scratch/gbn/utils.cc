#include "./utils.h"
#include "./config.h"

void incrementSequenceNumber(uint32_t &number) {
    number = (number + 1) % (GbnConfig::maxSequenceNumber + 1);
}

//TODO check if the bounds are inclusive or not
bool sequenceNumberInWindow(
        uint32_t sequenceNumber,
        uint32_t windowStart,
        uint32_t windowEnd
) {
    if(windowEnd >= windowStart) {
        return sequenceNumber >= windowStart && sequenceNumber <= windowEnd;
    } else {
        return sequenceNumber >= windowEnd && sequenceNumber <= windowStart;
    }
}
