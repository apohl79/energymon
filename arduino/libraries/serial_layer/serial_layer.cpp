#include "serial_layer.h"

serial_layer::serial_layer(uint8_t rx, uint8_t tx)
    : m_debug(false), m_rx(rx), m_tx(tx), m_delay(192), m_delay_half(m_delay/2), m_delay_double(m_delay*2) {
    pinMode(m_rx, INPUT);
    pinMode(m_tx, OUTPUT);
    digitalWrite(m_tx, LOW);
    // initialize random number gen for CSMA/CA
    // TODO: detect unused pin number
    randomSeed(analogRead(0));
}

void serial_layer::debug() { m_debug = true; }

void serial_layer::read(byte* dst, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        byte c;
        while (!read_byte(c)) {}
        c ^= 0xff;
        *(dst + i) = c;
    }
}

bool serial_layer::write(byte* src, uint16_t len) {
    while (1) {
        if (!carrier_sense()) {
            return false;
        }
        for (uint16_t i = 0; i < len; i++) {
            int err = write_byte(src[i] ^ 0xff);
            if (err > 0) {
                if (m_debug) {
                    Serial.print("DBG: collision on the serial bus (collision detection) -- err=");
                    Serial.println(err);
                }
                digitalWrite(m_tx, LOW);
                return false;
            }
        }
        return true;
    }
}
