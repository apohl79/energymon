/*
 * Completely based on http://www.mikrocontroller.net/topic/218643
 */
#ifndef _BSB_H_
#define _BSB_H_

#include "Arduino.h"
#include "serial_reader.h"

class bsb {
    struct msg {
        byte head;
        byte src;
        byte dst;
        byte length;
        byte data[32];
    };
    
public:
    bsb(uint8_t rx);
    ~bsb();
    void read_message();

private:
    serial_reader* m_serial;
    uint8_t m_rx;
    msg m_msg;

    void read(byte* to, uint16_t len);
    bool validate() const;
    void print_message() const;
    void print_temp(const char* name, const void* p) const;
};

#endif // _BSB_H_
