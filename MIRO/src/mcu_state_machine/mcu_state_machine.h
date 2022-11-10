/*
 #  STATE             TRIGGER
 0  Initialize        power on
 1  Receive           Initialize phase ends while no credentials are saved / On board flash button released while in Normal operational state.
 2  Normal operation  Connection established
 3  Transmit          On-board flash button released while in Receive state
 4  Reset             On-board flash button pressed and held for 5 seconds
 5  Deep sleep        Receive phase ends while no credentials are saved
 */


/*
 * MACRO
 */

#define BTN                   0
#define LED                   2
#define RF_RST                22
#define RF_SS                 21
#define TIMER0                0
#define TIMER1                1

#define DEBUG(a)              if (debug) { a; }


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


  // HELPER
void parse_ip_to_string(const char* dest, uint8_t* ip) {
  snprintf((char*)dest, 16, "%d.%d.%d.%d\0", *(ip + 0), *(ip + 1), *(ip + 2), *(ip + 3));
}

char* parse_ip_to_string(uint8_t* ip) {
  char* ip_str = new char[16]();
  snprintf(ip_str, 16, "%d.%d.%d.%d", *(ip + 0), *(ip + 1), *(ip + 2), *(ip + 3));
  return(ip_str);
}