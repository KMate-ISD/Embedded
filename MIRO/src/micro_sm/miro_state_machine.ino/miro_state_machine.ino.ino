/*
 #  STATE             TRIGGER
 0  Initialize        power on
 1  Listen            Initialize phase ends
 2  Normal operation  Connection established
 3  Transmit          On-board flash button voltage rising AND not in Halt state
 4  Halt              On-board flash button held for at least 1 sec
 5  Deep sleep        Listen phase ends AND no credentials are saved
 */


#include <Preferences.h>
#include <PubSubClient.h>
#include <WiFi.h>


/*
 * CONST
 */
 

/*
 * GLOBALS
 */
uint8_t miro_state;
uint16_t mqtt_port;
size_t t0;
const char* mqtt_broker;
const char* mqtt_pass;
const char* mqtt_user;
const char* wlan_ssid;
const char* wlan_psk;
Preferences preferences;
WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);


void setup() {
  // [0] Initialize
  miro_state  = 0;
  wlan_ssid   = "Telekom-B4Wf5Y";
  wlan_psk    = "PIcSafaSZ_+9*";
  mqtt_port   = 1883;
  mqtt_user   = "user";
  mqtt_pass   = "pass";
  mqtt_broker = (const char*)malloc(16*sizeof(char*));
  uint8_t mqtt_broker_bytes[4] = {192, 168, 1, 85};
  parse_ip_to_string(mqtt_broker, mqtt_broker_bytes);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  
  // [0] Connect to WLAN
  init_wifi();
  
  // [0] Connect to MQTT
  init_mqtt();
  
  // [1] Listen
}


void loop() {
  // [2] Normal operation
  if (!mqtt_client.connected())
  {
    mqtt_reconnect();
  }
  mqtt_client.loop();
  

  size_t t = millis();
  if (t - t0 > 5000)
  {
    t0 = t;
    Serial.println("state 2");
  }
}

void init_mqtt()
{
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(on_message);
}

void init_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wlan_ssid, wlan_psk);
  
  Serial.print("Connecting to Wi-Fi ..");
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  
  Serial.println(WiFi.localIP());
}

void parse_ip_to_string(const char* dest, uint8_t* ip) {
  snprintf((char*)dest, 16, "%d.%d.%d.%d\0", *(ip + 0), *(ip + 1), *(ip + 2), *(ip + 3));
}

void mqtt_reconnect()
{
  while (!mqtt_client.connected())
  {
    Serial.print("Reconnecting to MQTT broker... ");
    
    if (mqtt_client.connect("ESP32Client", mqtt_user, mqtt_pass))
    {
      Serial.println("connected.");
      mqtt_client.subscribe("admin/debug");
    }
    else
    {
      Serial.print("failed. rc=");
      Serial.print(mqtt_client.state());
      Serial.println("Retry in 5 seconds.");
      delay(5000);
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
    if(!strcmp(buf, "on"))
    {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if(!strcmp(buf, "off"))
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  free(buf);
}
