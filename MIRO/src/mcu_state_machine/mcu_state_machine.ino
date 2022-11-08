/*
 #  STATE             TRIGGER
 0  Initialize        power on
 1  Listen            Initialize phase ends while no credentials are saved / On board flash button released while in Normal operational state.
 2  Normal operation  Connection established
 3  Transmit          On-board flash button released while in Listen state
 4  Reset             On-board flash button pressed and held for 5 seconds
 5  Deep sleep        Listen phase ends while no credentials are saved
 */


#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFi.h>


/*
 * MACRO
 */

#define BTN             0
#define LED             2
#define RF_RST          22
#define RF_SS           21
#define TIMER0          0
#define TIMER1          1

#define DEBUG(a)        if (debug) { a; }


/*
 * ENUM
 */

enum State
{
  Initialize,
  Listen,
  Normal_op,
  Transmit,
  Reset,
  Deep_sleep,
  Undefined = 0xFF
};

enum Switch_state
{
  sw_normal,
  sw_receive,
  sw_transmit
};

/*
 * GLOBAL
 */

  // const
const size_t BAUD_RATE  = 9600;
const size_t REST       = 6000;
const size_t TIMER_DIV  = 80;
const size_t TIMER_TRSH = 500000;

  // NVM
Preferences preferences;

  // Wi-Fi
const char* wlan_ssid;
const char* wlan_psk;
WiFiClient wifi_client;

  // MQTT
uint16_t mqtt_port;
const char* mqtt_broker;
const char* mqtt_pass;
const char* mqtt_user;
PubSubClient* mqtt_client;

  // Op.
bool held               = false;
bool debug              = true;
uint8_t miro_state      = Undefined;
size_t t0;
size_t td               = 0;

  // Peripherals
bool led_state          = false;
uint8_t switch_state    = sw_normal;

  // Timer
bool autoreload         = true;
bool edge               = true;
bool timer_count_up     = true;
uint64_t alarm_treshold = TIMER_TRSH;
uint16_t timer_divider  = TIMER_DIV;
hw_timer_t* timer_cycle;
hw_timer_t* timer_span;


/*
 * FUNC
 */

void init_wifi(void);
void init_mqtt(void);
void init_timer(void);
void mqtt_reconnect(void);

void on_message(const char*, byte*, uint8_t);
void parse_ip_to_string(const char*, uint8_t*);

void IRAM_ATTR ISR(void);
void IRAM_ATTR on_alarm_cycle(void);
void IRAM_ATTR on_alarm_span(void);


/*
 * STATE 0, 1
 */

void setup()
{
  // [0] Initialize
    // Op.
  miro_state = Initialize;
  Serial.begin(BAUD_RATE);
  DEBUG(Serial.println())

    // NVM


    // WLAN
  wlan_ssid   = "Telekom-B4Wf5Y";
  wlan_psk    = "PIcSafaSZ_+9*";
  init_wifi();

    // MQTT
  mqtt_port   = 1883;
  mqtt_user   = "user";
  mqtt_pass   = "pass";
  mqtt_broker = (const char*)malloc(16*sizeof(char*));
  uint8_t mqtt_broker_bytes[4] = {192, 168, 1, 85};
  parse_ip_to_string(mqtt_broker, mqtt_broker_bytes);
  init_mqtt();

    // Peripherals
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);
  attachInterrupt(BTN, ISR, CHANGE);

    // Timer
  init_timer();
  
  // [1] Listen
  miro_state = Listen;
}


/*
 * STATE 2, 3, 4, 5
 */

void loop()
{
  // [2] Normal operation
  switch (miro_state)
  {
    case Listen:
      miro_state = Normal_op;
      break;

    case Normal_op:
      if (!mqtt_client->connected()) { mqtt_reconnect(); }
      mqtt_client->loop();

      td = t0 = millis();
      if (td - t0 > REST*5)
      {
        t0 = td;
        Serial.print("state ");
        Serial.println(miro_state);
      }
      break;

    case Transmit:
      break;

    case Deep_sleep:
      break;
    
    case Reset:
    default:
      ESP.restart();
  }
}


/*
 * FUNC DEF
 */

  // Initialize
