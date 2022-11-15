#include <PubSubClient.h>
#include <WiFi.h>
#include "mcu_credentials_processor.h"
#include "mcu_tag_encoder_decoder_ul.h"
#include "mcu_state_machine.h"
#include "C:\Computer_science\Projects\Embedded\MIRO\src\mcu_relay\mcu_reed_relay.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


/*
 * GLOBAL
 */

  // Debug
bool old_btn_state          = LOW;
bool old_held               = false;
bool old_led_state          = false;
uint8_t old_miro_state      = Undefined;
uint8_t old_switch_state    = sw_normal;

  // Op.
bool held                   = false;            // Pushbutton is held
bool debug                  = true;             // Display detailed info via Serial
bool config_exists          = false;            // Config loaded from NVM
bool trigger_exists         = false;            // Trigger loaded from NVM
uint8_t miro_state          = Undefined;        // State of operation
uint8_t c                   = 0;                // General purpose counter
size_t t0                   = 0;                // Initial time
size_t td;                                      // Measured time

  // Peripherals
bool btn_state;
bool led_state              = false;            // Status of on board LED
uint8_t switch_state        = sw_normal;        // Toggle Normal op., Receive and Transmit states

  // Timer
bool autoreload             = true;
bool edge                   = true;
bool timer_count_up         = true;
uint64_t alarm_treshold     = TIMER_TRSH;
uint16_t timer_divider      = TIMER_DIV;
hw_timer_t* timer_cycle;
hw_timer_t* timer_span;

  // Network
WiFiClient wifi_client;
PubSubClient* mqtt_client;

  // NVM
Preferences preferences;
Credentials_processor proc(preferences);

  // RFID
MFRC522 rfid(RF_SS, RF_RST);
Tag_encoder_decoder ecdc(rfid);


/*
 * FUNC
 */

void hard_reset(void);
void init_mqtt(void);
void init_timer(void);
void init_wifi(void);
bool mqtt_connect(void);
void on_message(const char*, byte*, uint8_t);
void print_state(void);
void setup();
void loop();

void IRAM_ATTR ISR(void);
void IRAM_ATTR on_alarm_cycle(void);
void IRAM_ATTR on_alarm_span(void);


/*
 * STATE 0, 1
 */

void setup()
{
    // Disable brownout warning
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // [0] Begin with Initialize state
  miro_state = Initialize;

    // Initialize
  Serial.begin(BAUD_RATE);
  SPI.begin();
  ecdc.rfid.PCD_Init();
  DEBUG(debug, Serial.println())

    // NVM;
  if (config_exists = proc.load_preferences())
  {
    proc.print_creds();
    DEBUG(debug, Serial.println("Config loaded from NVS."))

      // Wifi
    init_wifi();
      // MQTT
    init_mqtt();
  }
  else
  {
    DEBUG(debug, Serial.println("No valid configuration found. Entering listening mode."))
  }

  if (trigger_exists = proc.load_trigger())
  {
    Serial.print("Trigger loaded from NVM: ");
    Serial.println(proc.trigger + 1);
  }
  else
  {
    Serial.println("Warning! No triggers found.");
  }

    // Peripherals
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);
#ifdef REED
  pinMode(REED_RELAY, INPUT);
  pinMode(REED_LED, OUTPUT);
  relay_status = digitalRead(REED_RELAY);
  digitalWrite(REED_LED, !relay_status);
#endif

    // Interrupts
  attachInterrupt(BTN, ISR, CHANGE);
  init_timer();
  
    // [1] Continue with Normal operation or Receive if config is broken
  if (config_exists)
  {
    miro_state = Normal_op;
  }
  else
  {
    switch_state = sw_receive;
    miro_state = Receive;
    switch_to_receive();
  }

    // So commands depending on time delta would run once after Init no matter what
  t0 = millis() - REST;
}


/*
 * STATE 2, 3, 4, 5
 */

