#include "mcu_tag_encoder_decoder_ul.h"


uint8_t* reset_ul = new uint8_t[4]();

Tag_encoder_decoder::Tag_encoder_decoder(MFRC522& rfid)
  : rfid { rfid }
{

}

Tag_encoder_decoder::~Tag_encoder_decoder() { }

void Tag_encoder_decoder::dump_memory()
{
  this->rfid.PICC_DumpMifareUltralightToSerial();
}

void Tag_encoder_decoder::end_op()
{
  this->rfid.PICC_HaltA();
  this->rfid.PCD_StopCrypto1();
}

void Tag_encoder_decoder::precision_write(uint8_t* data, uint8_t len, uint8_t start_block=SECTOR_START, uint8_t start_byte=0)
{
  uint8_t i;
  for (i = start_block; i < len; i++)
  {
    uint8_t block = i/BLOCK_SIZE;
    uint8_t byte = (i%BLOCK_SIZE + start_block)%BLOCK_SIZE;
    Serial.println("Data would be placed like so:");
    Serial.print("data[");
    Serial.print(block);
    Serial.print(", ");
    Serial.print(byte);
    Serial.print("] ");
    Serial.println(*(data + i));
  }
  Serial.println("Precision not yet implemented. Use write_blocks() instead.");
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
  uint8_t i = 0;
  while(*(buffer + i))
  {
    Serial.print((char)*(buffer + i++));
  }
  Serial.println();
}

void Tag_encoder_decoder::read_blocks(uint8_t* dest, uint8_t start_block, uint8_t stop_block)
{
  uint8_t read_buffer_size = READ_BUFFER_SIZE;
  uint8_t* read_buffer_data = (uint8_t*)malloc(READ_BUFFER_SIZE*sizeof(uint8_t));

  Serial.print("Reading ");
  Serial.println(this->get_tag_type());
  Serial.println();

  uint8_t i;
  for (i = start_block; i < stop_block; i++)
  {
    char* buffer_str = new char[16];
    snprintf(buffer_str, 16, "BLOCK %2d  | ", i);
    Serial.print(buffer_str);

    rfid.MIFARE_Read(i, read_buffer_data, &read_buffer_size);
    memcpy(dest + BLOCK_SIZE*(i-start_block), read_buffer_data, BLOCK_SIZE);

    print_hex(read_buffer_data, BLOCK_SIZE);
    delete[] buffer_str;
  }
  Serial.println();

  free(read_buffer_data);

  Serial.print((stop_block - start_block)*BLOCK_SIZE);
  Serial.println(" bytes read.");
}

void Tag_encoder_decoder::reset_blocks(uint8_t start_block, uint8_t count)
{
  Serial.println("Blocks 0-3 contain manufacturer data, OTP and lock bytes. Start block should be 4 or higher for storing user data. Terminating process.");
  if (start_block < 4) { return; }

  uint8_t read_buffer_size = READ_BUFFER_SIZE;
  uint8_t* read_buffer_data = (uint8_t*)malloc(READ_BUFFER_SIZE*sizeof(uint8_t));

  Serial.print("Resetting user blocks");

  uint8_t i;
  for (i = start_block; i < start_block + count; i++)
  {
    rfid.MIFARE_Read(i, read_buffer_data, &read_buffer_size);
    if (memcmp(read_buffer_data, reset_ul, BLOCK_SIZE))
    {
      rfid.MIFARE_Ultralight_Write(i, reset_ul, BLOCK_SIZE);
    }
    Serial.print('.');
  }
  Serial.println(" Done.");

  free(read_buffer_data);
}

void Tag_encoder_decoder::write_blocks(uint8_t* source, uint8_t len, uint8_t start_block)
{
  Serial.println("Blocks 0-3 contain manufacturer data, OTP and lock bytes. Start block should be 4 or higher for storing user data. Terminating process.");
  if (start_block < 4) { return; }

  Serial.print("Saving data to tag...");
  rfid.MIFARE_Ultralight_Write(start_block, source, len);
  Serial.println(" Done.");
}

String Tag_encoder_decoder::get_tag_type()
{
  MFRC522::PICC_Type piccType = this->rfid.PICC_GetType(this->rfid.uid.sak);
  return(this->rfid.PICC_GetTypeName(piccType));
}