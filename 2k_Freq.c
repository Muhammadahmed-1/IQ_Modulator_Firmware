#include <avr/interrupt.h> 

#include <avr/io.h> 

#include <util/delay.h> 

 
 

#define F_CPU 8000000UL 

#define CS_PIN PD3 

#define CS_PORT PORTD 

#define CS_DDR DDRD 

 
 

static const uint8_t sine_wave[20] = { 

    128, 167, 202, 231, 249, 255, 249, 231, 202, 167, 

    128,  88,  53,  24,   6,   0,   6,  24,  53,  88 

}; 

 
 

volatile uint16_t last_rising_edge_time = 0; 

volatile uint16_t input_wave_period = 800; 

volatile uint8_t sample_index = 0; 

 
 

static uint8_t SPI0_transmit(uint8_t data) { 

    SPDR0 = data; 

    while (!(SPSR0 & (1 << SPIF))) { 

    } 

    return SPDR0; 

} 

 
 

static void SPI0_init(void) { 

    DDRB  |= (1 << PB5) | (1 << PB7); 

    CS_DDR |= (1 << CS_PIN); 

    CS_PORT |= (1 << CS_PIN); 

    SPCR0 = (1 << SPE) | (1 << MSTR); 

    SPSR0 |= (1 << SPI2X); 

} 

 
 

static void timer1_input_capture_init(void) { 

    DDRD &= ~(1 << PD6); 

    TCCR1A = 0; 

    TCCR1B = (1 << ICES1) | (1 << CS11); 

    TIMSK1 |= (1 << ICIE1); 

    TCNT1 = 0; 

    last_rising_edge_time = 0; 

} 

 
 

static void timer3_output_compare_init(void) { 

    TCCR3A = 0; 

    TCCR3B = (1 << WGM32) | (1 << CS31); 

    OCR3A = input_wave_period / 20; 

    TIMSK3 |= (1 << OCIE3A); 

    TCNT3 = 0; 

} 

 
 

ISR(TIMER1_CAPT_vect) { 

    uint16_t current = ICR1; 

    uint16_t period = (current - last_rising_edge_time) & 0xFFFF; 

    last_rising_edge_time = current; 

    input_wave_period = period; 

 
 

    uint16_t new_ocr = input_wave_period / 20; 

    if (new_ocr < 1) new_ocr = 1; 

    OCR3A = new_ocr; 

} 

 
 

ISR(TIMER3_COMPA_vect) { 

    if (sample_index >= 20) sample_index = 0; 

 
 

    uint8_t q_sample_index = sample_index + 5; 

    if (q_sample_index >= 20) q_sample_index -= 20; 

 
 

    uint8_t I_val = sine_wave[sample_index]; 

    uint8_t Q_val = sine_wave[q_sample_index]; 

 
 

    uint16_t cmd_I = 0; 

    cmd_I |= (1 << 13) | (1 << 12); 

    cmd_I |= ((uint16_t)I_val << 4); 

    CS_PORT &= ~(1 << CS_PIN); 

    SPDR0 = (cmd_I >> 8); while (!(SPSR0 & (1 << SPIF))); 

    SPDR0 = (cmd_I & 0xFF); while (!(SPSR0 & (1 << SPIF))); 

    CS_PORT |= (1 << CS_PIN); 

 
 

    uint16_t cmd_Q = 0; 

    cmd_Q |= (1 << 15); 

    cmd_Q |= (1 << 13) | (1 << 12); 

    cmd_Q |= ((uint16_t)Q_val << 4); 

    CS_PORT &= ~(1 << CS_PIN); 

    SPDR0 = (cmd_Q >> 8); while (!(SPSR0 & (1 << SPIF))); 

    SPDR0 = (cmd_Q & 0xFF); while (!(SPSR0 & (1 << SPIF))); 

    CS_PORT |= (1 << CS_PIN); 

 
 

    sample_index++; 

} 

 
 

int main(void) { 

    CLKPR = (1 << CLKPCE); 

    CLKPR = 0; 

    SPI0_init(); 

    timer1_input_capture_init(); 

    timer3_output_compare_init(); 

    sei(); 

 
 

    while (1) { 

    } 

} 

 
 

 
