#define F_CPU 8000000UL  

#include <avr/io.h>  

#include <avr/interrupt.h>  

#include <util/delay.h>  

#include <math.h>  

 
 

//pin intialization for switch and the led pin and chip select and the port each pin is at to be accessed. 

#define MODE_SWITCH_PIN PD1  

#define MODE_SWITCH_PORT PIND  

#define MODE_SWITCH_DDR  DDRD  

#define MODE_SWITCH_PUE  PORTD  

 
 

#define LED_PIN    PB3  

#define CS_PIN     PD3  

#define CS_PORT    PORTD  

#define CS_DDR     DDRD  

 
 

//Switch mode    

#define MODE_EXTERNAL 0  

#define MODE_INTERNAL 1  

 
 

volatile uint8_t current_state = MODE_EXTERNAL;  

 
 

static const uint8_t sine_wave[10] = {  

    128, 192, 240, 255, 240, 128, 64, 16, 0, 16  

};  

 
 
 

volatile uint16_t prev_rising_edge_time = 0;  

volatile uint16_t period_rising_edges   = 800;  

 
 

static const float freq_table[] = {  

1018.73, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.72, 1018.34, 1016.27,  

1012.25, 1008.1, 1006.51, 1006.27, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.27, 1006.64, 1008.66, 1014.57,  

1021.26, 1024.23, 1024.94, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1024.99, 1024.71, 1022.53, 1016.32, 1006.87, 1001.65,  

1000.15, 1000.01, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.01, 1000.23, 1002.49, 1010.3, 1024.4, 1033.7, 1037.04, 1037.48, 1037.5,  

1037.5, 1037.5, 1037.5, 1037.5, 1037.5, 1037.5, 1037.5, 1037.49, 1037.28, 1036.24, 1034.04, 1032.07, 1031.36, 1031.26, 1031.25, 1031.25, 1031.25, 1031.25,  

1031.25, 1031.25, 1031.25, 1031.25, 1031.22, 1030.81, 1028.72, 1022.81, 1016.16, 1013.1, 1012.56, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5,   

1012.5, 1012.49, 1012.33, 1011.31, 1007.77, 1003.45, 1000.69, 1000.08, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0,  

1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0,  

1000.0, 1000.01, 1000.1, 1000.8, 1002.56, 1004.87, 1005.96, 1006.23, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25,  

1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.34, 1007.13, 1010.9, 1017.88,  

1022.85, 1024.73, 1024.98, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1024.94, 1024.3, 1020.6, 1011.94, 1004.21, 1000.62, 1000.05, 1000.0,  

1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0,  

1000.0, 1000.0, 1000.02, 1000.38, 1003.49, 1012.67, 1026.77, 1034.75, 1037.21, 1037.49, 1037.5, 1037.5, 1037.5,1037.5, 1037.5, 1037.5, 1037.5, 1037.5, 1037.35, 1035.88,  

1029.47, 1017.87, 1009.19, 1006.66, 1006.27, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.23, 1006.05, 1005.03, 1003.0, 1000.92, 1000.16, 1000.01, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.05, 1000.63, 1004.59, 1013.8, 1024.96, 1029.94, 1031.15, 1031.25, 1031.26, 1031.26, 1031.26, 1031.24, 1031.24, 1031.24, 1031.24,  

1031.25, 1031.39, 1032.32, 1035.56, 1040.18, 1042.97, 1043.67, 1043.75, 1043.75, 1043.75, 1043.75, 1043.75, 1043.75, 1043.75, 1043.75, 1043.74, 1043.47, 1040.99, 1032.44, 1016.18, 1004.06, 1000.56, 1000.03, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0,  

1000.0, 1000.06, 1000.94, 1005.51, 1017.86, 1029.9, 1036.17, 1037.4, 1037.5, 1037.5,  

1037.5, 1037.5, 1037.5, 1037.5, 1037.5, 1037.5, 1037.48, 1037.04, 1033.69, 1023.12, 1010.28, 1002.07,  

1000.23, 1000.01, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.01, 1000.15, 1001.24, 1005.76, 1012.28, 1017.2, 1018.54, 1018.74, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75,  

1018.75, 1018.75, 1018.69, 1017.99, 1014.54, 1008.3, 1002.38, 1000.38, 1000.02, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.01, 1000.2, 1001.37, 1005.11, 1009.37, 1011.91, 1012.44, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5, 1012.5,  

1012.5, 1012.55, 1012.93, 1014.44, 1016.8, 1018.23, 1018.69, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.75, 1018.7, 1018.14, 1015.95, 1011.36, 1007.63, 1006.46, 1006.26, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.28, 1006.57, 1008.39, 1012.26, 1016.58, 1018.41,  

1010.09, 1007.09, 1006.35, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.25, 1006.34, 1007.13, 1010.9, 1017.88, 1022.85, 1024.73, 1024.98, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1025.0, 1024.94, 1024.3, 1020.6, 1011.94, 1004.21, 1000.63, 1000.04, 1000.0, 1000.0, 1000.0,  

};  

 
 

#define FREQ_TABLE_SIZE (sizeof(freq_table) / sizeof(freq_table[0]))  

volatile uint16_t freq_index = 0;  

 
 
 

