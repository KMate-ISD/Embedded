#include <MFRC522.h>    // RFID-RC522 module
#include <SPI.h>        // SPI bus

/* Wiring */
#define SS_PIN          21
#define RST_PIN         22

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
MFRC522   rfid(SS_PIN, RST_PIN);

/* Const */
String    miro        = "MIRO";
uint8_t   reset_ul[]  = { 0x00, 0x00, 0x00, 0x00 };

/* UI */
uint8_t     led           = LED_BUILTIN;
uint8_t     led_state     = 0;
uint8_t     button        = 0;
uint8_t     button_state  = 0;
uint8_t     int_mode      = FALLING;
hw_timer_t* int_timer     = NULL;

/* Funcs */
void IRAM_ATTR ISR(void);
void IRAM_ATTR on_timer();
void print_hex(void);

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
  i = 0;
  Serial.println();
  while(*(buffer_data + i))
  {
    Serial.print((char)*(buffer_data + i++));
  }
  Serial.println();

  DEBUG(Serial.println("Debug mode ON.");)
}

/* Operation */
void loop()
{
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

      // Dump tag memory to serial (general)
      // rfid.PICC_DumpToSerial(&rfid.uid);
      // rfid.PICC_DumpToSerial(&rfid.uid); // Does this halt at the end?

      // Dump tag memory to serial (UL)
      rfid.PICC_DumpMifareUltralightToSerial();

      // Write block
      // rfid.MIFARE_Ultralight_Write(7, buffer_data, sizeof(buffer_data));

      // Read block
      uint8_t sector = 7;
      rfid.MIFARE_Read(sector, buffer_data, &buffer_data_size);
      Serial.print("Sector ");
      Serial.print(sector);
      Serial.print(":");
      print_hex(buffer_data, BLOCK_SIZE);

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
