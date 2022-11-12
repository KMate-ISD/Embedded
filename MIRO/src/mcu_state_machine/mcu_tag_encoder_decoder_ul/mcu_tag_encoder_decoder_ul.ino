#include <SPI.h>
#include <string>
#include "mcu_tag_encoder_decoder_ul.h"
#include "mcu_state_machine.h"


/* Globals */
MFRC522 rfid(RF_SS, RF_RST);
Tag_encoder_decoder ecdc(rfid);

/* Init */
void setup()
{
  Serial.begin(BAUD_RATE);
  SPI.begin();
  ecdc.rfid.PCD_Init();

  // String user = "\0";
  // Serial.print("\nUser: ");
  // while (user.isEmpty())
  // {
  //   user = Serial.readStringUntil('\n');
  // }
  // Serial.println(user);
}

/* Operation */
void loop()
{
  if (rfid.PICC_IsNewCardPresent()) // New tag in proximity of the reader
  {
    if (rfid.PICC_ReadCardSerial()) // NUID read
    {
      // Read lines
      uint8_t start_block = SECTOR_START;
      uint8_t stop_block = BLOCK_COUNT;
      uint8_t len_data = (stop_block - start_block)*BLOCK_SIZE;
      uint8_t* data = (uint8_t*)malloc(len_data*sizeof(uint8_t));
      ecdc.read_blocks(data, start_block, stop_block);
      Serial.println();
      ecdc.print_hex(data, len_data);

      // Safe close
      ecdc.end_op();
      free(data);
    }
  }
}