#include <MFRC522.h>    // RFID-RC522 module
#include <SPI.h>        // SPI bus

/* Wiring */
#define SS_PIN          21
#define RST_PIN         22
#define BLOCK_SIZE      4
#define MESSAGE_SIZE    32

/* Global */
uint8_t   buffer_data_size;
uint8_t*  buffer_data;
char*     buffer_message;
MFRC522   rfid(SS_PIN, RST_PIN);

void print_hex(void);

void setup()
{
  buffer_data_size  = 16 + 2;
  buffer_data       = (uint8_t*)malloc(16);
  buffer_message    = (char*)malloc(MESSAGE_SIZE + 1);
  
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
}

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
