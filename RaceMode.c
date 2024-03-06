#include <stdint.h>
#include "msp432.h"

//Initialize line sensors
//Deploy battery saving techniques

uint32_t ClockFrequency = 3000000;
uint32_t Prewait = 0;
uint32_t CPMwait = 0;
uint32_t Postwait = 0;                  // loops between Current Power Mode matching requested mode and PCM module idle (expect about 0)
uint32_t IFlags = 0;                    // non-zero if transition is invalid
uint32_t Crystalstable = 0;

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

    //Clock 48MHZ Init
    // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    while(PCM->CTL1&0x00000100){
  //  while(PCMCTL1&0x00000100){
      Prewait = Prewait + 1;
      if(Prewait >= 100000){
        return;                           // time out error
      }
    }
    // request power active mode LDO VCORE1 to support the 48 MHz frequency
    PCM->CTL0 = (PCM->CTL0&~0xFFFF000F) |     // clear PCMKEY bit field and AMR bit field
  //  PCMCTL0 = (PCMCTL0&~0xFFFF000F) |     // clear PCMKEY bit field and AMR bit field
              0x695A0000 |                // write the proper PCM key to unlock write access
              0x00000001;                 // request power active mode LDO VCORE1
    // check if the transition is invalid (see Figure 7-3 on p344 of datasheet)
    if(PCM->IFG&0x00000004){
      IFlags = PCM->IFG;                    // bit 2 set on active mode transition invalid; bits 1-0 are for LPM-related errors; bit 6 is for DC-DC-related error
      PCM->CLRIFG = 0x00000004;             // clear the transition invalid flag
      // to do: look at CPM bit field in PCMCTL0, figure out what mode you're in, and step through the chart to transition to the mode you want
      // or be lazy and do nothing; this should work out of reset at least, but it WILL NOT work if Clock_Int32kHz() or Clock_InitLowPower() has been called
      return;
    }
    // wait for the CPM (Current Power Mode) bit field to reflect a change to active mode LDO VCORE1
    while((PCM->CTL0&0x00003F00) != 0x00000100){
      CPMwait = CPMwait + 1;
      if(CPMwait >= 500000){
        return;                           // time out error
      }
    }
    // wait for the PCMCTL0 and Clock System to be write-able by waiting for Power Control Manager to be idle
    while(PCM->CTL1&0x00000100){
      Postwait = Postwait + 1;
      if(Postwait >= 100000){
        return;                           // time out error
      }
    }
    // initialize PJ.3 and PJ.2 and make them HFXT (PJ.3 built-in 48 MHz crystal out; PJ.2 built-in 48 MHz crystal in)
    PJ->SEL0 |= 0x0C;
    PJ->SEL1 &= ~0x0C;                    // configure built-in 48 MHz crystal for HFXT operation
  //  PJDIR |= 0x08;                      // make PJ.3 HFXTOUT (unnecessary)
  //  PJDIR &= ~0x04;                     // make PJ.2 HFXTIN (unnecessary)
    CS->KEY = 0x695A;                     // unlock CS module for register access
    CS->CTL2 = (CS->CTL2&~0x00700000) |   // clear HFXTFREQ bit field
             0x00600000 |                 // configure for 48 MHz external crystal
             0x00010000 |                 // HFXT oscillator drive selection for crystals >4 MHz
             0x01000000;                  // enable HFXT
    CS->CTL2 &= ~0x02000000;              // disable high-frequency crystal bypass
    // wait for the HFXT clock to stabilize
    while(CS->IFG&0x00000002){
      CS->CLRIFG = 0x00000002;              // clear the HFXT oscillator interrupt flag
      Crystalstable = Crystalstable + 1;
      if(Crystalstable > 100000){
        return;                           // time out error
      }
    }
    // configure for 2 wait states (minimum for 48 MHz operation) for flash Bank 0
    FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL&~0x0000F000)|FLCTL_BANK0_RDCTL_WAIT_2;
    // configure for 2 wait states (minimum for 48 MHz operation) for flash Bank 1
    FLCTL->BANK1_RDCTL = (FLCTL->BANK1_RDCTL&~0x0000F000)|FLCTL_BANK1_RDCTL_WAIT_2;
    CS->CTL1 = 0x20000000 |               // configure for SMCLK divider /4
             0x00100000 |                 // configure for HSMCLK divider /2
             0x00000200 |                 // configure for ACLK sourced from REFOCLK
             0x00000050 |                 // configure for SMCLK and HSMCLK sourced from HFXTCLK
             0x00000005;                  // configure for MCLK sourced from HFXTCLK
    CS->KEY = 0;                          // lock CS module from unintended access
    ClockFrequency = 48000000;
  //  SubsystemFrequency = 12000000;



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

    //SysTick Init
    SysTick->CTRL = 0;              // 1) disable SysTick during setup
    SysTick->LOAD = 48000 - 1;     // 2) reload value sets period
    SysTick->VAL = 0;               // 3) any write to current clears it
    SCB->SHP[11] = 2<<5;     // set priority into top 3 bits of 8-bit register
    SysTick->CTRL = 0x00000007;     // 4) enable SysTick with core clock and interrupts

    //Bump Init
    P4->SEL0 &= ~0xED;
    P4->SEL1 &= ~0xED; // configure as GPIO
    P4->DIR &= ~0xED; // input
    P4->REN |= 0xED;
    P4->OUT |= 0xED; // pullup resistor
}

