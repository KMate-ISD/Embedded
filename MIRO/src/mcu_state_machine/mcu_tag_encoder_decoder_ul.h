#ifndef TAG_DECODER
#define TAG_DECODER

#include <MFRC522.h>

/* Wiring */
#define RF_SS                 21
#define RF_RST                22

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
#define BLOCK_COUNT           16
#define BLOCK_SIZE            4
#define READ_BUFFER_SIZE      16 + 2
#define SECTOR_START          4

/* Debug */
#define MESSAGE_SIZE          32
#define RESET                 0


class Tag_encoder_decoder
{
  private:
    uint8_t* reset_ul;

  public:
    MFRC522 rfid;

    Tag_encoder_decoder(MFRC522&);
    ~Tag_encoder_decoder();

    void dump_memory();
    void end_op();
    void precision_write(uint8_t*, uint8_t, uint8_t, uint8_t);
    void print_hex(uint8_t*, uint8_t);
    void print_to_serial(char*);
    void read_blocks(uint8_t*, uint8_t, uint8_t);
    void reset_blocks(uint8_t, uint8_t);
    void write_blocks(uint8_t*, uint8_t, uint8_t);
    String get_tag_type();
};

#endif // TAG_DECODER