#include "bsb.h"
#include "crc.h"

#define BSWAP16(n) (((((uint16_t)(n) & 0xFF)) << 8) | (((uint16_t)(n) & 0xFF00) >> 8))

byte Q_HOTTAP[11] =
    {0xDC, 0x86, 0x00, 0x0B, 0x06, 0x3D, 0x31, 0x05, 0x2F, 0x00, 0x00};
byte Q_COLLECTOR[11] =
    {0xDC, 0x86, 0x00, 0x0B, 0x06, 0x3D, 0x49, 0x05, 0x2A, 0x00, 0x00};
byte Q_BUFFER[11] =
    {0xDC, 0x86, 0x00, 0x0B, 0x06, 0x3D, 0x05, 0x05, 0x34, 0x00, 0x00};

bsb::bsb(uint8_t rx, uint8_t tx) : m_debug(false), m_serial(new serial_layer(rx, tx)) {
    m_query_list = (byte**) malloc(bsb::TOTAL_QUERIES * sizeof(byte*));
    m_query_len = (uint16_t*) malloc(bsb::TOTAL_QUERIES * sizeof(uint16_t));
    init_query(bsb::HOTTAP, Q_HOTTAP, sizeof(Q_HOTTAP));
    init_query(bsb::COLLECTOR, Q_COLLECTOR, sizeof(Q_COLLECTOR));
    init_query(bsb::BUFFER, Q_BUFFER, sizeof(Q_BUFFER));
}

void bsb::debug() {
    m_debug = true;
    m_serial->debug();
}

void bsb::init_query(int type, byte* query, uint16_t len) {
    uint16_t crc = BSWAP16(crc16(query, len - sizeof(crc)));
    m_query_list[type] = (byte*) malloc(len);
    m_query_len[type] = len;
    memcpy(m_query_list[type], query, len - sizeof(crc));
    memcpy(m_query_list[type] + len - sizeof(crc), &crc, sizeof(crc));
}

void bsb::read_message() {
    m_serial->read(&m_msg.head, 1);
    if (m_msg.head == 0xdc) {
        m_serial->read(&m_msg.src, 3);
        m_serial->read(&m_msg.data[0], m_msg.length - 4);
        if (validate()) {
            print_message();
        }
    }
}

bool bsb::validate() const {
    return (0 == crc16(&m_msg.head, m_msg.length));
}

void bsb::print_temp(const char* name, const void* p) {
    Serial.print("temp_");
    Serial.print(name);
    Serial.print("=");
    int16_t t = BSWAP16(*((int16_t*) p));
    Serial.println((float) t / 64);
}

void bsb::print_message() {
    bool noout = false;
    if (m_msg.data[0] == 0x02) {
        if (m_msg.data[3] == 0x02) {
            switch (m_msg.data[4]) {
            case 0x19:
                print_temp("outdoor", &m_msg.data[5]);
                break;
            case 0x29:
                print_temp("feed", &m_msg.data[5]);
                break;
            default:
                noout = true;
            }
        } else {
            noout = true;
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
            default:
                noout = true;
            }
        } else {
            noout = true;
        }
    }
    if (m_debug && noout) {
        Serial.print("DBG: [DC] [SRC:");
        Serial.print(m_msg.src, HEX);
        Serial.print("] [DST:");
        Serial.print(m_msg.dst, HEX);
        Serial.print("] [LEN:");
        Serial.print(m_msg.length);
        Serial.print("] ");
        for (uint8_t i = 0; i < m_msg.length - 4; i++) {
            Serial.print(m_msg.data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

bool bsb::write_message(int type) {
    return m_serial->write(m_query_list[type], m_query_len[type]);
}
