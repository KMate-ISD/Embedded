#ifndef CREDENTIALS_PROCESSOR
#define CREDENTIALS_PROCESSOR

#include <cstdarg>
#include <Preferences.h>


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
    
    int8_t field;
    int8_t direction;
    size_t treshold;
    char* trigger;

    Preferences preferences;

    Credentials_processor(Preferences&);
    ~Credentials_processor();
    
    bool check_and_decode(uint8_t*);
    uint8_t check_if_preferences_has_keys(uint8_t, ...);
    void alloc_mem_mqtt_creds(void);
    void alloc_mem_wifi_creds(uint8_t, uint8_t);
    void set_mqtt_server(uint8_t*, uint16_t);
    void set_mqtt_creds(char*, char*);
    void set_wifi_creds(char*, uint8_t, char*, uint8_t);
    void set_field(uint8_t);
    size_t load_field();
    void save_field();
    void set_treshold(int8_t, size_t);
    size_t load_treshold();
    void save_treshold();
    void add_trigger(char*);
    bool load_trigger();
    void save_trigger();
    bool load_preferences(void);
    void save_preferences(void);
    void print_creds(void);
};

#endif // CREDENTIALS_PROCESSOR