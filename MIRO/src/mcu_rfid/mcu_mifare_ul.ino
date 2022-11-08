#include <MFRC522.h>    // RFID-RC522 module
#include <SPI.h>        // SPI bus

/* Wiring */
#define RF_SS           21
#define RF_RST          22



/* UL tag
 * -------------------------
 * Page 0 b0-2  UID
 *        b3    check byte 0
 * Page 1 b0-3  UID
 * Page 2 b0    check byte 1
 *        b1    internal res
 *        b2    lock byte 0
 *        b3    lock byte 1
 * Page 3 b0-4  OTP
 * Page 4-15    user defined
 * -------------------------
 */
#define BLOCK_COUNT     16
#define BLOCK_SIZE      4
#define SECTOR_START    4

/* Debug */
#define DEBUG(x)        if (DEBUG_MODE) {x}
#define DEBUG_MODE      0
#define MESSAGE_SIZE    32
#define RESET           0

/* Global */
uint8_t   buffer_data_size;
uint8_t*  buffer_data;
char*     buffer_message;
MFRC522   rfid(RF_SS, RF_RST);

/* Const */
String    miro        = "MIRO";
String    user        = "\0";
String    pass        = "\0";
uint8_t   reset_ul[]  = { 0x00, 0x00, 0x00, 0x00 };

/* UI */
uint8_t     led           = LED_BUILTIN;
uint8_t     led_state     = 0;
uint8_t     button        = 0;
uint8_t     button_state  = 0;
uint8_t     int_mode      = FALLING;
hw_timer_t* int_timer     = NULL;

/* Timer */
uint8_t timer_number;
uint16_t timer_divider;
bool timer_count_up;
hw_timer_t* timer;
void (*interrupt_vector)(void);
bool edge;
uint64_t alarm_treshold;
bool autoreload;

/* Funcs */
void IRAM_ATTR ISR(void);
void IRAM_ATTR on_timer();
void print_hex(void);
void print_buffer_to_serial(uint8_t*);

/* Init */
void setup()
{
  buffer_data_size  = 16 + 2;
  buffer_data       = (uint8_t*)malloc(16);
  buffer_message    = (char*)malloc(MESSAGE_SIZE + 1);
  
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  /* UI setup */
  pinMode(led, OUTPUT);
  pinMode(button, INPUT);
  attachInterrupt(button, ISR, int_mode);

  /* Timer setup */
    //timerBegin)
  uint8_t timer_number;
  uint16_t timer_divider;
  bool timer_count_up;
    //timerAttachInterrupt
  hw_timer_t* timer;
  void (*interrupt_vector)(void);
  bool edge;
    //timerAlarmWrite
  timer
  uint64_t alarm_treshold;
  bool autoreload;
    //timerAlarmEnable
  timer
  /* https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html */
    //timerAlarmReadSeconds(timer)
    //timerReadSeconds(timer)
    //timerSetDivider(timer, timer_divider)
    //timerStart(timer)
    //timerStop(timer)
    //timerRestart(timer)
    //timerDetachInterrupt(timer)
  

  uint8_t setup_timer(hw_timer_t* int_timer, void (*f)(void), bool edge)

  int_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(int_timer, &on_timer, true);
  timerAlarmWrite(int_timer, 500000, true);
  timerAlarmEnable(int_timer); //Just Enable

  /* Store miro in buffer */
  uint8_t i;
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    *(buffer_data + i) = miro[i];
  }
  *(buffer_data + BLOCK_SIZE) = 0;

  /* Print buffer to serial */
  Serial.println();
  print_buffer_to_serial(buffer_data)

  DEBUG(Serial.println("Debug mode ON.");)
}

/* Operation */
void loop()
{
  while (!(user[0]))
  {
    Serial.print("User: ");
    while (!(Serial.available())) { }
    user = Serial.readStringUntil('\n');
    Serial.println(user);
  }
  
  if (rfid.PICC_IsNewCardPresent()) // New tag in proximity of the reader
  {
    if (rfid.PICC_ReadCardSerial()) // NUID read
    {
      // Get tag type
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // Send UID to serial
      Serial.print("UID:");
      print_hex(rfid.uid.uidByte, rfid.uid.size);

      // Read pages
      DEBUG(
        Serial.println("Reading memory...");
        uint8_t i;
        for (i = 0; i < BLOCK_COUNT; i++)
        {
          rfid.MIFARE_Read(i, buffer_data, &buffer_data_size);
          Serial.print(i < 10 ? "PAGE #0" : "PAGE #");
          Serial.print(i);
          Serial.print("\t");
          print_hex(buffer_data, BLOCK_SIZE); 
        }
        Serial.println("Done.");
      )

      // Reset user blocks
      if (RESET)
      {
        Serial.print("Resetting user pages");
        uint8_t i;
        for (i = SECTOR_START; i < BLOCK_COUNT; i++)
        {
          if (memcmp(buffer_data, reset_ul, BLOCK_SIZE))
          {
            rfid.MIFARE_Ultralight_Write(i, reset_ul, BLOCK_SIZE); 
          }
          Serial.print('.');
        }
        Serial.println(" Done.");
      }

      // Save user_input to sector_start
      rfid.MIFARE_Read(SECTOR_START, buffer_data, &buffer_data_size);
      
      char* user_buffer = (char*)malloc(BLOCK_SIZE + 1);
      uint8_t j = 0;
      while (user[j])
      {
        *(user_buffer + j) = user[j];
        j++;
      }
      
      if (memcmp(buffer_data, user_buffer, BLOCK_SIZE))
      {
        // Read user
        uint8_t i;
        for (i = 0; i < BLOCK_SIZE; i++)
        {
          *(buffer_data + i) = user[i];
        }
        *(buffer_data + BLOCK_SIZE) = 0; 

        // Write to sector start
        rfid.MIFARE_Ultralight_Write(SECTOR_START, buffer_data, BLOCK_SIZE);
        Serial.println("User saved.");
      }
      
      free(user_buffer);
      user.clear();

      // Dump tag memory to serial (general)
      // rfid.PICC_DumpToSerial(&rfid.uid); // Does this halt at the end?

      // Dump tag memory to serial (UL)
      rfid.PICC_DumpMifareUltralightToSerial();

      // Halt
      rfid.PICC_HaltA();      // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}

/* Interrupt vector */
void IRAM_ATTR ISR()
{
  digitalWrite(led, led_state = button_state = !button_state);
  if (button_state)
  {
    if (!timerStarted(int_timer)) { timerStart(int_timer); }
    timerRestart(int_timer);
  }
  else
  {
    timerStop(int_timer);
  }
  DEBUG(Serial.println("Button on GPIO0 pressed.");)
}

void IRAM_ATTR on_timer()
{
  if (button_state) { digitalWrite(led, led_state = !led_state); }
}

/* Function definitions */
void print_hex(byte* bytes, uint8_t len)
{
  uint8_t i;
  for (int i = 0; i < len; i++)
  {
    Serial.print(*(bytes + i) < 0x10 ? " 0" : " ");
    Serial.print(*(bytes + i), HEX);
  }
  Serial.println();
}

void print_buffer_to_serial(uint8_t* buffer)
{
  i = 0;
  while(*(buffer + i))
  {
    Serial.print((char)*(buffer + i++));
  }
  Serial.println();
}