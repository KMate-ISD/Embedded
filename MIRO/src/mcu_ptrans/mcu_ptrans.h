#include <PubSubClient.h>
#include "arduino.h"
#include "..\mcu_state_machine\mcu_state_machine.h"


size_t analog_value;

size_t take_measurement(PubSubClient* mqtt_client, uint8_t ptx_pin)
{
  analog_value = analogRead(ptx_pin);
  return(analog_value);
}

void publish_measurement(bool debug, uint8_t field, PubSubClient* mqtt_client)
{
  DEBUG(debug,
    Serial.print("analog_value: ");
    Serial.println(analog_value)
  )

  char* payload = new char[8]();
  snprintf(payload, 7, "%d%d", field, analog_value);
  mqtt_client->publish(UQ_TOPIC_DATA, payload);
  delete[] payload;
}



