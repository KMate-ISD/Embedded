#include "arduino.h"
#include "mcu_state_machine.h"
#include "mcu_reed_relay.h"


void update_reed_status(bool debug, PubSubClient* mqtt_client)
{
  relay_status = !relay_status;
  digitalWrite(REED_LED, !relay_status);
  if (!relay_status)
  {
    mqtt_client->publish(UQ_TOPIC_TRIG, "FIRE");
    DEBUG(debug, Serial.println("Door open."))
  }
}