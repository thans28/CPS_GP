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
    uint8_t lineSense = Reflectance_Read();
    uint8_t output;

    switch(lineSense){
    case (0x00 || 0xFF): //none or all sensors
        output = 0x01;
        break;
    case 0x18: //center 2 sensors
        output = 0x02;
        break;
    case 0xF0: //left 4 sensors
        output = 0x03;
        break;
    case 0x0F: //right 4 sensors
        output = 0x04;
        break;
    case (0xC0 || 0x60): //two of the left sensors
        output = 0x05;
        break;
    case (0x06 || 0x03): //two of the right sensors
        output = 0x06;
        break;
    case 0x30: //two sensors just to the left of center
        output = 0x07;
        break;
    case 0xC0: //two sensors just to the right of center
        output = 0x08;
        break;
    default: // if we get something wonky, just ignore it and stay in the same state
        output = 0x00;
        break;
    }


    return output;
}

//motor control
void MotorsRun(uint32_t left, uint32_t right, uint8_t dir, uint32_t time){

}
