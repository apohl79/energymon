#include <Arduino.h>
#include <serial_layer.h>
#include <crc.h>
#include <bsb.h>
#include <SimpleTimer.h>

bool DEBUG = true;

bsb bus(5, 3); // RX:D5, TX:D3
SimpleTimer timer;

uint32_t t1, t2;

bool send_query(int type) {
  if (bus.write_message(type)) {
    bus.read_message();
    return true;
  } else {
    return false;
  }
}

void query_data() {
  for (int type = 0; type < bsb::TOTAL_QUERIES; type++) {
    if (DEBUG) {
      Serial.print("DBG: sending query for msg type ");
      Serial.println(type);
    }
    int retry = 3;
    bool success = false;
    do {
      success = send_query(type);
      if (success) {
        retry = 0;
      } else {
        if (DEBUG) {
          Serial.print("DBG: query failed, retry=");
          Serial.println(retry);
        }
        delay(2000 + random(3000));
      }
    } while (--retry > 0);
    if (!success) {
      Serial.println("ERR: query failed");
      return;
    }
  }
}

void setup() {
  Serial.begin(9600);
  // query temperaturs every minute
  timer.enable(timer.setInterval(60000, query_data));
  
  if (DEBUG) {
    bus.debug();
  }

  randomSeed(analogRead(0));
}

void loop() {
  bus.read_message();
  timer.run();
}


