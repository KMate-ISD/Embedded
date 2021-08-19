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
#include <stdlib.h>


/* ========================================
 * | DECLARATION
 * ======================================== */

/* ----------------------------------------
 * | Constants
 * ---------------------------------------- */

/* Port mapping */
const uint8_t RCLK = 4;
const uint8_t SRCLK = 5;
const uint8_t SER_A = 2;
const uint8_t SER_B = 3;

/* complements used for clearing a bit */
const uint8_t RCLK_R = 0xEF;
const uint8_t SRCLK_R = 0xDF;
const uint8_t SER_A_R = 0xFB;
const uint8_t SER_B_R = 0xF7;

/* ----------------------------------------
 * | Data fields
 * ---------------------------------------- */

/* System */
uint8_t counter_byte = 0;		// TIMER2 OVF scaling
uint8_t active_row = 0;			// *** 3bit | Marks the active row on the LED matrix
uint8_t seed;					// Random seed

/* Game model */
int8_t score;					// *** ?bit
uint8_t direction;				// *** 2bit | UP, LEFT, DOWN, RIGHT: 0, 1, 2, 3
uint8_t fruit_position;			// *** 5bit
uint8_t game_field[8];			// 8byte || unnecessary? see snake_body
uint8_t snake_body[64];			// *** 64*5bit || 64bit + 64*2bit + 5bit (is_set + direction + head_position)?
uint8_t snake_length;			// *** 5bit
uint8_t is_concluded;			// *** 1bit
uint16_t total_ticks;

/* is_concluded + direction + fruit_position? -> 8bit -> 1byte
 * snake_body -> 192bit -> 24byte
 * snake_head + snake_length -> 10bit -> 2byte
 * 76byte vs 27byte */

/* ----------------------------------------
 * | Functions and methods
 * ---------------------------------------- */

/* Initialization */
void initialize_ports(void);
void initialize_timer2_overflow(void);
void initialize_game_model(void);

/* Timed behaviour */
void push_to_matrix(uint8_t, uint8_t);
void read_joystick_input(void);
void advance_game_state(void);

/* Game mechanics */
void move_snake(void);
void project_game_status_onto_game_field(void);
void project_game_status_changes_onto_game_field(uint8_t, uint8_t, uint8_t);
uint8_t check_collision(uint8_t, uint8_t);
uint8_t check_victory_condition(void);
uint8_t spawn_fruit(void);


/* ========================================
 * | PROCESS
 * ======================================== */

int main(void)
{
    initialize_ports();
    initialize_timer2_overflow();
    initialize_game_model();
    
    while (!(is_concluded))
    {
    }
    
    return total_ticks;
}


/* ========================================
 * | DEFINITION
 * ======================================== */

/* Initialization */
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

void initialize_game_model()
{
    seed = 131;
    srand(seed);
    snake_body[0] = 50;
    snake_body[1] = 60;
    snake_body[2] = 70;
    snake_length = 3;
    direction = 0;
    game_field[5] = 0x80;
    game_field[6] = 0x80;
    game_field[7] = 0x80;
    fruit_position = spawn_fruit();
    score = 0;
    total_ticks = 0;
    project_game_status_onto_game_field();
}

