#include <serial_reader.h>
#include <crc.h>
#include <bsb.h>

bsb bus(5); // D5

void setup() {
  Serial.begin(9600);
}

void loop() {
  bus.read_message();
}

