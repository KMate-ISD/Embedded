#include "mcu_credentials_processor.h"
#include "mcu_tag_encoder_decoder_ul.h"
#include "mcu_state_machine.h"


Credentials_processor::Credentials_processor(Preferences& preferences)
  : preferences { preferences }
{
  this->alloc_mem_mqtt_creds();
  this->treshold = 0;
  this->field = 0;
}

Credentials_processor::~Credentials_processor()
{
  if (this->len_wlan_ssid) { delete[] this->wlan_ssid; }
  if (this->len_wlan_psk) { delete[] this->wlan_psk; }
  delete[] this->mqtt_broker;
  delete[] this->mqtt_user;
  delete[] this->mqtt_pass;
  delete[] this->trigger;
}

bool Credentials_processor::check_and_decode(uint8_t* data)
{
  /*
   * mqtt user    len = 4
   * mqtt pass    len = 4
   * 0xE0         ssid start signal
   * ssid len
   * ssid         len = dynamic
   * 0xED         psk start signal
   * psk len
   * psk          len = dynamic
   * 0xEA         wlan end signal
   * wlan len     ssid len + psk len
   * broker ip    len = 4
   * port         len = 2
   * 0xDB         conclude signal
   */

  char sig_ssid_start   = 0xE0;
  char sig_psk_start    = 0xED;
  char sig_wlan_end     = 0xEA;
  char sig_full_stop    = 0xDB;

  uint8_t* p_sig_ssid_start;
  uint8_t* p_sig_psk_start;
  uint8_t* p_sig_wlan_end;
  uint8_t* p_len_ssid;
  uint8_t* p_len_psk;

  bool ret = false;

  p_sig_ssid_start = data + 8;
  if (*p_sig_ssid_start != sig_ssid_start) { return(ret); }
  p_len_ssid = p_sig_ssid_start + 1;

  p_sig_psk_start = p_len_ssid + *p_len_ssid + 1;
  if (*p_sig_psk_start != sig_psk_start) { return(ret); }
  p_len_psk = p_sig_psk_start + 1;

  p_sig_wlan_end = p_len_psk + *p_len_psk + 1;
  if (*p_sig_wlan_end != sig_wlan_end) { return(ret); }

  if (*(p_sig_wlan_end + 1) != *p_len_ssid + *p_len_psk) { return(ret); }

  if (*(p_sig_wlan_end + 8) == sig_full_stop) { ret = true; }

  if (ret)
  {
    this->set_wifi_creds((char*)(p_sig_ssid_start + 2), *p_len_ssid, (char*)(p_sig_psk_start + 2), *p_len_psk);
    this->set_mqtt_creds((char*)data, (char*)(data + 4));
    this->set_mqtt_server(p_sig_wlan_end + 2, *(p_sig_wlan_end + 6)*256 + *(p_sig_wlan_end + 7));
    this->print_creds();
  }

  return(ret);
}

uint8_t Credentials_processor::check_if_preferences_has_keys(uint8_t args_count, ...)
{
  uint8_t i;
  uint8_t log_val = 1;

  va_list args;
  va_start(args, args_count);
  for (i = 0; i < args_count; i++)
  {
    log_val &= this->preferences.isKey(va_arg(args, const char*));
  }
  va_end(args);

  return(log_val);
}

void Credentials_processor::alloc_mem_mqtt_creds()
{
  this->mqtt_broker = new uint8_t[4];
  this->mqtt_user = new char[LEN_MQ_CREDS + 1]();
  this->mqtt_pass = new char[LEN_MQ_CREDS + 1]();
}

void Credentials_processor::alloc_mem_wifi_creds(uint8_t len_wlan_ssid, uint8_t len_wlan_psk)
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
  alloc_mem_wifi_creds(len_wlan_ssid, len_wlan_psk);
  memcpy(this->wlan_ssid, wlan_ssid, this->len_wlan_ssid);
  memcpy(this->wlan_psk, wlan_psk, this->len_wlan_psk);
}

void Credentials_processor::set_field(uint8_t field)
{
  this->field = field;
}

size_t Credentials_processor::load_field()
{
  this->preferences.begin("miro_creds", RO_MODE);
  this->field = this->preferences.getShort("field");
  this->preferences.end();
  return(this->field);
}

void Credentials_processor::save_field()
{
  this->preferences.begin("miro_creds", RW_MODE);
  this->preferences.putShort("field", this->field);
  this->preferences.end();
}

void Credentials_processor::set_treshold(int8_t direction, size_t treshold)
{
  this->direction = direction;
  this->treshold = treshold;
}

size_t Credentials_processor::load_treshold()
{
  this->preferences.begin("miro_creds", RO_MODE);
  this->direction = this->preferences.getChar("direction");
  this->treshold = this->preferences.getShort("treshold");
  this->preferences.end();
  return(this->treshold);
}

void Credentials_processor::save_treshold()
{
  this->preferences.begin("miro_creds", RW_MODE);
  this->preferences.putChar("direction", this->direction);
  this->preferences.putShort("treshold", this->treshold);
  this->preferences.end();
}

void Credentials_processor::add_trigger(char* trigger)
{
  delete[] this->trigger;
  
  int i = 0;
  while (*(trigger + i++)) { }

  this->trigger = new char[i + 1]();
  *(this->trigger) = i;
  memcpy(this->trigger + 1, trigger, i);
}

bool Credentials_processor::load_trigger()
{
  this->preferences.begin("miro_creds", RO_MODE);

  bool exist = this->preferences.isKey("trigger");
  if (exist)
  {
    uint8_t len = this->preferences.getUChar("len_trigger");
    this->trigger = new char[len + 1]();
    this->preferences.getString("trigger", this->trigger + 1, len);
    *(this->trigger) = len;
  }

  this->preferences.end();
  return(exist);
}

void Credentials_processor::save_trigger()
{
  this->preferences.begin("miro_creds", RW_MODE);
  this->preferences.putString("trigger", this->trigger + 1);
  this->preferences.putUChar("len_trigger", *(this->trigger));
  this->preferences.end();
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
    this->alloc_mem_wifi_creds(this->preferences.getUChar("len_ssid"), this->preferences.getUChar("len_psk"));

    this->mqtt_port = this->preferences.getUShort("mqtt_port");
    this->preferences.getBytes("mqtt_broker", this->mqtt_broker, 4);
    this->preferences.getString("mqtt_user", this->mqtt_user, LEN_MQ_CREDS + 1);
    this->preferences.getString("mqtt_pass", this->mqtt_pass, LEN_MQ_CREDS + 1);
    this->preferences.getString("wlan_psk", this->wlan_psk, this->len_wlan_psk + 1);
    this->preferences.getString("wlan_ssid", this->wlan_ssid, this->len_wlan_ssid + 1);
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
}

void Credentials_processor::print_creds()
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