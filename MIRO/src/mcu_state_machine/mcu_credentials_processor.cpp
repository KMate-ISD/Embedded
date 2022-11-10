#include "mcu_credentials_processor.h"


Credentials_processor::Credentials_processor(Preferences& preferences)
  : preferences { preferences }
{
  this->alloc_mqtt_creds();
}

Credentials_processor::~Credentials_processor()
{
  if (this->len_wlan_ssid) { delete[] this->wlan_ssid; }
  if (this->len_wlan_psk) { delete[] this->wlan_psk; }
  delete[] this->mqtt_broker;
  delete[] this->mqtt_user;
  delete[] this->mqtt_pass;
}

uint8_t Credentials_processor::check_if_preferences_has_keys(uint8_t args_count, ...)
{
  va_list args;
  uint8_t i;
  uint8_t log_val = 1;
  va_start(args, args_count);
  for (i = 0; i < args_count; i++)
  {
    log_val &= preferences.isKey(va_arg(args, const char*));
  }
  va_end(args);
  return(log_val);
}

void Credentials_processor::alloc_mqtt_creds()
{
  this->mqtt_broker = new uint8_t[4];
  this->mqtt_user = new char[LEN_MQ_CREDS + 1]();
  this->mqtt_pass = new char[LEN_MQ_CREDS + 1]();
}

void Credentials_processor::alloc_wifi_creds(uint8_t len_wlan_ssid, uint8_t len_wlan_psk)
{
  if (this->len_wlan_ssid) { delete[] this->wlan_ssid; }
  if (this->len_wlan_psk) { delete[] this->wlan_psk; }
  this->len_wlan_ssid = len_wlan_ssid;
  this->len_wlan_psk = len_wlan_psk;
  this->wlan_ssid = new char[this->len_wlan_ssid + 1]();
  this->wlan_psk = new char[this->len_wlan_psk + 1]();
}

void Credentials_processor::set_mqtt_server(uint8_t* mqtt_broker_bytes, uint16_t mqtt_port)
{
  memcpy(this->mqtt_broker, mqtt_broker_bytes, 4);
  this->mqtt_port = mqtt_port;
}

void Credentials_processor::set_mqtt_creds(char* mqtt_user, char* mqtt_pass)
{
  memcpy(this->mqtt_user, mqtt_user, LEN_MQ_CREDS);
  memcpy(this->mqtt_pass, mqtt_pass, LEN_MQ_CREDS);
}

void Credentials_processor::set_wifi_creds(char* wlan_ssid, uint8_t len_wlan_ssid, char* wlan_psk, uint8_t len_wlan_psk)
{
  alloc_wifi_creds(len_wlan_ssid, len_wlan_psk);
  memcpy(this->wlan_ssid, wlan_ssid, this->len_wlan_ssid);
  memcpy(this->wlan_psk, wlan_psk, this->len_wlan_psk);
}

bool Credentials_processor::load_preferences()
{
  this->preferences.begin("miro_creds", RO_MODE);
  bool exist = this->check_if_preferences_has_keys(6,
    "mqtt_port",
    "mqtt_broker",
    "mqtt_user",
    "mqtt_pass",
    "wlan_psk",
    "wlan_ssid");

  if (exist)
  {
    this->alloc_wifi_creds(this->preferences.getUChar("len_ssid"), this->preferences.getUChar("len_psk"));

    this->mqtt_port = this->preferences.getUShort("mqtt_port");
    this->preferences.getBytes("mqtt_broker", this->mqtt_broker, 4);
    this->preferences.getString("mqtt_user", this->mqtt_user, LEN_MQ_CREDS + 1);
    this->preferences.getString("mqtt_pass", this->mqtt_pass, LEN_MQ_CREDS + 1);
    this->preferences.getString("wlan_psk", this->wlan_psk, this->len_wlan_psk + 1);
    this->preferences.getString("wlan_ssid", this->wlan_ssid, this->len_wlan_ssid + 1);

    Serial.println("Preferences loaded.");
  }
  else
  {
    Serial.println("No preferences to load.");
  }
  
  this->preferences.end();

  return(exist);
}

void Credentials_processor::save_preferences()
{
  this->preferences.begin("miro_creds", RW_MODE);

  this->preferences.putUChar("len_psk", this->len_wlan_psk);
  this->preferences.putUChar("len_ssid", this->len_wlan_ssid);
  this->preferences.putUShort("mqtt_port", this->mqtt_port);
  this->preferences.putBytes("mqtt_broker", this->mqtt_broker, 4);
  this->preferences.putString("mqtt_user", this->mqtt_user);
  this->preferences.putString("mqtt_pass", this->mqtt_pass);
  this->preferences.putString("wlan_psk", this->wlan_psk);
  this->preferences.putString("wlan_ssid", this->wlan_ssid);

  this->preferences.end();

  Serial.println("Preferences saved to NVS.");
}

bool Credentials_processor::print_creds()
{
  bool exist = this->load_preferences();

  if (exist)
  {
    char* ip_str = new char[16]();
    parse_ip_to_string(ip_str, this->mqtt_broker);

    Serial.print("Port\t");
    Serial.println(this->mqtt_port);
    Serial.print("Broker\t");
    Serial.println(ip_str);
    Serial.print("User\t");
    Serial.println(this->mqtt_user);
    Serial.print("Pass\t");
    Serial.println(this->mqtt_pass);
    Serial.print("Psk\t");
    Serial.println(this->wlan_psk);
    Serial.print("Ssid\t");
    Serial.println(this->wlan_ssid);

    delete[] ip_str;
  }

  return(exist);
}