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


void print_hex(void);
void print_buffer_to_serial(uint8_t*);


/* Init */
void setup()
{
  buffer_data_size  = 16 + 2;
  buffer_data       = (uint8_t*)malloc(16);
  buffer_message    = (char*)malloc(MESSAGE_SIZE + 1);
  

  SPI.begin();
  rfid.PCD_Init();

  
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
      // rfid.PICC_DumpToSerial(&rfid.uid); // Does this halt at the end?

      // Dump tag memory to serial (UL)
      rfid.PICC_DumpMifareUltralightToSerial();

    }
  }
}