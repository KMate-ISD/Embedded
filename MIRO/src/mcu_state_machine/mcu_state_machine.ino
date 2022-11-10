#include <PubSubClient.h>
#include <WiFi.h>
#include "mcu_credentials_processor.h"


/*
 * GLOBAL
 */

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
bool held                   = false;
bool debug                  = true;
uint8_t miro_state          = Undefined;
size_t t0                   = 0;
size_t td;

  // Peripherals
bool btn_state;
bool led_state              = false;
uint8_t switch_state        = sw_normal;

  // Timer
bool autoreload             = true;
bool edge                   = true;
bool timer_count_up         = true;
uint64_t alarm_treshold     = TIMER_TRSH;
uint16_t timer_divider      = TIMER_DIV;
hw_timer_t* timer_cycle;
hw_timer_t* timer_span;

  // NVM
Preferences preferences;
Credentials_processor proc(preferences);

  // Debug
bool old_btn_state          = LOW;
bool old_held               = false;
bool old_led_state          = false;
uint8_t old_miro_state      = Undefined;
uint8_t old_switch_state    = sw_normal;


/*
 * FUNC
 */

void init_wifi(void);
void init_mqtt(void);
void init_timer(void);
void mqtt_reconnect(void);

void on_message(const char*, byte*, uint8_t);
void parse_ip_to_string(const char*, uint8_t*);
void print_state(void);
uint8_t check_if_preferences_has_keys(uint8_t, void*);

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

    // NVM;
  bool exist = proc.print_creds();

    // WLAN
  wlan_ssid   = "Telekom-B4Wf5Y";
  wlan_psk    = "PIcSafaSZ_+9*";
  init_wifi();

    // MQTT
  mqtt_port   = 1883;
  mqtt_user   = "user";
  mqtt_pass   = "pass";
  mqtt_broker = (const char*)malloc(LEN_MAX_IP*sizeof(char));
  uint8_t mqtt_broker_bytes[LEN_MAX_IP/4] = {192, 168, 1, 85};
  parse_ip_to_string(mqtt_broker, mqtt_broker_bytes);
  init_mqtt();

  if (!exist)
  {
    proc.set_mqtt_server(mqtt_broker_bytes, mqtt_port);
    proc.set_mqtt_creds((char*)mqtt_user, (char*)mqtt_pass);
    proc.set_wifi_creds((char*)wlan_ssid, 15, (char*)wlan_psk, 14);
  }

    // Peripherals
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);
  attachInterrupt(BTN, ISR, CHANGE);

    // Timer
  init_timer();
  
  // [1] Continue with normal operation 
  miro_state = Normal_op;
}


/*
 * STATE 2, 3, 4, 5
 */

void loop()
{
  // [2] Normal operation
  switch (miro_state)
  {
    case Normal_op:
      if (!mqtt_client->connected()) { mqtt_reconnect(); }
      mqtt_client->loop();

      break;

    case Transmit:
      break;

    case Receive:
      break;

    case Deep_sleep:
      break;
    
    case Reset:
    default:
      Serial.println("\n-= HARD RESET =-\n");
      DEBUG(print_state())

      timerStop(timer_cycle);
      timerRestart(timer_cycle);

      timerStop(timer_span);
      timerRestart(timer_span);

      digitalWrite(LED, led_state = LOW);
      ESP.restart();
  }

  td = millis();
  if (td - t0 > REST*4)
  {
    t0 = td;
    Serial.print("\nstate ");
    Serial.println(miro_state);
  }

  if (led_state != old_led_state)
  {
    digitalWrite(LED, old_led_state = led_state);
  }

  DEBUG(print_state())
}


/*
 * FUNC DEF
 */

  // Initialize
void init_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wlan_ssid, wlan_psk);
  
  Serial.print("Connecting to Wi-Fi..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(REST/6);
  }
  Serial.print(' ');
  Serial.println(WiFi.localIP());
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
    Serial.print("Reconnecting to MQTT broker... ");
    
    if (mqtt_client->connect("ESP32Client", mqtt_user, mqtt_pass))
    {
      Serial.println("connected.");
      mqtt_client->subscribe("admin/debug");
    }
    else
    {
      Serial.print("failed. rc=");
      Serial.println(mqtt_client->state());
      Serial.print("Retry in ");
      Serial.print(REST/1000);
      Serial.println(" seconds.");
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
    if(!strcmp(buf, "DEBUG"))
    {
      debug = !debug;
      Serial.print("Debug mode: ");
      Serial.println(debug);
    }
    timerSetDivider(timer_cycle, timer_divider);
  }

  free(buf);
}

  // Debug
void print_state()
{
  btn_state = digitalRead(BTN);
  if (
    old_btn_state != btn_state
    || old_held != held
    || old_miro_state != miro_state
    || old_switch_state != switch_state)
  {
    Serial.print("\ngpio0\t");
    Serial.println(btn_state);
    Serial.print("held\t");
    Serial.println(held);
    Serial.print("state\t");
    Serial.println(miro_state);
    Serial.print("switch\t");
    Serial.println(switch_state%3);

    Serial.print("timer_cycle started ");
    Serial.print("\t");
    Serial.print(timerStarted(timer_cycle));
    Serial.print("\t");
    Serial.println(timerRead(timer_cycle));
    Serial.print("timer span started ");
    Serial.print("\t");
    Serial.print(timerStarted(timer_span));
    Serial.print("\t");
    Serial.println(timerRead(timer_span));

    old_btn_state = btn_state;
    old_held = held;
    old_miro_state = miro_state;
    old_switch_state = switch_state;

    Serial.println();
  }
}


/*
 * ISR
 */

void IRAM_ATTR ISR()
{
  if (held)
  {
    led_state = LOW;
    switch_state++;

    timerStop(timer_span);

    if (switch_state%3 != sw_normal)
    {
      if (switch_state%3 == sw_transmit)
      {
        timer_divider = TIMER_DIV >> 1;
      }
      else
      {
        timer_divider = TIMER_DIV >> 3;
      }
      timerSetDivider(timer_cycle, timer_divider);
      timerRestart(timer_cycle);
      timerStart(timer_cycle);
    }

    held = false;
    miro_state = switch_state%3 + 1;
  }
  else
  {
    timerStop(timer_cycle);

    timerRestart(timer_span);
    timerStart(timer_span);

    held = true;
    led_state = HIGH;
  }
}

void IRAM_ATTR on_alarm_cycle()
{
  if (switch_state) { led_state = !led_state; }
}

void IRAM_ATTR on_alarm_span()
{
  miro_state = Reset;
}