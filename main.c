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

#define scaler 80

State_t FSM[10]={
 {0x08,75*scaler,75*scaler,7*scaler,{Center,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x02,50*scaler,50*scaler,7*scaler,{SharpL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x04,50*scaler,50*scaler,7*scaler,{SharpR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,30*scaler, 80*scaler,7*scaler,{SlightL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,80*scaler,30*scaler,7*scaler,{SlightR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,65*scaler, 70*scaler, 7*scaler,{CenterL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,70*scaler, 65*scaler, 7*scaler,{CenterR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,45*scaler,45*scaler,10*scaler,{Err,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x01,45*scaler,45*scaler,5000*scaler,{Lost,Stop,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
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
