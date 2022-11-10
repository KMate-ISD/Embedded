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


/*
 * FUNC
 */

void init_wifi(void);
void init_mqtt(void);
void init_timer(void);
void mqtt_reconnect(void);

void on_message(const char*, byte*, uint8_t);
void parse_ip_to_string(const char*, uint8_t*);
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

    // NVM
  Credentials_t creds;

  bool exist = get_preferences(preferences, creds);
  Serial.print("Valid preferences found: ");
  Serial.println(exist);

  print_preferences(creds);

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
    creds.len_wlan_psk = 13 + 1;
    creds.len_wlan_ssid = 14 + 1;

    creds.mqtt_broker = (uint8_t*)malloc(LEN_MQ_CREDS*sizeof(uint8_t));
    creds.mqtt_broker_str = (char*)malloc((LEN_MAX_IP + 1)*sizeof(char));
    creds.mqtt_user = (char*)malloc((LEN_MQ_CREDS + 1)*sizeof(char));
    creds.mqtt_pass = (char*)malloc((LEN_MQ_CREDS + 1)*sizeof(char));
    creds.wlan_psk = (char*)malloc((creds.len_wlan_psk)*sizeof(char));
    creds.wlan_ssid = (char*)malloc((creds.len_wlan_ssid)*sizeof(char));

    creds.mqtt_port = mqtt_port;
    memcpy(creds.mqtt_broker, mqtt_broker_bytes, LEN_MAX_IP/4);
    memcpy(creds.mqtt_user, mqtt_user, LEN_MQ_CREDS + 1);
    memcpy(creds.mqtt_pass, mqtt_pass, LEN_MQ_CREDS + 1);
    memcpy(creds.wlan_psk, wlan_psk, creds.len_wlan_psk);
    memcpy(creds.wlan_ssid, wlan_ssid, creds.len_wlan_ssid);
  }
  
    // Creds
  if (!exist)
  {
    set_preferences(preferences, creds);
    Serial.println("Preferences set.");
  }

    // Peripherals
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  attachInterrupt(BTN, ISR, CHANGE);

    // Timer
  init_timer();
  
  // [1] Continue with normal operation 
  miro_state = Normal_op;
}


/*
 * STATE 2, 3, 4, 5
 */

bool old_btn_state = digitalRead(BTN);
bool old_held = held;
bool old_led_state = led_state;
uint8_t old_state = miro_state;
uint8_t old_switch_state = switch_state; 

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
      ESP.restart();
  }

  // td = millis();
  // if (td - t0 > REST/100)
  // {
  //   t0 = td;
  // }
  if (old_led_state != led_state)
  {
    digitalWrite(LED, led_state);
    old_led_state = led_state;
  }

  DEBUG(
    bool btn_state = digitalRead(BTN);
    if (
      old_state != miro_state
      || old_held != held
      || old_switch_state%3 != switch_state%3
      || old_btn_state != btn_state)
    {
      Serial.print("\nBTN ");
      Serial.println(btn_state);
      Serial.print("state ");
      Serial.println(miro_state);
      Serial.print("held ");
      Serial.println(held);
      Serial.print("switch ");
      Serial.println(switch_state%3);

      old_state = miro_state;
      old_held = held;
      old_switch_state = switch_state;
      old_btn_state = btn_state;
    }
  )
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
    DEBUG(Serial.print('.'))
    delay(REST/6);
  }
  DEBUG(Serial.print(' '))
  
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

  timer_span = timerBegin(TIMER1, timer_divider*6, timer_count_up);
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
    held = false;
    switch_state++;
    timerStop(timer_cycle);
    timerRestart(timer_cycle);
    led_state = LOW;

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
      timerStart(timer_cycle);
    }

    timerStop(timer_span);
    timerRestart(timer_span);
    miro_state = switch_state%3 + 1;
  }
  else
  {
    timerStop(timer_cycle);
    timerRestart(timer_cycle);
    timerStart(timer_span);
    led_state = HIGH;
    held = true;
  }
}

void IRAM_ATTR on_alarm_cycle()
{
  if (switch_state) { led_state = !led_state; }
}

void IRAM_ATTR on_alarm_span()
{
  // mby check gpio 0 before to fuck up the rare bug
  timerStop(timer_span);
  timerRestart(timer_span);
  led_state = LOW;
  if (!digitalRead(BTN))
  {
    miro_state = Reset; 
  }
  else
  {
    held = false;
    ISR();
  }
}