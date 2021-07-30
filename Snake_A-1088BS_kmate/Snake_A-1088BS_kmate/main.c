/*
 * Snake_A-1088BS_kmate.c
 *
 * Created: 2021. 07. 29. 23:46:38
 * Author : m8_krk
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char counter_byte = 0;
unsigned char row_marker = 7;

void initialize_ports(void);
void initialize_timer2_overflow(void);
void advance_game_state(void);
void push_to_matrix(void);
void read_joystick_input(void);

int main(void)
{
    /* Replace with your application code */
    while (1)
    {
    }
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

void push_to_matrix()
{
}

void read_joystick_input()
{	
}

ISR(TIMER2_OVF_vect)
{
	if (counter_byte % 25 == 0) /* Roughly 20 times per second */
	{
		read_joystick_input();
	}
	
	if !(counter_byte--) /* Roughly twice per second */
	{
		advance_game_state();
	}
	
	push_to_matrix(); /* Roughly 61 full cycles per second */
}