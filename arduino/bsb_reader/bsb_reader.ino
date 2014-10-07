#include <serial_layer.h>
#include <crc.h>
#include <bsb.h>
#include <SimpleTimer.h>

bool DEBUG = true;

bsb bus(5, 3); // RX:D5, TX:D3
SimpleTimer timer;

void setup() {
  Serial.begin(9600);
  // query temperaturs every minute
  timer.enable(timer.setInterval(60000, query_data));
  
  if (DEBUG) {
    bus.debug();
  }
}

void loop() {
  bus.read_message();
  timer.run();
}

void query_data() {
  for (int type = 0; type < bsb::TOTAL_QUERIES; type++) {
    if (DEBUG) {
      Serial.print("DBG: sending query for msg type ");
      Serial.println(type);
    }
    bus.write_message(type);
    bus.read_message();
  }
}

