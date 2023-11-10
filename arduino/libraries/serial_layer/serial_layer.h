#ifndef _SERIAL_READER_
#define _SERIAL_READER_

#include "Arduino.h"

class serial_layer {
public:
    serial_layer(uint8_t rx, uint8_t tx);
    void debug();
    void read(byte* dst, uint16_t len);
    bool write(byte* src, uint16_t len);

private:
    bool m_debug;
    uint8_t m_rx;
    uint8_t m_tx;
    uint16_t m_delay;
    uint16_t m_delay_half;
    uint16_t m_delay_double;

    inline bool carrier_sense() {
        uint64_t time_to_check = millis() + 5;
        while (millis() < time_to_check) {
            if (LOW == digitalRead(m_rx)) {
                if (m_debug) {
                    Serial.println("DBG: serial bus in use (carrier sense)");
                }
                return false;
            }
        }
        return true;
    }

    inline bool write_bit(int state, uint16_t time) {
        digitalWrite(m_tx, state);
        uint64_t time_to_check = micros() + time - 4; // 4 micros for the call to micros()
        delayMicroseconds(time / 2);
        // check for a collision by checking for the expected state for the given time
        while (micros() < time_to_check) {
            // we need to check for the inverted state, putting TX to HIGH pulls RX to LOW
            if (state == digitalRead(m_rx)) {
                return false;
            }
        }
        return true;
    }

    inline int write_byte(byte c) {
        // start bit
        if (!write_bit(HIGH, m_delay)) {
            return 1;
        }
        uint8_t p = 0;
        uint8_t t;
        for (t = 0x80; t > 0; t >>= 1) {
            if (c & t) p++;
        }
        // send data
        for (byte mask = 0x01; mask > 0; mask <<= 1) {
            if (c & mask) {
                if (!write_bit(LOW, m_delay)) {
                    return 1000 + mask;
                }
            }
            else {
                if (!write_bit(HIGH, m_delay)) {
                    return 2000 + mask;
                }
            }
        }
        // parity
        if (p & 0x01) {
            if (!write_bit(HIGH, m_delay)) {
                return 4;
            }
        } else {
            if (!write_bit(LOW, m_delay)) {
                return 5;
            }
        }
        // stop bit
        if (!write_bit(LOW, m_delay)) {
            return 6;
        }
        return 0;
    }

    inline bool read_byte(byte& c) {
        c = 0;
        while (digitalRead(m_rx));
        if (LOW == digitalRead(m_rx)) {
            delayMicroseconds(m_delay_half);
            for (int bit = 0; bit < 8; bit++) {
                delayMicroseconds(m_delay);
                c |= digitalRead(m_rx) << bit;
            }
            delayMicroseconds(m_delay_double);
            return true;
        }
        return false;
    }
};

#endif // _SERIAL_READER_
