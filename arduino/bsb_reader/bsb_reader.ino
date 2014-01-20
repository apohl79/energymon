#include <serial_layer.h>
#include <crc.h>
#include <bsb.h>
#include <SimpleTimer.h>

bsb bus(5, 3); // RX:D5, TX:D3
SimpleTimer timer;

void setup() {
  Serial.begin(9600);
  // query temperaturs every minute
  timer.enable(timer.setInterval(60000, query_data));
}

void loop() {
  bus.read_message();
  timer.run();
}

void query_data() {
  for (int type = 0; type < bsb::TOTAL_QUERIES; type++) {
    bus.write_message(type);
    bus.read_message();
  }
}

