#ifndef TAG_DECODER
#define TAG_DECODER

class Tag_encoder_decoder
{
  public:
    void read_memory();
    void reset_user_blocks();
    void save_user_input();
    void end_op();
    void print_hex(uint8_t*, uint8_t len);
    void print_to_serial(char* buffer);
};

#endif // TAG_DECODER