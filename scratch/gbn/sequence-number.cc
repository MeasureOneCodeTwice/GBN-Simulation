#include "./sequence-number.h"


void SequenceNumber::ApplyModulus() {
    value = value % SequenceNumber::modulus;
}

SequenceNumber::SequenceNumber(uint32_t value) {
    this->value = value;
    ApplyModulus();
}

SequenceNumber& SequenceNumber::operator++() {
    ++this->value;
    ApplyModulus();
    return *this;
}

SequenceNumber SequenceNumber::operator+(const SequenceNumber &other) const {
    return SequenceNumber(this->value + other.value);
}

SequenceNumber SequenceNumber::operator+(uint32_t &other) const {
    return SequenceNumber(this->value + other);
}

SequenceNumber SequenceNumber::operator-(const SequenceNumber &other) const {
    return SequenceNumber(this->value - other.value);
}

bool SequenceNumber::operator==(const SequenceNumber &other) const {
    return this->value == other.value;
}


bool SequenceNumber::operator<=(const SequenceNumber &other) const {
    return this->value <= other.value;
}


bool SequenceNumber::operator>=(const SequenceNumber &other) const {
    return this->value >= other.value;
}

SequenceNumber& SequenceNumber::operator=(const SequenceNumber& other) {
    this->value = other.value;
    return *this;
};

uint32_t SequenceNumber::getValue() const {
    return this->value;
}
std::string SequenceNumber::to_string() const {
    return std::to_string(this->value);
}

bool SequenceNumber::inWindow(SequenceNumber windowStart, SequenceNumber windowEnd) const {
    if(windowEnd >= windowStart) {
        return *this >= windowStart && *this <= windowEnd;
    } else {
        return *this >= windowStart || *this <= windowEnd;
    }
}