void loop()
{
  // Common
  td = millis();
  DEBUG(debug, print_state())
  
  if (
    (miro_state != Reset
      || miro_state != Deep_sleep)
    && config_exists)
  {
    keep_mqtt_connected();
  }

  // [2] Normal operation
  switch (miro_state)
  {
    case Normal_op:
      if (relay_status != digitalRead(REED_RELAY)) { update_reed_status(debug, mqtt_client); }
      break;

    case Transmit:
      if (td - t0 > REST/3)
      {
        // config/data
        // config/trigger
        if (!(++c % 8)) { switch_to_normal(); }
        mqtt_client->publish("config/trigger", UQ_NODE);
        t0 = td;
      }
      break;

    case Receive:
      if (read_credentials())
      {
        Serial.println("Resetting node...\n");
        soft_reset();
      }
      break;

    case Deep_sleep:
      break;
    
    case Reset:
    default:
      hard_reset();
  }

  if (led_state != old_led_state)
  {
    digitalWrite(LED, old_led_state = led_state);
  }
}


/*
 * FUNC DEF
 */

  // Reset
void hard_reset()
{
  Serial.println("\n:: HARD RESET ::\n");

  proc.preferences.begin("miro_creds", RW_MODE);
  proc.preferences.clear();
  proc.preferences.end();

  soft_reset();
}

void soft_reset()
{
  timerStop(timer_cycle);
  timerRestart(timer_cycle);

  timerStop(timer_span);
  timerRestart(timer_span);

  digitalWrite(LED, led_state = LOW);
  ESP.restart();
}

  // Network
void init_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(proc.wlan_ssid, proc.wlan_psk);
  
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
  mqtt_client->setServer(proc.mqtt_broker, proc.mqtt_port);
  mqtt_client->setCallback(on_message);
}

bool keep_mqtt_connected()
{
  bool ret = false;

  if (!mqtt_client->connected() && td - t0 > REST)
  {
    if (ret = !mqtt_connect())
    {
      Serial.print("failed. rc=");
      Serial.println(mqtt_client->state());
      Serial.print("Retry in ");
      Serial.print(REST/1000);
      Serial.println(" seconds.");
    }
    t0 = td;
    ret = !ret;
  }
  else
  {
    ret = true;
    mqtt_client->loop();
  }

  return(ret);
}

