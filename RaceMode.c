#include <stdint.h>
#include "msp432.h"
#include "Clock.h"
#include "CortexM.h"

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

    // Motors Init
    P5->SEL0 &=~0x30;
    P5->SEL1 &=~0x30;
    P2->SEL0 &=~0xC0;
    P2->SEL1 &=~0xC0;
    P3->SEL0 &=~0xC0;
    P3->SEL1 &=~0xC0;
    P5->DIR |= 0x30;
    P2->DIR |= 0xC0;
    P3->DIR |= 0xC0;
    P3->OUT |= 0xC0;

    // PWM Init --> period = 15000, duty3 = 0, duty4 = 0
    //if(duty3 >= 15000) return; // bad input
    //if(duty4 >= 15000) return; // bad input
    P2->DIR |= 0xC0;          // P2.6, P2.7 output
    P2->SEL0 |= 0xC0;         // P2.6, P2.7 Timer0A functions
    P2->SEL1 &= ~0xC0;        // P2.6, P2.7 Timer0A functions
    TIMER_A0->CCTL[0] = 0x0080;      // CCI0 toggle
    TIMER_A0->CCR[0] = 15000;       // Period is 2*period*8*83.33ns is 1.333*period
    TIMER_A0->EX0 = 0x0000;        //    divide by 1
    TIMER_A0->CCTL[3] = 0x0040;      // CCR3 toggle/reset
    TIMER_A0->CCR[3] = 0;        // CCR3 duty cycle is duty3/period
    TIMER_A0->CCTL[4] = 0x0040;      // CCR4 toggle/reset
    TIMER_A0->CCR[4] = 0;        // CCR4 duty cycle is duty4/period
    TIMER_A0->CTL = 0x02F0;        // SMCLK=12MHz, divide by 8, up-down mode
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
    case 0x00: //none
    case 0xFF: //all sensors
        output = 0x01;
        break;
    case 0x18:
    case 0x10:
    case 0x08: //center 2 sensors
        output = 0x02;
        break;
    case 0xF0: //left 4 sensors
        output = 0x03;
        break;
    case 0x0F: //right 4 sensors
        output = 0x04;
        break;
    case 0xC0:
    case 0x60: //two of the left sensors
        output = 0x05;
        break;
    case 0x06:
    case 0x03: //two of the right sensors
        output = 0x06;
        break;
    case 0x30:
    case 0x31: //two sensors just to the left of center
        output = 0x07;
        break;
    case 0x0C:
    case 0x1C: //two sensors just to the right of center
        output = 0x08;
        break;
    default: // if we get something wonky, just ignore it and stay in the same state
        output = 0x00;
        break;
    }


    return output;
}


void PWM_Duty3(uint16_t duty3){
    if(duty3 >= TIMER_A0->CCR[0]) return; // bad input
      TIMER_A0->CCR[3] = duty3;        // CCR3 duty cycle is duty3/period

}


void PWM_Duty4(uint16_t duty4){
    if(duty4 >= TIMER_A0->CCR[0]) return; // bad input
      TIMER_A0->CCR[4] = duty4;        // CCR4 duty cycle is duty4/period

}


void Motor_Stop(void){
    P5->OUT &=~0x30;
    P2->OUT &=~0xC0;
    P3->OUT &=~0xC0;
}


void Motor_Forward(uint16_t leftDuty, uint16_t rightDuty){
  // write this as part of Lab 13
    P5->OUT &=~0x30;
    P2->OUT |= 0xC0;
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}

void Motor_Right(uint16_t leftDuty, uint16_t rightDuty){
  // write this as part of Lab 13
    P5->OUT &=~0x10;
    P2->OUT |= 0x80;
    P3->OUT |= 0x80;
    P5->OUT |= 0x20;
    P2->OUT |= 0x40;
    P3->OUT |= 0x40;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}


void Motor_Left(uint16_t leftDuty, uint16_t rightDuty){
  // write this as part of Lab 13
    P5->OUT &=~0x20;
    P2->OUT |= 0x40;
    P3->OUT |= 0x40;
    P5->OUT |= 0x10;
    P2->OUT |= 0x80;
    P3->OUT |= 0x80;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}


void Motor_Backward(uint16_t leftDuty, uint16_t rightDuty){
  // write this as part of Lab 13
    P5->OUT |= 0x30;
    P2->OUT |= 0xC0;
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}


//motor control
bool MotorsRun(uint16_t left, uint16_t right, uint8_t dir, uint16_t time){
    switch (dir){
        case 0b01:
            // Reverse Mode
            P5->OUT |= 0x30;
            P2->OUT |= 0xC0;
            P3->OUT |= 0xC0;
            PWM_Duty3(right);
            PWM_Duty4(left);
            return true;
            break;
        case 0b10:
            // Left Mode
            P5->OUT &=~0x20;
            P2->OUT |= 0x40;
            P3->OUT |= 0x40;
            P5->OUT |= 0x10;
            P2->OUT |= 0x80;
            P3->OUT |= 0x80;
            PWM_Duty3(right);
            PWM_Duty4(left);
            return true;
            break;
        case 0b100:
            // Right Mode
            P5->OUT &=~0x10;
            P2->OUT |= 0x80;
            P3->OUT |= 0x80;
            P5->OUT |= 0x20;
            P2->OUT |= 0x40;
            P3->OUT |= 0x40;
            PWM_Duty3(right);
            PWM_Duty4(left);
            return true;
            break;
        case 0b1000:
            // Forward
            P5->OUT &=~0x30;
            P2->OUT |= 0xC0;
            P3->OUT |= 0xC0;
            PWM_Duty3(right);
            PWM_Duty4(left);
            return true;
            break;
        default:
            P5->OUT &=~0x30;
            P2->OUT &=~0xC0;
            P3->OUT &=~0xC0;
            return false;
            break;
}
