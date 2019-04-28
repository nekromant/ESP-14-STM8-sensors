#include "esphome.h"
#include "esphome/log.h"

using namespace esphome;

class STC15Component: public UARTDevice, public PollingComponent {
 public:
  STC15Component(UARTComponent *parent, uint32_t update_interval = 3000) : UARTDevice(parent), PollingComponent(update_interval)
  {

  }

  sensor::Sensor *temperature  = new sensor::Sensor();
  sensor::Sensor *temperature2 = new sensor::Sensor();
  sensor::Sensor *humidity = new sensor::Sensor();
  sensor::Sensor *lux = new sensor::Sensor();
  sensor::Sensor *pressure = new sensor::Sensor();

  void setup() override {
    setTimeout(3000);
    // nothing to do here
  }

  void update() override {
     while (available()) {
      char c = read();
    }

    static const char *TAG = "sensor.stc";
    ESP_LOGCONFIG(TAG, "STC"); 

    write('R');
    write('Q');  
    delay(50);

    char buffer[14];
    for (int i=0; i<14; i++) {
      buffer[i] = read();    
      ESP_LOGCONFIG(TAG, "buf[%d] = %x", i, buffer[i]); 
    } 

    if ((buffer[0] != 'D') || (buffer[1] != 'F')) {
      ESP_LOGCONFIG(TAG, "Sync fail");
      return;       
    }

    uint16_t tmp = (((uint16_t) buffer[2]) << 8) | buffer[3];
    lux->publish_state((float) tmp);

    uint32_t tmp2 = (((uint16_t) buffer[4]) << 8) | buffer[5];
    tmp = (((uint16_t) buffer[6]) << 8) | buffer[7];

    pressure->publish_state((((tmp2 << 16)) | tmp) / 100.0 / 1.33322387415);

    tmp = (((uint16_t) buffer[8]) << 8) | buffer[9];
    temperature->publish_state(((float) tmp) / 100.0);

    tmp = (((uint16_t) buffer[12]) << 8) | buffer[13];
    temperature2->publish_state(((float) tmp) / 10.0);

    tmp = (((uint16_t) buffer[10]) << 8) | buffer[11];
    humidity->publish_state(((float) tmp) / 10.0);

  }
};
