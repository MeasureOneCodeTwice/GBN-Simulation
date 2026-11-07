#pragma once

#include <stdint.h>
#include "../config.h"

class SequenceNumber {

    public:
        SequenceNumber() = default;
        SequenceNumber(uint32_t);

        bool operator==(const SequenceNumber& other) const;

        SequenceNumber operator-(const SequenceNumber& other) const;
        SequenceNumber operator+(const SequenceNumber& other) const;
        SequenceNumber operator+(uint32_t& other) const;

        SequenceNumber& operator=(const SequenceNumber& other);
        SequenceNumber& operator++(); //prefix increment

        //where the other SequenceNumber is "further ahead" in the ring
        //(e.g. this = m_baseSequenceNumber, other = m_nextSequenceNumber)
        uint32_t distance(const SequenceNumber &other) const;

        std::string to_string() const;
        bool inWindow(const SequenceNumber windowStart, const SequenceNumber windowEnd) const;

        uint32_t getValue() const;

    private:
        //comparison doesn't make sense in modular arithmatic,
        //these are helper methods that just compare the underlying value.
        bool operator>=(const SequenceNumber& other) const;
        bool operator<=(const SequenceNumber& other) const;

        void ApplyModulus();
        static const uint32_t modulus = GbnConfig::maxSequenceNumber + 1;
        uint32_t value;
};
