#include <avr/io.h> 

#include <avr/interrupt.h> 

#include <util/delay.h> 

#include <math.h> 
 

#define F_CPU 8000000UL 

 

static const uint8_t sine_wave[10] = { 

    128, 192, 240, 255, 240, 128, 64, 16, 0, 16 

}; // DID IT USING 20 SAMPLES BETTER RESOULTION AND BETTER PHASE  

 
 

static const float freq_table[] = { 

    1018.73, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.72, 1018.34, 1016.27, 

    1012.25, 1008.1, 1006.51, 1006.27, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.27, 1006.64, 1008.66, 1014.57, 

    1021.26, 1024.23, 1024.94, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1024.99, 1024.71, 1022.53, 1016.32, 1006.87, 1001.65, 

    1000.15, 1000.01, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.01, 1000.23, 1002.49, 1010.3, 1024.4, 1033.7, 1037.04, 1037.48, 1037.5 

}; 

 
 

#define FREQ_TABLE_SIZE (sizeof(freq_table) / sizeof(freq_table[0])) 

 
 

#define CS_PIN PD3 

#define CS_PORT PORTD 

#define CS_DDR DDRD 

 
 

static void SPI0_init(void) { 

    DDRB |= (1 << PB5) | (1 << PB7); 

    CS_DDR  |= (1 << CS_PIN); 

    CS_PORT |= (1 << CS_PIN); 

    SPCR0 = (1 << SPE) | (1 << MSTR); 

    SPSR0 |= (1 << SPI2X); 

} 

 
 

static void spi_write16(uint16_t data) { 

    SPDR0 = (data >> 8); 

    while (!(SPSR0 & (1 << SPIF))); 

    SPDR0 = (data & 0xFF); 

    while (!(SPSR0 & (1 << SPIF))); 

} 

 
 

static void write_to_dac(uint8_t channelAB, uint8_t value) { 

    uint16_t cmd = 0; 

    cmd |= ((uint16_t)channelAB << 15); 

    cmd |= (1 << 13) | (1 << 12); 

    cmd |= ((uint16_t)value << 4); 

    CS_PORT &= ~(1 << CS_PIN); 

    spi_write16(cmd); 

    CS_PORT |= (1 << CS_PIN); 

} 

 
 

volatile uint8_t sineIndex = 0; 

volatile uint16_t timer3_compare_value = 100; 

 
 

static void timer3_init(void) { 

    TCCR3A = 0; 

    TCCR3B = (1 << WGM32) | (1 << CS31); 

    OCR3A  = timer3_compare_value; 

    TIMSK3 |= (1 << OCIE3A); 

    TCNT3  = 0; 

} 

 
 

ISR(TIMER3_COMPA_vect) { 

    if (sineIndex >= 10) sineIndex = 0; 

    uint8_t quadratureIndex = sineIndex + 2; 

    if (quadratureIndex >= 10) quadratureIndex -= 10; 

    uint8_t I_val = sine_wave[sineIndex]; 

    uint8_t Q_val = sine_wave[quadratureIndex]; 

    write_to_dac(0, I_val); 

    write_to_dac(1, Q_val); 

    sineIndex++; 

} 

 

volatile uint16_t freq_index = 0; 

 

static void timer4_init(void) { 

    uint16_t TIMER4_INTERVAL_US = 10667; 

    TCCR4A = 0; 

    TCCR4B = (1 << WGM42) | (1 << CS41); 

    OCR4A  = TIMER4_INTERVAL_US; 

    TIMSK4 |= (1 << OCIE4A); 

    TCNT4  = 0; 

} 

 
 

static uint16_t calculate_timer3_ticks(float freq_hz) { 

    if (freq_hz < 1.0) { 

        freq_hz = 1.0; 

    } 

    float period_us = 100000.0f / freq_hz; 

    uint16_t ocr_val = (uint16_t)(period_us + 0.5f); 

    if (ocr_val > 1) ocr_val -= 1; 

    return ocr_val; 

} 

 
 

ISR(TIMER4_COMPA_vect) { 

    float new_freq = freq_table[freq_index]; 

    uint16_t new_ocr3 = calculate_timer3_ticks(new_freq); 

    OCR3A = new_ocr3; 

    freq_index++; 

    if (freq_index >= FREQ_TABLE_SIZE) { 

        freq_index = 0; 

    } 

} 

 
 

int main(void) { 

    CLKPR = (1 << CLKPCE); 

    CLKPR = 0; 

    SPI0_init(); 

    timer3_init(); 

    timer4_init(); 

    sei(); 

    while (1) {} 

} 

 
