// Fourbinerone: 
// four LED output; one bit digital or analog input
//
// cf.
//
//     Atmel ATtiny13 manual 
//     http://www.atmel.com/Images/doc8126.pdf 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

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


#define NEXT_DECAY_COUNT 300

volatile uint8_t display[4]; // we'll only use four bits of each cell
                             // for sixteen shades of gray -- at a time
                             // if flags & FRAME_BUFFER, then we'll use the top four
                             // else the bottom

volatile uint8_t input;      // last read input
volatile uint16_t _clicks;    // count interrupts fired for larger scale timing

volatile uint8_t flags;      

enum FLAGS {
    // when the first bit of flags is set, then new input has occured
    NEW_INPUT    = 0b00000001,
    // when the second bit of flags is 0, then the lower half of the bytes in
    // display[] is displayed when it is 1, then the upper half is used
    FRAME_BUFFER = 0b00000010,
    // direction is >> when 0 or << when 1
    DIRECTION    = 0b00000100,
    // flag set when it's time to decay
    NEXT_DECAY   = 0b00001000,
    NEXT_ANIM    = 0b00010000
};

//////////////////////////////////
// frame buffer manipulation
//////////////////////////////////
void set_hidden_fb(uint8_t cell, uint8_t value) {
    uint8_t visible_fb;
    if (flags & FRAME_BUFFER) {
        // setting lower half of byte; preserve upper (visible half)
        visible_fb = 0xf0;
        value     &= 0x0f;
    }
    else {
        visible_fb = 0x0f;
        // left shift value by four, for placing in the high bit frame buffer
        value <<= 4;
    }
    display[cell] = (display[cell] & visible_fb) | value;
}

uint8_t get_visible_fb(uint8_t cell) {
    uint8_t value = display[cell];
    if (flags & FRAME_BUFFER) {
         value >>= 4;
    }
    else {
         value &= 0x0f;
    }
    return value;
}

void switch_fb() {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        flags ^= FRAME_BUFFER;
    }
}

void switch_off_input() {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        flags &= ~(NEW_INPUT); 
    }
}

uint8_t read_clear_flag(uint8_t FLAG) {
    uint8_t is_set = flags & FLAG;

    if (is_set) {
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
            flags &= ~(FLAG);
        }
    }

    return is_set;
}


// when the timer sends an interrupt, refresh the display
// and read input
ISR(TIM0_COMPA_vect) {
    static uint8_t display_count;

    // homemade pwm

    // loop from 0x0 to 0xf repeatedly
    display_count = (display_count + 1) & 0x0f;
    uint8_t c;
    for (c = 0; c < 4; c++) {
        if (get_visible_fb(c) > display_count ) {
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
        flags |= NEW_INPUT;
    }

    _clicks++;
    // make the integer rollover happen on a boundary that is divisible by lots
    // of shit
    if (_clicks == 60000)
        _clicks = 0;

    // set app level timer flags, as appropriate
    if ( _clicks % NEXT_DECAY_COUNT == 0 )
        flags |= NEXT_DECAY;
    if ( _clicks % NEXT_ANIM_COUNT  == 0 )
        flags |= NEXT_ANIM;
}


void decay_display() {
    for (uint8_t c=0; c<4; c++) {
        set_hidden_fb(
            c,
            get_visible_fb(c) / 2
        );
    }
    switch_fb();
}

typedef int (*funcptr)();          // http://c-faq.com/decl/recurfuncp.html
typedef funcptr (*ptrfuncptr)();

funcptr setup(), wait();

int main(void) {
    //----------------------------------------------------
    // chip setup

    // PORTB is output for four output LEDs
    DDRB = (1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3);

    // set CTC mode - 11.7.2 p.64
    //    registers - 11.9.1 P.73
    TCCR0A = 1<<WGM01;
    // set the clock source and prescaling - 11.9.2 p.73
    // ((9.6Mhz/8) / 8) = 150Khz
    TCCR0B |= (1<<CS01);
    // set the timer comparison register
    // 15 timer clock cycles = .1ms
    OCR0A = 15;

    // enable timer comparison interrupt - 11.9.6 p.75
    TIMSK0 = 1<<OCIE0A;

    sei(); // Enable global interrupts 

    // 
    //----------------------------------------------------
    
    // state machine with visual decay
    ptrfuncptr state = setup;

    while(1) {
        state = (ptrfuncptr)(*state)();

        if (read_clear_flag(NEXT_DECAY)) {
            decay_display();
        }
    }

    return 0;
}


funcptr setup() {
    for (uint8_t c=0; c<4; c++) {
       set_hidden_fb(c, (1<<(c+1)) - 1);
    }
    switch_fb();

    return (funcptr) wait;
}

funcptr wait() {
    return (funcptr) wait;
}
