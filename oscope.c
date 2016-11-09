
#include "HaD_Badge.h"
#include <stdlib.h>
#include "oscope.h"

#define N_SAMPLES 16

uint8_t samples[N_SAMPLES];
uint8_t sampleIndex;
uint8_t trigger = 128; //set this to the threshold needed to start capturing a set of samples

enum {
    RESETTING,
    UNTRIGGERED,
    TRIGGERED,
} triggerState = UNTRIGGERED;

enum {
    RISING,
    CONTINUOUS,
} triggerMode = RISING;


#define calcPr(t) ((12000000/8)/t)
uint8_t timingIndex = 3;
#define N_TIMINGS 8
const unsigned short timings[] = {
    calcPr(100),
    calcPr(200),
    calcPr(500),
    calcPr(1000),
    calcPr(2000),
    calcPr(5000),
    calcPr(10000),
    calcPr(20000),
};

void sample(uint8_t sample) {
    if (triggerMode == RISING) {
        //wait for a value high enough to trigger
        if (triggerState == UNTRIGGERED && sample >= trigger) {
            triggerState = TRIGGERED;
        }

        //capture samples until buffer is full
        if (triggerState == TRIGGERED) {
            samples[sampleIndex] = sample;
            if (sampleIndex < N_SAMPLES)
                sampleIndex++;
            else {
                sampleIndex = 0;
                triggerState = RESETTING;
            }
        }
        
        //wait for value to reset below trigger threshold
        if (triggerState == RESETTING && sample < trigger) {
            triggerState = UNTRIGGERED;
        }
    } else {
        //continuous
        samples[sampleIndex] = sample;
        if (sampleIndex < N_SAMPLES) {
            sampleIndex++;
        } else {
            sampleIndex = 0;
        }
    }
}

void oscopeIsr() {
    //when TMR1 compare fires, start up ADC and reset TMR1
    if (CCP1IF) {
        GO=1;
        TMR1 = 0;
        CCP1IF = 0;
    }
    
    //when ADC is done, store the sample
    if (ADIF) {
        sample(ADRESH);
        ADIF = 0;
    }
}


void nextTiming() {
    timingIndex = (timingIndex + 1) % N_TIMINGS;
    CCPR1 = timings[timingIndex];
}


void handleInput() {
    switch (getControl()) {
        case (ESCAPE):
            return;
        case (LEFT):
            break;
        case (RIGHT):
            break;
        case (UP):
            nextTiming();
            break;
        case (DOWN):
            break;
    }
}

void drawSamples() {
    int i;
    
    T0IE = 0; //needed to avoid strange drawing artifacts. Probably the crappy free compiler putting intermediate results in Buffer
    for (i = 0; i < 15; i++) {
        //draw a dot based on the top 3 bits of the sample
        //gives roughly 0.375v/div on a 3v battery
        Buffer[i] = (1<< (samples[i]>>5));
    }
    //draw a dot to indicate the timing mode
    Buffer[15] = 1 << timingIndex;
    T0IE = 1;
}

void oscopeRun() {
    int i;
    
    Brightness = 15;
    
    //set up b4 as the scope input
    TRISB4 = 1;    
    ANSB4 = 1;
    ADCON0bits.CHS = 0b01011; //AN11, B4
    
    ADFM = 0; //left align 10 bits, 8 msb in ADRESH
    ADCON2bits.ADCS = 0b110; //FOSC/64 = 1.3us
    ADCON0bits.ADON = 1;
    ADIF = 0;
    ADIE = 1;
    
    //enable these to use the FVR of 2.048v as ADC ref.
//    VREFCON0bits.FVREN = 1;
//    VREFCON0bits.FVRS = 0b10; //2.048v     
//    ADCON1bits.PVCFG = 0b10; //use FVR

    //init the TMR1 compare based on default timing setting
    CCPR1 = timings[timingIndex];
    //NOTE we could have been able to automatically trigger ADC and reset the time if CCP2 was used instead of CCP1
    CCP1CONbits.CCP1M = 0b1010; //Compare mode: generate software interrupt on compare match
    CCP1IE = 1;
    
    T1CON = 0b00000111; //enable tmr1, 16 bit rw mode, no sync (doesn't matter))
    T1CONbits.T1CKPS = 0b11; //12MIPS 1:8 = 1.5M/s

    for (i = 0; i < N_SAMPLES; i++) {
        samples[i] = 0;
    }

    //because we don't use any of the IR stuff, disable interrupts related to those peripherals (not sure this is needed)
    RCIE = 0;
    TXIE = 0;
    TMR2IE = 0;
    CCP2IE = 0;

    while(1) {
        handleInput();
        drawSamples();
    }
}
