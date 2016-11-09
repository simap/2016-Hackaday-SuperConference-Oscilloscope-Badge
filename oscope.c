
#include "HaD_Badge.h"
#include <stdlib.h>
#include "oscope.h"

#define N_SAMPLES 16

uint8_t samples[N_SAMPLES];
uint8_t sampleIndex;
uint8_t trigger = 128;

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

uint8_t scaleIndex = 0;



void sample(uint8_t sample) {
    if (triggerMode == RISING) {
        if (triggerState == UNTRIGGERED && sample >= trigger) {
            triggerState = TRIGGERED;
        }

        if (triggerState == TRIGGERED) {
            samples[sampleIndex] = sample;
            if (sampleIndex < N_SAMPLES)
                sampleIndex++;
            else {
                sampleIndex = 0;
                triggerState = RESETTING;
            }
        }

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
    if (CCP1IF) {
        GO=1;
        TMR1 = 0;
        CCP1IF = 0;
    }
    
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
    
    //This shows how to get user input
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
    
    T0IE = 0;
    for (i = 0; i < 15; i++) {
     Buffer[i] = (1<< (samples[i]>>5));
    }
    Buffer[15] = 1 << timingIndex;
    T0IE = 1;
}

void oscopeRun() {
    int i;
    
    Brightness = 15;
    
    TRISB0 = 1;    
    ANSB0 = 1;
    
    TRISB4 = 1;    
    ANSB4 = 1;
    
//    ADCON0bits.CHS = 0b01100; //AN12, B0
    ADCON0bits.CHS = 0b01011; //AN11, B4
    
//    TRISB3 = 0;
    
//    VREFCON0bits.FVREN = 1;
//    VREFCON0bits.FVRS = 0b10; //2.048v 
    
    ADFM = 0; //left align 10 bits, 8 msb in ADRESH
    ADCON2bits.ADCS = 0b110; //FOSC/64 = 1.3us
    ADCON2bits.ACQT = 0b111;
    ADCON0bits.ADON = 1;
    //ADCON1bits.PVCFG = 0b10; //use FVR

    CCPR1 = timings[timingIndex];
    CCP1CONbits.CCP1M = 0b1010;
    CCP1IE = 1;
    
    T1CON = 0b00000111;
//    T1CONbits.T1CKPS = 0b00; //1:1 12M
    T1CONbits.T1CKPS = 0b11; //1:8 1.5M

    ADIF = 0;
    ADIE = 1;
    
    for (i = 0; i < N_SAMPLES; i++) {
        samples[i] = 0;
    }

    RCIE = 0;
    TXIE = 0;
    TMR2IE = 0;
    CCP2IE = 0;

    while(1) {
        handleInput();
        drawSamples();
    }
}
