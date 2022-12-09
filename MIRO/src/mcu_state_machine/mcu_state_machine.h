#ifndef STATE_MACHINE
#define STATE_MACHINE

/*
 #    STATE             TRIGGER
 0    Initialize        power on
 1.0  Receive           Initialize phase ends while no credentials are saved / On board flash button released while in Normal operational state.
 1.1  Listen            This is the exact same as Receive in function, but switching out of this state is prohibited (reset possible)
 2    Normal operation  Connection established
 3    Transmit          On-board flash button released while in Receive state
 4    Reset             On-board flash button pressed and held for 5 seconds
 5    Deep sleep        Receive phase ends while no credentials are saved
 */


/*
 * MACRO
 */

#define PTX

#ifdef CAM
  #define SCK                 14
  #define MISO                12
  #define MOSI                13
  #define SS                  15
  #define RST                 0
  #define LED                 4
  #define BTN                 2
#else
  #define RF_RST              22
  #define RF_SS               21
  #define LED                 2
  #define BTN                 0
#endif

#define TIMER0                0
#define TIMER1                1
#define RW_MODE               false
#define RO_MODE               true

#define DEBUG(a, b)           if (a) { b; }

// Unique per node

#ifdef REED
#define UQ_NODE               "REED"
#define UQ_TOPIC_CONF         "config/REED"
#define UQ_TOPIC_DATA         "data/REED"
#define UQ_TOPIC_TRIG         "trigger/REED"
#define REED_RELAY            32
#define REED_LED              33
#elif defined(CAM)
#define UQ_NODE               "CAM"
#define UQ_TOPIC_CONF         "config/CAM"
#define UQ_TOPIC_DATA         "data/CAM"
#define UQ_TOPIC_TRIG         "trigger/CAM"
#elif defined(SDLEV)
#define UQ_NODE               "SDLEV"
#define UQ_TOPIC_CONF         "config/SDLEV"
#define UQ_TOPIC_DATA         "data/SDLEV"
#define UQ_TOPIC_TRIG         "trigger/SDLEV"
#elif defined(PTX)
#define UQ_NODE               "PTX"
#define UQ_TOPIC_CONF         "config/PTX"
#define UQ_TOPIC_DATA         "data/PTX"
#define UQ_TOPIC_TRIG         "trigger/PTX"
#define PTX_IN                36
#endif


  // const
const uint8_t LEN_MQ_CREDS  = 4;
const uint8_t LEN_MAX_IP    = 16;
const size_t BAUD_RATE      = 9600;
const size_t REST           = 6000;
const size_t TIMER_DIV      = 80;
const size_t TIMER_TRSH     = 500000;


/*
 * ENUM
 */

enum State
{
  Initialize,
  Normal_op,
  Transmit,
  Receive,
  Reset,
  Deep_sleep,
  Undefined = 0xFF
};

enum Switch_state
{
  sw_normal,
  sw_transmit,
  sw_receive
};

enum CN_type
{
  cn_default,
  cn_trigger,
  cn_action,
  cn_data
};


  // HELPER
inline void parse_ip_to_string(const char* dest, uint8_t* ip)
{
  snprintf((char*)dest, 16, "%d.%d.%d.%d\0", *(ip + 0), *(ip + 1), *(ip + 2), *(ip + 3));
}

#endif // STATE_MACHINE
