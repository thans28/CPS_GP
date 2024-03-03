#include <stdint.h>
#include "msp432.h"
#include "Clock.h"

//Initialize line sensors
//Deploy battery saving techniques
void RaceMode_Init(void){
    //Reflectance Init
    P5->SEL0 &=~0x08;
    P5->SEL1 &=~0x08;
    P5->DIR |= 0x08;
    P5->OUT &=~0x08;
    P9->SEL0 &=~0x04;
    P9->SEL1 &=~0x04;
    P9->DIR |= 0x04;
    P9->OUT &=~0x04;

    P7->SEL0 &=~0xFF;
    P7->SEL1 &=~0xFF;
    P7->DIR &=~0xFF;



    Clock_Init48MHz();
}

//read in 8-bit input from line sensors
uint8_t Reflectance_Read(void){
    uint8_t result;

    P4->SEL0 &= ~0xFF;
    P4->SEL1 &= ~0xFF;    //  P4.0 as GPIO
    P4->DIR |= 0xFF; 

    P5->OUT |= 0x08;      // turn on 4 even IR LEDs
    P9->OUT |= 0x04;
    P7->DIR = 0xFF;       // make P7.7-P7.0 out
    P7->OUT = 0xFF;       // prime for measurement
    Clock_Delay1us(10);   // wait 10 us
    P7->DIR = 0x00;       // make P7.7-P7.0 in

    //-----ADD SOME SORT OF DELAY HERE--------

    P4->OUT = P7->IN&0xFF; // convert P7.0 input to digital

    P5->OUT &= ~0x08;     // turn off 4 even IR LEDs
    P9->OUT &= ~0x04;

    result = P4->OUT; // replace this line
    return result;
}

//interpret line sensor reading into usable data
uint8_t InterpretVal(uint8_t data){
    
}

//motor control
void MotorsRun(uint32_t left, uint32_t right, uint8_t dir, uint32_t time){

}
