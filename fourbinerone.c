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
volatile uint8_t clicks;     // count interrupts fired for larger scale timing

// when the timer sends an interrupt, refresh the display
// and read input
ISR(WDT_vect) {
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

    // set wdt prescaler
    WDTCR = (1<<WDP0);   // 4k ~32ms  8.5.2 p.43

    // Enable watchdog timer interrupts
    WDTCR |= (1<<WDTIE);

    sei(); // Enable global interrupts 
    
    uint8_t next_clicks;
    while(1) {
        if ( new_input ) {
            new_input = 0;
            if ( input ) {
                // button was just pressed
                // turn on the first light
                display |= 0b1000;
                next_clicks = clicks + 4;
            }
            else {
                // button was just released
            }
        }

        // rotate display 
        if (clicks == next_clicks) {
            next_clicks += 4;
            display >>= 1;
        }
    }
    
    return 0;
}

