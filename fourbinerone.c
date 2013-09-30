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
// 5 PB0  -->|--ww--GND
// 6 PB1  -->|--ww--GND
// 7 PB2  -->|--ww--GND
// 2 PB3  -->|--ww--GND
//
//
// 1 input
// PIN
// 3 ADC2
// 3 PB4
// 


uint8_t display=0; // we'll only use the bottom four bits of this 

// when the timer sends an interrupt, refresh the display
ISR(WDT_vect) {
    // display the bottom four bits of display on the LEDs
    PORTB = (PORTB & 0xf0) | (0xf & display);
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

    while(1) {
        // binary counter
        display++; 
        if (display & 0b10000)
            display=0;
        _delay_ms(150);
    }
    
    return 0;
}

