/*
 * Completely based on http://www.mikrocontroller.net/topic/218643
 */
#ifndef _BSB_H_
#define _BSB_H_

#include "Arduino.h"
#include "serial_layer.h"

class bsb {
    struct msg {
        byte head;
        byte src;
        byte dst;
        byte length;
        byte data[32];
    };

    
public:
    enum {
        HOTTAP,
        COLLECTOR,
        BUFFER,
        TOTAL_QUERIES
    };

    bsb(uint8_t rx, uint8_t tx);
    void read_message();
    void write_message(int type);

private:
    serial_layer* m_serial;
    msg m_msg;
    byte** m_query_list;
    uint16_t* m_query_len;

    void init_query(int type, byte* query, uint16_t len);
    bool validate() const;
    void print_message();
    void print_temp(const char* name, const void* p);
};

#endif // _BSB_H_