/* Timed behaviour */
void push_to_matrix(uint8_t active_row, uint8_t row_of_bits)
{ 
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        /* Send bit to serial pins */
        uint8_t row = (active_row >> i) & 0x01;
        uint8_t column = (~row_of_bits >> i) & 0x01;
        PORTD &= SER_A_R & SER_B_R;
        PORTD |= (row << SER_B) | (column << SER_A);
        
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

void advance_game_state()
{
    is_concluded = check_victory_condition();
    
    if (!(is_concluded))
    {
        move_snake();
    }
}

/* Game mechanics */
void move_snake()
{
    uint8_t is_grown = 0;
    uint8_t old_tail = 0;
    uint8_t snake_head_proposed_y = snake_body[0] / 10;
    uint8_t snake_head_proposed_x = snake_body[0] % 10;
    
    switch (direction)
    {
        case 1:
        snake_head_proposed_x -= 1;
        break;
        case 2:
        snake_head_proposed_y += 1;
        break;
        case 3:
        snake_head_proposed_x += 1;
        break;
        default:
        snake_head_proposed_y -= 1;
        break;
    }
    
    is_concluded = check_collision(snake_head_proposed_y, snake_head_proposed_x);
    
    if (!(is_concluded))
    {
        uint8_t i = snake_length - 1;
        uint8_t snake_head_proposed = snake_head_proposed_y*10 + snake_head_proposed_x;
        old_tail = snake_body[i];
        
        if (fruit_position == snake_head_proposed)
        {
            snake_body[snake_length++] = snake_body[i];
            is_grown = 1;
            score++;
            fruit_position = spawn_fruit();
        }
        
        for (; i > 0; i--)
        {
            snake_body[i] = snake_body[i - 1];
        }
        
        snake_body[i] = snake_head_proposed;
    }
    
    project_game_status_changes_onto_game_field(snake_body[0], old_tail, is_grown);
}

void project_game_status_onto_game_field()
{
    uint8_t i;
    uint8_t fruit_y = fruit_position / 10;
    uint8_t fruit_x = fruit_position % 10;
    uint8_t snake_cell_y;
    uint8_t snake_cell_x;
    
    for (i = 0; i < 8; i++)
    {
        game_field[i] = 0;
    }
    
    game_field[fruit_y] |= (1 << (7 - fruit_x));
        
    for (i = 0; i < snake_length; i++)
    {
        snake_cell_y = snake_body[i] / 10;
        snake_cell_x = snake_body[i] % 10;
        game_field[snake_cell_y] |= (1 << (7 - snake_cell_x));
    }
}

void project_game_status_changes_onto_game_field(uint8_t snake_head, uint8_t old_tail, uint8_t is_grown)
{
    uint8_t fruit_y = fruit_position / 10;
    uint8_t fruit_x = fruit_position % 10;
    uint8_t snake_head_y = snake_head / 10;
    uint8_t snake_head_x = snake_head % 10;
    uint8_t old_tail_y = old_tail / 10;
    uint8_t old_tail_x = old_tail % 10;
    
    game_field[fruit_y] |= (1 << (7 - fruit_x));
    game_field[snake_head_y] |= (1 << (7 - snake_head_x));
    
    if (!(is_grown))
    {
        game_field[old_tail_y] &= ~(1 << (7 - old_tail_x));
    }
}

uint8_t check_collision(uint8_t snake_head_proposed_y, uint8_t snake_head_proposed_x)
{
    if (((snake_head_proposed_y*10 / 7) > 10) ||
        ((snake_head_proposed_x*10 / 7) > 10) ||
        (game_field[snake_head_proposed_y] & (1 << (7 - snake_head_proposed_x))))
    {
        return 1;
    } 
    else
    {
        return 0;
    }
}

uint8_t check_victory_condition()
{
    if (snake_length > 63)
    {
        return 1;
    } 
    else
    {
        return 0;
    }
}

uint8_t spawn_fruit()
{
    uint8_t fruit_proposed_y;
    uint8_t fruit_proposed_x;
    
    do
    {
        fruit_proposed_y = rand() % 8;
        fruit_proposed_x = rand() % 8;
    } while (game_field[fruit_proposed_y] & (1 << (7 - fruit_proposed_x)));
    
    return fruit_proposed_y*10 + fruit_proposed_x;
}

/* Interrupt */
ISR(TIMER2_OVF_vect)							// Roughly 448* per second
{
    if (!(counter_byte % 25))					// Roughly 20* per second
    {
        read_joystick_input();
    }
    
    if (!(--counter_byte || is_concluded))		// Roughly 2* per second
    {
        advance_game_state();
        total_ticks++;
    }
    
    active_row = counter_byte % 8;				// Roughly 61* full cycles per second
    push_to_matrix((1 << active_row), game_field[active_row]);
}