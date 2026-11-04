#include "./utils.h"
#include "./config.h"

void incrementSequenceNumberBy(uint32_t &number, uint32_t by) {
    number = (number + by) % (::GbnConfig::maxSequenceNumber + 1);
}
