// Fourbinerone: 
// four LED output; one bit digital or analog input
//
// cf.
//
//     Atmel ATtiny13 manual 
//     http://www.atmel.com/Images/doc8126.pdf 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 9.6E6L / 8 // CPU Freq. Must come before delay.h include. 9.6MHz / 8
#include <util/delay.h>

// 4 output LEDs
// PIN
//   5 PB0  -->|--ww--GND
//   6 PB1  -->|--ww--GND
//   7 PB2  -->|--ww--GND
//   2 PB3  -->|--ww--GND
//
//
// 1 input
// PIN
//   3 ADC2 / PINB4
// 


volatile uint8_t display[4]; // we'll only use the bottom four bits of each cell
                             // for sixteen shades of gray
volatile uint8_t input;      // last read input
volatile uint8_t new_input;  // flag set when new input is detected
volatile uint16_t clicks;    // count interrupts fired for larger scale timing

// when the timer sends an interrupt, refresh the display
// and read input
ISR(TIM0_COMPA_vect) {
    static uint8_t display_count;

    // homemade pwm
    display_count++;
    // display_count = display_count % 8
    if ( display_count & 0xf0 ) {
        display_count = 0;
    }
    uint8_t c;
    for (c = 0; c < 4; c++) {
        if ((display[c] & 0x0f) > display_count ) {
            PORTB |= (1<<c);
        }
        else {
            PORTB &= ~(1<<c);
        }
    }

    // check for button change
    uint8_t test_input = PINB & (1<<PB4);
    if ( input != test_input ) {
        input = test_input;
        new_input = 1;
    }

    clicks++;
}


int main(void) {
    // PORTB is output for four output LEDs
    DDRB = (1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3);
    // Set up Port B data to be all low
    PORTB = 0;

    // set CTC mode - 11.9.1 P.73
    TCCR0A = 1<<WGM01;
    // set the clock source and prescaling - 11.9.2 p.73
    // ((9.6Mhz/8) / 8)*15 = .0001 = 1/10 ms
    TCCR0B |= (1<<CS00);
    // set the timer comparison register
    OCR0A = 15;

    // enable timer comparison interrupt - 11.9.6 p.75
    TIMSK0 = 1<<OCIE0A;

    sei(); // Enable global interrupts 
    
    display[0] =  1;
    display[1] =  3;
    display[2] =  7;
    display[3] = 15;

    while(1) {
        if ( new_input ) {
            new_input = 0;
            if ( input ) {
                // button was just pressed
                uint8_t c;
                for (c=0; c<4; c++) {
                    display[c] = rand() % 16;
                }
            }
            else {
                // button was just released
            }
        }

//         if ( !(clicks % 9) ) {
        if ( 0 ) {
            uint8_t c;
            for (c=0; c<4; c++) {
                display[c] = rand() % 16;
            }
        }
    }
    
    return 0;
}

