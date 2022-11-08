#define BAUD_RATE   9600
#define TIMEOUT     800
#define DEBUG(x)    if (DEBUG_MODE) { x; }

#define DEBUG_MODE  1

#define REED_RELAY  22
#define STATUS_LED  23

size_t relay_status;
void update_status(void);

void setup() {
  pinMode(REED_RELAY, INPUT);
  pinMode(STATUS_LED, OUTPUT);

  DEBUG(Serial.begin(BAUD_RATE));
  DEBUG(while (!Serial) { });
}

void loop() {
  if (relay_status != digitalRead(REED_RELAY)) { update_status(); }
}

void update_status()
{
  digitalWrite(STATUS_LED, relay_status = !relay_status);
  DEBUG(relay_status ? Serial.println("Door closed.") : Serial.println("Door open."));
}
