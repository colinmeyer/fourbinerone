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


uint8_t display=0;    // we'll only use the bottom four bits of this 
uint8_t test_input=0; // scratch
uint8_t input=0;      // last read input
uint8_t new_input=0;  // flag set when new input is detected

// when the timer sends an interrupt, refresh the display
// and read input
ISR(WDT_vect) {
    // display the bottom four bits of display on the LEDs
    PORTB = (PORTB & 0xf0) | (0xf & display);

    // check for button change
    test_input = PINB & (1<<PB4);
    if ( input != test_input ) {
        input = test_input;
        new_input = 1;
    }
}

uint8_t c=0;
void bump_count() {
    c--;
    c &= 0b11; // c = c % 4
    display = 1<<c;
}

int main(void) {
    // PORTB is output for four output LEDs
    DDRB = (1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3);
    // Set up Port B data to be all low
    PORTB = 0;

    // set wdt prescaler
    WDTCR = (1<<WDP0);   // 4k ~32ms  8.5.2 p.43

    // Enable watchdog timer interrupts
    WDTCR |= (1<<WDTIE);

    sei(); // Enable global interrupts 

    display = 1;
    while(1) {
        if ( new_input ) {
            new_input = 0;
            if ( input ) {
                // button was just pressed
                bump_count();
            }
            else {
                // button was just released
                bump_count();
            }
        }
        // XXX not sure why, but we detect no input w/o the following line
        _delay_ms(1);
    }
    
    return 0;
}