static uint16_t compute_OCR3A_for_10sample(float freq_hz) {  

    if (freq_hz < 1.0) {  

        freq_hz = 1.0;  

    }  

    float period_us = 100000.0f / freq_hz;  

    uint16_t ticks  = (uint16_t)(period_us + 0.5f);  

    if (ticks > 0) ticks -= 1;  

    return ticks;  

}  

 
 

static void SPI0_init(void) {  //SPI initailization 

    DDRB  |= (1 << PB5) | (1 << PB7);  

    CS_DDR  |= (1 << CS_PIN);  

    CS_PORT |= (1 << CS_PIN);  

    SPCR0 = (1 << SPE) | (1 << MSTR);  

    SPSR0 |= (1 << SPI2X);  

 
 

}  

 
 

static void SPI0_write16(uint16_t data) {  

    SPDR0 = (data >> 8);  

    while (!(SPSR0 & (1 << SPIF))) {}  

    SPDR0 = (data & 0xFF);  

    while (!(SPSR0 & (1 << SPIF))) {}  

}  

 
 

static void dac_write(uint8_t channelAB, uint8_t value) {  

    uint16_t cmd = 0;  

    cmd |= ((uint16_t)channelAB << 15);  

    cmd |= (1 << 13) | (1 << 12);  

    cmd |= ((uint16_t)value << 4);  

    CS_PORT &= ~(1 << CS_PIN);  

    SPI0_write16(cmd);  

    CS_PORT |= (1 << CS_PIN);  

}  

 
 

static void timer3_init(void) {  

    TCCR3A = 0;  

    TCCR3B = (1 << WGM32) | (1 << CS31);  

    TIMSK3 |= (1 << OCIE3A);  

    OCR3A   = 100;  

    TCNT3   = 0;  

}  

 
 

ISR(TIMER3_COMPA_vect) {  

    static uint8_t idx = 0;  

    if (idx >= 10) idx = 0;  

    uint8_t q_idx = idx + 2;  

    if (q_idx >= 10) q_idx -= 10;  

    uint8_t I_val = sine_wave[idx];  

    uint8_t Q_val = sine_wave[q_idx];  

    dac_write(0, I_val);  

    dac_write(1, Q_val);  

    idx++;  

}  

   

static void timer1_input_capture_init(void) {  

    DDRD  &= ~(1 << PD6);  

    TCCR1A = 0;  

    TCCR1B = (1 << ICES1) | (1 << CS11);  

    TIMSK1 |= (1 << ICIE1);  

    TCNT1 = 0;  

    prev_rising_edge_time = 0;  

}  

 
 

ISR(TIMER1_CAPT_vect) {  

    if (current_state == MODE_EXTERNAL) {  

        uint16_t now = ICR1;  

        uint16_t measured_period = (now - prev_rising_edge_time) & 0xFFFF;  

        prev_rising_edge_time = now;  

        period_rising_edges = measured_period;  

        uint16_t new_ocr = period_rising_edges / 10;  

        if (new_ocr < 1) new_ocr = 1;  

        OCR3A = new_ocr;  

 
 

    }  
 

}  

  

static void timer4_init(void) {  

    TCCR4A = 0;  

    TCCR4B = (1 << WGM42) | (1 << CS41);  

    OCR4A  = 10667;  

    TIMSK4 |= (1 << OCIE4A);  

    TCNT4  = 0;  

}  

 
 

   

 
 

ISR(TIMER4_COMPA_vect) {  

    if (current_state == MODE_INTERNAL) {  

        float freq = freq_table[freq_index];  

        uint16_t new_ocr = compute_OCR3A_for_10sample(freq);  

        OCR3A = new_ocr;  

        freq_index++;  

        if (freq_index >= FREQ_TABLE_SIZE) {  

            freq_index = 0;  

        }  

 
 

    }  

 
 

}   

 
 

int main(void) {  

    CLKPR = (1 << CLKPCE);  

    CLKPR = 0;  

    MODE_SWITCH_DDR &= ~(1 << MODE_SWITCH_PIN);  

    MODE_SWITCH_PUE |=  (1 << MODE_SWITCH_PIN);  

    DDRB |= (1 << LED_PIN);  

    SPI0_init();  

    timer1_input_capture_init();  

    timer3_init();  

    timer4_init();  

    OCR3A = 100;  

 
 

    sei();  

    uint8_t last_switch_state = (MODE_SWITCH_PORT & (1 << MODE_SWITCH_PIN));  

 
 

    while (1) {  

        uint8_t current_switch_state = (MODE_SWITCH_PORT & (1 << MODE_SWITCH_PIN));  

        if (current_switch_state != last_switch_state) {  

            if (!(current_switch_state)) {  

                current_state = MODE_EXTERNAL;  

                PORTB |= (1 << LED_PIN);  

                period_rising_edges = 800;  

                prev_rising_edge_time = 0;  

                OCR3A = period_rising_edges / 10;  

            } else {  

                current_state = MODE_INTERNAL;  

                PORTB &= ~(1 << LED_PIN); 

                freq_index = 0;  

                OCR3A = compute_OCR3A_for_10sample(freq_table[freq_index]);  

            }  

            last_switch_state = current_switch_state;  

        }  

        _delay_ms(20);  

    }  

}  

 
 

  

 