bool mqtt_connect()
{
  Serial.print("Connecting to MQTT broker... ");
  bool ret = mqtt_client->connect("ESP32Client", proc.mqtt_user, proc.mqtt_pass);

  if (ret)
  {
    char* topic_auth = new char[10]();
    snprintf(topic_auth, 10, "auth/%s", proc.mqtt_user);
    DEBUG(debug, 
      Serial.print("Publishing IMALIVE message to ");
      Serial.println(topic_auth))
    mqtt_client->publish(topic_auth, "IMALIVE");
    delete[] topic_auth;

    mqtt_client->subscribe("admin/debug");
    mqtt_client->subscribe("config/data");
    mqtt_client->subscribe("config/trigger");
    mqtt_client->subscribe(UQ_TOPIC_TRIG);

    if (trigger_exists)
    {
      char* topic_trig = new char[8 + *proc.trigger]();
      memcpy(topic_trig, "trigger/", 8);
      memcpy(topic_trig + 8, proc.trigger + 1, *proc.trigger);
      mqtt_client->subscribe(topic_trig);
      DEBUG(debug, 
        Serial.print("Subscribed to ");
        Serial.println(topic_trig);
      )
      delete[] topic_trig;
    }

    Serial.print(proc.mqtt_user);
    Serial.println(" connected.");
  }

  return(ret);
}

  // Callback
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

  if (miro_state == Receive && !strcmp(topic, "config/trigger"))
  {
    if (trigger_exists)
    {
      char* topic_trig_old = (char*)malloc((8 + *proc.trigger)*sizeof(char));
      memcpy(topic_trig_old, "trigger/", 8);
      memcpy(topic_trig_old + 8, proc.trigger + 1, *proc.trigger);
      mqtt_client->unsubscribe(topic_trig_old);
      free(topic_trig_old);
    }

    proc.add_trigger(buf);
    DEBUG(debug, 
      Serial.print(proc.trigger + 1);
      Serial.print(" added as trigger. (length: ");
      Serial.print((uint8_t)*proc.trigger);
      Serial.println(")");
    )

    char* topic_trig = (char*)malloc((8 + *proc.trigger)*sizeof(char));
    memcpy(topic_trig, "trigger/", 8);
    memcpy(topic_trig + 8, proc.trigger + 1, *proc.trigger);
    mqtt_client->subscribe(topic_trig);
    DEBUG(debug, 
      Serial.print("Subscribed to ");
      Serial.println(topic_trig)
    )
    proc.save_trigger();
    DEBUG(debug, 
      Serial.print("New trigger saved to NVM: ");
      Serial.println(buf);
    )

    mqtt_client->publish(topic_trig, "GOTCHA");

    trigger_exists = true;
    switch_to_normal();
    free(topic_trig);
  }

  if (miro_state == Transmit && !strcmp(topic, UQ_TOPIC_TRIG))
  {
    if (!strcmp(buf, "GOTCHA"))
    {
      switch_to_normal();
    }
  }

  if (!strcmp(topic, "admin/debug"))
  {
    if (!strcmp(buf, "DEBUG"))
    {
      debug = !debug;
      Serial.print("Debug mode: ");
      Serial.println(debug);
    }
    else if (debug && !strcmp(buf, "NVMCLEAR"))
    {
      proc.preferences.begin("miro_creds", RW_MODE);
      proc.preferences.clear();
      proc.preferences.end();
      Serial.println("Preferences cleared.");
    }
    else if (debug && !strcmp(buf, "GETTRIG"))
    {
      Serial.print(proc.trigger + 1);
      Serial.print(" //length: ");
      Serial.print((uint8_t)*proc.trigger);
    }
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

  // RFID
bool read_credentials()
{
  bool data_structure_appropriate = 0;
  
  if (rfid.PICC_IsNewCardPresent()) // New tag in proximity of the reader
  {
    if (rfid.PICC_ReadCardSerial()) // NUID read
    {
      // Read lines
      uint8_t start_block = SECTOR_START;
      uint8_t stop_block = BLOCK_COUNT;
      uint8_t len_data = (stop_block - start_block)*BLOCK_SIZE;
      uint8_t* data = (uint8_t*)malloc(len_data*sizeof(uint8_t));
      ecdc.read_blocks(data, start_block, stop_block);
      Serial.println();
      DEBUG(debug, ecdc.print_hex(data, len_data))

      if (data_structure_appropriate = proc.check_and_decode(data))
      {
        proc.save_preferences();
        DEBUG(debug, Serial.println("Preferences saved to NVS."))
      }

      // Safe close
      ecdc.end_op();
      free(data);
    }
  }

  return(data_structure_appropriate);
}


/*
 * ISR
 */

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

void start_timer_cycle()
{
  timerSetDivider(timer_cycle, timer_divider);
  timerRestart(timer_cycle);
  timerStart(timer_cycle);
}

void switch_to_normal()
{
  timerStop(timer_cycle);
  c = 0;
  led_state = LOW;
  switch_state = sw_normal;
  miro_state = Normal_op;
}

void switch_to_receive()
{
  timer_divider = TIMER_DIV >> 3;
  start_timer_cycle();
}

void switch_to_transmit()
{
  timer_divider = TIMER_DIV >> 1;
  start_timer_cycle();
}

void IRAM_ATTR ISR()
{
  if (config_exists)
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
          switch_to_transmit();
        }
        else
        {
          switch_to_receive();
        }
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
}

void IRAM_ATTR on_alarm_cycle()
{
  if (switch_state) { led_state = !led_state; }
}

void IRAM_ATTR on_alarm_span()
{
  miro_state = Reset;
}