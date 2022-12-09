#include <PubSubClient.h>
#include "arduino.h"
#include "..\mcu_state_machine\mcu_state_machine.h"


size_t pt_value;

size_t take_measurement_ptx(PubSubClient* mqtt_client, uint8_t ptx_pin)
{
  pt_value = analogRead(ptx_pin);
  return(pt_value);
}

void publish_measurement(bool debug, uint8_t field, PubSubClient* mqtt_client)
{
  DEBUG(debug,
    Serial.print("pt_value: ");
    Serial.println(pt_value)
  )

  char* payload = new char[8]();
  snprintf(payload, 7, "%d%d", field, pt_value);
  mqtt_client->publish(UQ_TOPIC_DATA, payload);
  delete[] payload;
}