void BatterySave(void){
    P1->OUT = 0x00;
    P6->OUT = 0x00;
    P8->OUT &=~ 0xC0;
}

void Clock_Delay1us(uint32_t n){
  n = (382*n)/100;; // 1 us, tuned at 48 MHz
  while(n){
    n--;
  }
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

    Clock_Delay1us(1000);

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
    case 0xE0:
    case 0xC0:
    case 0x80:
        output = 0x03;
        break;
    case 0x0F: //right 4 sensors
    case 0x07:
    case 0x03:
    case 0x01:
        output = 0x04;
        break;
    case 0x40:
    case 0x20:
    case 0x60: //two of the left sensors
        output = 0x05;
        break;
    case 0x06: //two of the right sensors
    case 0x02:
    case 0x04:
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

//motor control
uint8_t MotorsRun(uint16_t left, uint16_t right, uint8_t dir, uint32_t time){
    uint8_t keepGoing=0x01;
    switch (dir){
        case 0x01:
            // Reverse Mode
            P5->OUT |= 0x30;
            P2->OUT |= 0xC0;
            P3->OUT |= 0xC0;
            PWM_Duty3(right);
            PWM_Duty4(left);
            break;
        case 0x02:
            // Left Mode
            P5->OUT &=~0x20;
            P2->OUT |= 0x40;
            P3->OUT |= 0x40;
            P5->OUT |= 0x10;
            P2->OUT |= 0x80;
            P3->OUT |= 0x80;
            PWM_Duty3(right);
            PWM_Duty4(left);
            break;
        case 0x04:
            // Right Mode
            P5->OUT &=~0x10;
            P2->OUT |= 0x80;
            P3->OUT |= 0x80;
            P5->OUT |= 0x20;
            P2->OUT |= 0x40;
            P3->OUT |= 0x40;
            PWM_Duty3(right);
            PWM_Duty4(left);
            break;
        case 0x08:
            // Forward
            P5->OUT &=~0x30;
            P2->OUT |= 0xC0;
            P3->OUT |= 0xC0;
            PWM_Duty3(right);
            PWM_Duty4(left);
            break;
        default:
            // Stop Motors, Return False
            P5->OUT &=~0x30;
            P2->OUT &=~0xC0;
            P3->OUT &=~0xC0;
            keepGoing=0x00;
            break;
    }
    Clock_Delay1us(time);
    return keepGoing;
}