void init_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wlan_ssid, wlan_psk);
  
  DEBUG(Serial.print("Connecting to Wi-Fi.."))
  
  while (WiFi.status() != WL_CONNECTED)
  {
    DEBUG(Serial.print('.'));
    delay(REST/6);
  }
  
  DEBUG(Serial.println(WiFi.localIP()))
}

void init_mqtt()
{
  mqtt_client = new PubSubClient(wifi_client);
  mqtt_client->setServer(mqtt_broker, mqtt_port);
  mqtt_client->setCallback(on_message);
}

void init_timer()
{
  timer_cycle = timerBegin(TIMER0, timer_divider, timer_count_up);
  timerAttachInterrupt(timer_cycle, &on_alarm_cycle, edge);
  timerAlarmWrite(timer_cycle, alarm_treshold, autoreload);
  timerAlarmEnable(timer_cycle);
  timerStop(timer_cycle);

  timer_span = timerBegin(TIMER1, timer_divider*5, timer_count_up);
  timerAttachInterrupt(timer_span, &on_alarm_span, edge);
  timerAlarmWrite(timer_span, alarm_treshold, autoreload);
  timerAlarmEnable(timer_span);
  timerStop(timer_span);
}

  // MQTT
void mqtt_reconnect()
{
  while (!mqtt_client->connected())
  {
    DEBUG(Serial.print("Reconnecting to MQTT broker... "))
    
    if (mqtt_client->connect("ESP32Client", mqtt_user, mqtt_pass))
    {
      DEBUG(Serial.println("connected."))
      mqtt_client->subscribe("admin/debug");
    }
    else
    {
      DEBUG(
        Serial.print("failed. rc=");
        Serial.println(mqtt_client->state());
        Serial.print("Retry in ");
        Serial.print(REST/1000);
        Serial.println(" seconds."))
      delay(REST);
    }
  }
}

void on_message(const char* topic, byte* msg, uint8_t len)
{
  Serial.print(topic);
  Serial.print(" | ");
  
  char* buf = (char*)malloc((len + 1)*sizeof(char));
  uint8_t i;
  for (i = 0; i < len; i++)
  {
    Serial.print((const char)*(msg + i));
    *(buf + i) = (const char)*(msg + i);
  }
  *(buf + i) = '\0';
  
  Serial.println();

  if (!strcmp(topic, "admin/debug"))
  {
    if(!strcmp(buf, "DOUBLE"))
    {
      timer_divider >>= 1;
    }
    else if(!strcmp(buf, "HALVE"))
    {
      timer_divider <<= 1;
    }
    else if(!strcmp(buf, "DEBUG"))
    {
      debug = !debug;
      Serial.print("Debug mode: ");
      Serial.println(debug);
    }
    timerSetDivider(timer_cycle, timer_divider);
  }

  free(buf);
}

  // HELPER
void parse_ip_to_string(const char* dest, uint8_t* ip) {
  snprintf((char*)dest, 16, "%d.%d.%d.%d\0", *(ip + 0), *(ip + 1), *(ip + 2), *(ip + 3));
}


/*
 * ISR
 */

void IRAM_ATTR ISR()
{
  if (miro_state == Reset)
  {
    held = false;
  }
  else if (held)
  {
    timerStop(timer_span);
    timerRestart(timer_span);
    switch_state++;
    digitalWrite(LED, led_state = LOW);

    timerStop(timer_cycle);
    timerRestart(timer_cycle);
    if (switch_state%3 != sw_normal)
    {
      if (switch_state%3 == sw_transmit)
      {
        timer_divider = TIMER_DIV >> 3;
      }
      else
      {
        timer_divider = TIMER_DIV >> 1;
      }
      timerSetDivider(timer_cycle, timer_divider);
      timerStart(timer_cycle);
    }

    held = false;
  }
  else
  {
    timerStop(timer_cycle);
    timerRestart(timer_cycle);
    timerStart(timer_span);
    digitalWrite(LED, led_state = HIGH);
    held = true;
  }
}

void IRAM_ATTR on_alarm_cycle()
{
  if (switch_state) { digitalWrite(LED, led_state = !led_state); }
}

void IRAM_ATTR on_alarm_span()
{
  digitalWrite(LED, led_state = LOW);
  timerStop(timer_span);
  timerRestart(timer_span);
  miro_state = Reset;
}