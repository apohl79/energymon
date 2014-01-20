#ifndef _SERIAL_READER_
#define _SERIAL_READER_

#include "Arduino.h"

class serial_layer {   
public:
    serial_layer(uint8_t rx, uint8_t tx)
        : m_debug(false), m_rx(rx), m_tx(tx), m_delay(192), m_delay_half(m_delay/2), m_delay_double(m_delay*2) {
        pinMode(m_rx, INPUT);
        pinMode(m_tx, OUTPUT);
        digitalWrite(m_tx, LOW);
        // initialize random number gen for CSMA/CA
        // TODO: detect unused pin number
        randomSeed(analogRead(0));
    }

    void debug() { m_debug = true; }

    void read(byte* dst, uint16_t len) {
        for (uint16_t i = 0; i < len; i++) {
            byte c;
            while (!read_byte(c)) {}
            c ^= 0xff;
            if (m_debug) {
                if (c <= 0xf) {
                    Serial.print("0");
                }
                Serial.print(c, HEX);
                Serial.print(" ");
            }
            *(dst + i) = c;
        }
    }

    void write(byte* src, uint16_t len) {
        uint32_t collisions = 0;
        while (1) {
            carrier_sense();        
            for (uint16_t i = 0; i < len; i++) {
                if (m_debug) {
                    if (src[i] <= 0xf) {
                        Serial.print("0");
                    }
                    Serial.print(src[i], HEX);
                    Serial.print(" ");
                }
                if (!write_byte(src[i] ^ 0xff)) {
                    Serial.println("DBG: collision");
                    // collision, calculate binary exponential backoff and wait
                    uint64_t backoff = random(((1 << collisions) - 1)) * m_delay;
                    delayMicroseconds(backoff);
                    // retry
                    continue;
                }
            }
            if (m_debug) {
                Serial.println();
            }
            return;
        }
    }

private:
    bool m_debug;
    uint8_t m_rx;
    uint8_t m_tx;
    uint16_t m_delay;
    uint16_t m_delay_half;
    uint16_t m_delay_double;

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

    inline void carrier_sense() {
        uint32_t collisions = 0;
        uint64_t time_to_check = millis() + 3;
        while (millis() < time_to_check) {
            if (LOW == digitalRead(m_rx)) {
                collisions++;
                uint64_t backoff = random(((1 << collisions) - 1)) * m_delay;
                delayMicroseconds(backoff);
                time_to_check = millis() + 3;
            }
        }
    }

    inline bool collision(int expected_state) {
        // now we check for a collision after half of the delay time and after the full time too
        for (int i = 2; i; i--) {
            delayMicroseconds(m_delay_half);
            if (expected_state != digitalRead(m_rx)) {
                return true;
            }
        }
        return false;
    }
    
    inline bool write_byte(byte c) {
        // start bit
        digitalWrite(m_tx, HIGH);
        //if (collision(HIGH)) {
        //    return false;
        //}
        delayMicroseconds(m_delay);
        uint8_t p = 0;
        uint8_t t;
        for (t = 0x80; t > 0; t >>= 1) {
            if (c & t) p++;
        }
        // send data
        for (byte mask = 0x01; mask > 0; mask <<= 1) {
            if (c & mask) {
                digitalWrite(m_tx, LOW);
            }
            else {
                digitalWrite(m_tx, HIGH);
            }
            delayMicroseconds(m_delay);
        }
        // parity
        if (p & 0x01) {
            digitalWrite(m_tx, HIGH);
        } else {
            digitalWrite(m_tx, LOW);
        }
        delayMicroseconds(m_delay);
        // stop bit
        digitalWrite(m_tx, LOW);
        delayMicroseconds(m_delay);
        return true;
    }
};

#endif // _SERIAL_READER_
