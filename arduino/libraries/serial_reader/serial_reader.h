#ifndef _SERIAL_READER_
#define _SERIAL_READER_

#include "Arduino.h"

/*
 * 4800 baud serial reader
 *
 * http://arduino.cc/en/Tutorial/SoftwareSerial
 */
class serial_reader {
public:
    serial_reader(uint8_t rx)
        : m_rx(rx), m_delay(188), m_delay_half(m_delay/2), m_delay_double(m_delay*2) {
        pinMode(m_rx, INPUT);
    }

    bool read(byte& c) {
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

private:
    uint8_t m_rx;
    uint16_t m_delay;
    uint16_t m_delay_half;
    uint16_t m_delay_double;
};

#endif // _SERIAL_READER_
