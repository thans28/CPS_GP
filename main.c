// main.c
// Runs on MSP432

// University of Pittsburgh
// Department of Electrical and Computer Engineering
// ECE 1188: Cyber-Physical Systems
// Design Project 1 - Line-Following Robot Race

// Team Koopa
// Kushal Parekh, Oday Abushaban, and Tyler Hansen

#include <stdint.h>
#include "msp.h"

struct State {
  uint8_t Dir; //indicate direction of each motor as 2 bit number XY where X is left & Y is right, 1 is forward & 0 is backward
  uint16_t LeftM; //output for left motor, initially set to 32 bit for PWM
  uint16_t RightM; //output for right motor, initially set to 32 bit for PWM
  uint32_t Time;  // time to run
  const struct State *Next[9];};// depends on 4-bit input

typedef const struct State State_t;

uint16_t errCount = 0;
volatile uint8_t Bump = 0x00;
volatile uint8_t numInts = 0;

#define Center &FSM[0]
#define SharpL &FSM[1]
#define SharpR &FSM[2]
#define SlightL &FSM[3]
#define SlightR &FSM[4]
#define CenterL &FSM[5]
#define CenterR &FSM[6]
#define Err &FSM[7]
#define Lost &FSM[8]
#define Stop &FSM[9]

#define mScaler 80
#define tScaler 80

State_t FSM[10]={
 {0x08,75*mscaler,75*mscaler,7*tscaler,{Center,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x02,50*mscaler,50*mscaler,7*tscaler,{SharpL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x04,50*mscaler,50*mscaler,7*tscaler,{SharpR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,30*mscaler, 80*mscaler,7*tscaler,{SlightL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,80*mscaler,30*mscaler,7*tscaler,{SlightR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,65*mscaler, 70*mscaler, 7*tscaler,{CenterL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,70*mscaler, 65*mscaler, 7*tscaler,{CenterR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,45*mscaler,45*mscaler,10*tscaler,{Err,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x01,45*mscaler,45*mscaler,5000*tscaler,{Lost,Stop,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x00,0,0,1,{Stop,Stop,Stop,Stop,Stop,Stop,Stop,Stop,Stop}}
};

State_t *Mode;  // state pointer

void SysTick_Handler(void){
    numInts++;

    if(numInts==10){
        Bump = (~(((P4->IN)&0x01) | (((P4->IN)&0x0C)>>1) | (((P4->IN)&0xE0)>>2)))&~0xC0;

        if(Bump != 0x00){
            Mode = Stop;
        }
    }
}

int main(void){
  RaceMode_Init();

  Mode = Center;                    // initial state: dead center
  while(MotorsRun(Mode->LeftM,Mode->RightM,Mode->Dir,Mode->Time)!=0x00){

    Mode = Mode->Next[InterpretVal()];      // transition to next state

    if(Mode->Next[0] == Err){
        errCount++;
        if(errCount >= 500){
            Mode = Lost;
        }
    }else{
        errCount = 0;
    }
  }
}
