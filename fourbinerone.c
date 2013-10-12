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


volatile uint8_t display;    // we'll only use the bottom four bits of this 
volatile uint8_t input;      // last read input
volatile uint8_t new_input;  // flag set when new input is detected
volatile uint16_t clicks;    // count interrupts fired for larger scale timing

// when the timer sends an interrupt, refresh the display
// and read input
ISR(TIM0_COMPA_vect) {
    uint8_t test_input; // scratch

    // display the bottom four bits of display on the LEDs
    PORTB = (PORTB & 0xf0) | (0xf & display);

    // check for button change
    test_input = PINB & (1<<PB4);
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
    TCCR0B |= (1<<CS01);
    // set the timer comparison register
    OCR0A = 15;

    // enable timer comparison interrupt - 11.9.6 p.75
    TIMSK0 = 1<<OCIE0A;

    sei(); // Enable global interrupts 
    
    uint16_t next_clicks;
    while(1) {
        if ( new_input ) {
            new_input = 0;
            if ( input ) {
                // button was just pressed
                // turn on the first light
                display |= 0b1000;
                next_clicks = clicks + 10000;
            }
            else {
                // button was just released
            }
        }

        // rotate display 
        if (clicks == next_clicks) {
            next_clicks += 10000;
            display >>= 1;
        }
    }
    
    return 0;
}

