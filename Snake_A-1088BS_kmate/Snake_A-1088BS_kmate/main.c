/*
 * Snake_A-1088BS_kmate.c
 *
 * Created: 2021. 07. 29. 23:46:38
 * Author : m8_krk
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char counter_byte = 0;

void initialize_ports(void);
void initialize_timer2_overflow(void);

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

ISR(TIMER2_OVF_vect)
{
}

