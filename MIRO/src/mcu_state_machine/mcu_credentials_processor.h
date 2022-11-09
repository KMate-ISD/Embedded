#include <cstdarg>
#include <Preferences.h>
#include "mcu_state_machine.h"


  // Credentials
typedef struct {
  size_t mqtt_port;
  uint8_t* mqtt_broker;
  char* mqtt_broker_str;
  char* mqtt_user;
  char* mqtt_pass;
  char* wlan_psk;
  char* wlan_ssid;
  uint8_t len_wlan_psk;
  uint8_t len_wlan_ssid;
} Credentials_t;

  // Credential handling
uint8_t check_if_preferences_has_keys(Preferences& pref, uint8_t args_count, ...)
{
  va_list args;
  uint8_t i;
  uint8_t log_val = 1;
  va_start(args, args_count);
  for (i = 0; i < args_count; i++)
  {
    log_val &= pref.isKey(va_arg(args, const char*));
  }
  va_end(args);
  return(log_val);
}

bool get_preferences(Preferences& pref, Credentials_t& creds)
{
  pref.begin("miro_creds", true);
  bool exist = check_if_preferences_has_keys(pref, 6,
    "mqtt_port",
    "mqtt_broker",
    "mqtt_user",
    "mqtt_pass",
    "wlan_psk",
    "wlan_ssid");

  if (exist)
  {
    creds.len_wlan_psk = pref.getUChar("len_psk");
    creds.len_wlan_ssid = pref.getUChar("len_ssid");

    creds.mqtt_broker = (uint8_t*)malloc(LEN_MQ_CREDS*sizeof(uint8_t));
    creds.mqtt_broker_str = (char*)malloc((LEN_MAX_IP + 1)*sizeof(char));
    creds.mqtt_user = (char*)malloc((LEN_MQ_CREDS + 1)*sizeof(char));
    creds.mqtt_pass = (char*)malloc((LEN_MQ_CREDS + 1)*sizeof(char));
    creds.wlan_psk = (char*)malloc((creds.len_wlan_psk)*sizeof(char));
    creds.wlan_ssid = (char*)malloc((creds.len_wlan_ssid)*sizeof(char));

    creds.mqtt_port = pref.getUShort("mqtt_port");
    pref.getBytes("mqtt_broker", creds.mqtt_broker, LEN_MAX_IP/4);
    parse_ip_to_string(creds.mqtt_broker_str, creds.mqtt_broker);
    pref.getString("mqtt_user", creds.mqtt_user, LEN_MQ_CREDS + 1);
    pref.getString("mqtt_pass", creds.mqtt_pass, LEN_MQ_CREDS + 1);
    pref.getString("wlan_psk", creds.wlan_psk, creds.len_wlan_psk);
    pref.getString("wlan_ssid", creds.wlan_ssid, creds.len_wlan_ssid);
  }
  
  pref.end();

  return(exist);
}

void set_preferences(Preferences& pref, Credentials_t& creds)
{
  pref.begin("miro_creds", false);

  pref.putUChar("len_psk", creds.len_wlan_psk);
  pref.putUChar("len_ssid", creds.len_wlan_ssid);
  pref.putUShort("mqtt_port", creds.mqtt_port);
  pref.putBytes("mqtt_broker", creds.mqtt_broker, LEN_MAX_IP/4);
  pref.putString("mqtt_user", creds.mqtt_user);
  pref.putString("mqtt_pass", creds.mqtt_pass);
  pref.putString("wlan_psk", creds.wlan_psk);
  pref.putString("wlan_ssid", creds.wlan_ssid);

  pref.end();
}

void print_preferences(Credentials_t& creds)
{
  Serial.print("Port\t");
  Serial.println(creds.mqtt_port);
  Serial.print("Broker\t");
  Serial.println(creds.mqtt_broker_str);
  Serial.print("User\t");
  Serial.println(creds.mqtt_user);
  Serial.print("Pass\t");
  Serial.println(creds.mqtt_pass);
  Serial.print("Psk\t");
  Serial.println(creds.wlan_psk);
  Serial.print("Ssid\t");
  Serial.println(creds.wlan_ssid);
}