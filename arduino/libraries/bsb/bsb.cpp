#include "bsb.h"
#include "crc.h"

bool DEBUG = true;

#define BSWAP16(n) (((((uint16_t)(n) & 0xFF)) << 8) | (((uint16_t)(n) & 0xFF00) >> 8))

bsb::bsb(uint8_t rx) : m_rx(rx) {
    m_serial = new serial_reader(m_rx);
}

bsb::~bsb() {
    delete m_serial;
}

void bsb::read_message() {
    if (DEBUG) {
        Serial.print("DBG: ");
    }
    read(&m_msg.head, 1);
    if (m_msg.head == 0xdc) {
        read(&m_msg.src, 3);
        read(&m_msg.data[0], m_msg.length - 4);
        bool valid = validate();
        if (DEBUG) {
            if (valid) {
                Serial.println("[CRC OK]");
            } else {
                Serial.println("[CRC ERROR]");
            }
        }
        if (valid) {
            print_message();
        }
    } else {
        if (DEBUG) {
            Serial.print("invalid head ");
            Serial.println(m_msg.head, HEX);
        }
    }
}

bool bsb::validate() const {
    return (0 == crc16(&m_msg.head, m_msg.length));
}

void bsb::print_temp(const char* name, const void* p) const {
    Serial.print("temp_");
    Serial.print(name);
    Serial.print("=");
    int16_t t = *((int16_t*) p);
    Serial.println((float) BSWAP16(t) / 64);
}

void bsb::print_message() const {
    if (m_msg.data[0] == 0x02) {
        if (m_msg.data[3] == 0x02) {
            switch (m_msg.data[4]) {
            case 0x19:
                print_temp("outdoor", &m_msg.data[5]);
                break;
            case 0x29:
                print_temp("hottap", &m_msg.data[5]);
                print_temp("buffer", &m_msg.data[7]);
                break;
            }
        }
    } else if (m_msg.data[0] == 0x07) {
        if (m_msg.data[3] == 0x05) {
            switch (m_msg.data[4]) {
            case 0x19:
                print_temp("boiler", &m_msg.data[6]);
                break;
            case 0x21:
                print_temp("outdoor", &m_msg.data[6]);
                break;
            case 0x34:
                print_temp("buffer", &m_msg.data[6]);
                break;
            case 0x2a:
                print_temp("collector", &m_msg.data[6]);
                break;
            case 0x2f:
                print_temp("hottap", &m_msg.data[6]);
                break;
            case 0x6e:
                print_temp("outdoor_max", &m_msg.data[6]);
                break;
            case 0x6f:
                print_temp("outdoor_min", &m_msg.data[6]);
                break;
            }
        }
    }
}

void bsb::read(byte* to, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        byte c;
        while (!m_serial->read(c)) {}
        c ^= 0xff;
        if (DEBUG) {
            if (c <= 0xf) {
                Serial.print("0");
            }
            Serial.print(c, HEX);
            Serial.print(" ");
        }
        *(to + i) = c;
    }
}
