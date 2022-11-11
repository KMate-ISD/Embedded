#include "mcu_tag_decoder_ul.h"





void Tag_encoder_decoder::read_memory()
{
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
}

void Tag_encoder_decoder::reset_user_blocks()
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

void Tag_encoder_decoder::save_user_input()
{
  while (!(user[0]))
  {
    user = Serial.readStringUntil('\n');
    Serial.println(user);
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
}

void Tag_encoder_decoder::end_op()
{
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void Tag_encoder_decoder::print_hex(byte* bytes, uint8_t len)
{
  uint8_t i;
  for (int i = 0; i < len; i++)
  {
    Serial.print(*(bytes + i) < 0x10 ? " 0" : " ");
    Serial.print(*(bytes + i), HEX);
  }
  Serial.println();
}

void Tag_encoder_decoder::print_to_serial(char* buffer)
{
  i = 0;
  while(*(buffer + i))
  {
    Serial.print((char)*(buffer + i++));
  }
  Serial.println();
}