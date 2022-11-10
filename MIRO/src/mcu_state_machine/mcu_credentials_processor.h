#ifndef CREDENTIALS_PROCESSOR
#define CREDENTIALS_PROCESSOR

#include <cstdarg>
#include <Preferences.h>
#include "mcu_state_machine.h"


class Credentials_processor
{
  public:
    uint16_t mqtt_port;
    uint8_t* mqtt_broker;
    char* mqtt_user;
    char* mqtt_pass;

    uint8_t len_wlan_psk;
    uint8_t len_wlan_ssid;
    char* wlan_psk;
    char* wlan_ssid;

    Preferences preferences;

    Credentials_processor(Preferences& preferences);
    ~Credentials_processor();
    
    uint8_t check_if_preferences_has_keys(uint8_t args_count, ...);
    void alloc_mqtt_creds();
    void alloc_wifi_creds(uint8_t, uint8_t);
    void set_mqtt_server(uint8_t*, uint16_t);
    void set_mqtt_creds(char*, char*);
    void set_wifi_creds(char*, uint8_t, char*, uint8_t);
    bool load_preferences(void);
    void save_preferences(void);
    bool print_creds(void);
};

#endif // CREDENTIALS_PROCESSOR