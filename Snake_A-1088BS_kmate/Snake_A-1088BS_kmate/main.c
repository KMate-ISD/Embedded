/*
 * Snake_A-1088BS_kmate.c
 *
 * Created: 2021. 07. 29. 23:46:38
 * Author : m8_krk
 */

/*
 * 74HC595 8bit shift register
 * Shift register clock: SRCLK (TI), SH_CP (Philips)
 * Storage register clock: RCLK (TI), ST_CP (Philips)
 * Serial data input: SER (TI), DS (Philips)
 */

#include <avr/io.h>
#include <avr/interrupt.h>

const uint8_t RCLK = 4;
const uint8_t SRCLK = 5;
const uint8_t SER_A = 2;
const uint8_t SER_B = 3;

/* complements used for clearing a bit */
const uint8_t RCLK_R = 0xEF;
const uint8_t SRCLK_R = 0xDF;
const uint8_t SER_A_R = 0xFB;
const uint8_t SER_B_R = 0xF7;

unsigned char counter_byte = 0;
unsigned char row_of_bits = 0;
/* Game state */
uint8_t direction = 0x00; // UP, DOWN, LEFT, RIGHT: 0, 1, 2, 3
uint8_t game_field[8] = {0, 0, 0, 0, 0, 0x80, 0x80, 0x80};
uint8_t snake_body[64] = {50, 60, 70};
uint8_t snake_length = 3;

void initialize_ports(void);
void initialize_timer2_overflow(void);
void advance_game_state(void);
void push_to_matrix(uint8_t, uint8_t);
void read_joystick_input(void);

int main(void)
{
	initialize_ports();
	initialize_timer2_overflow();
	while (1)
	{
	}
}

void initialize_ports()
{
	DDRD |= (1 << RCLK) | (1 << SRCLK) | (1 << SER_A) | (1 << SER_B);
}

void initialize_timer2_overflow()
{
	TCCR2A = 0;
	TCCR2B = (1 << CS22) | (0 << CS21) | (1 << CS20);
	TIMSK2 = (1 << TOIE2);
	sei();
}

void advance_game_state()
{
}

void push_to_matrix(uint8_t column_of_bits, uint8_t row_of_bits)
{	
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		/* Send bit to serial pins */
		uint8_t column = (column_of_bits >> i) & 0x01;
		uint8_t row = (row_of_bits >> i) & 0x01;
		PORTD &= SER_A_R & SER_B_R;
		PORTD |= (column << SER_A) | (row << SER_B);
		
		/* SH_CP LOW-TO-HIGH (Push to shift register) */
		PORTD &= SRCLK_R;
		PORTD |= (1 << SRCLK);
	}

	/* ST_CP LOW-TO-HIGH (Push to storage register) */
	PORTD &= RCLK_R;
	PORTD |= (1 << RCLK);
}

void read_joystick_input()
{
}

ISR(TIMER2_OVF_vect) /* Roughly 448 calls per second */
{
	if (!(counter_byte % 25)) /* Roughly 20 times per second */
	{
		read_joystick_input();
	}
	
	if (!(--counter_byte)) /* Roughly twice per second */
	{
		advance_game_state();
	}
	
	row_of_bits = 1 << (counter_byte % 8);
	push_to_matrix(0x01, row_of_bits); /* Roughly 61 full cycles per second */
}