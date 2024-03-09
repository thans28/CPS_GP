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
#include "CortexM.h"
#include "TExaS.h"
#include "RaceMode.h"

struct State {
  uint8_t Dir; //indicate direction of each motor as 2 bit number XY where X is left & Y is right, 1 is forward & 0 is backward
  uint16_t LeftM; //output for left motor, initially set to 32 bit for PWM
  uint16_t RightM; //output for right motor, initially set to 32 bit for PWM
  uint32_t Time;  // time to run
  const struct State *Next[9];};// depends on 8-bit input

typedef const struct State State_t;

uint16_t errCount = 0;
uint16_t lostCount = 0;

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

#define scaler 65 //60 is good
#define timeScaler 80 //80 is good

//center delay coeff - good = 4

State_t FSM[10]={
 {0x08,85*scaler,85*scaler,5*timeScaler,{Center,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x02,50*scaler,50*scaler,6*timeScaler,{SharpL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x04,50*scaler,50*scaler,6*timeScaler,{SharpR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,30*scaler, 80*scaler,6*timeScaler,{SlightL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,80*scaler,30*scaler, 6*timeScaler,{SlightR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,65*scaler, 70*scaler, 7*timeScaler,{CenterL,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,70*scaler, 65*scaler, 7*timeScaler,{CenterR,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x08,45*scaler,45*scaler,10*timeScaler,{Err,Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x01,85*scaler,75*scaler,10*timeScaler,{Lost,Lost,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x00,0,0,1,{Stop,Stop,Stop,Stop,Stop,Stop,Stop,Stop,Stop}}
};

State_t *Mode;  // state pointer

void HandleCollision(void){
   Mode=Stop;
}

void BumpInt_Init(void(*task)(void)){
    P4->SEL0 &= ~0xED;
    P4->SEL1 &= ~0xED; // configure as GPIO
    P4->DIR &= ~0xED; // input
    P4->REN |= 0xED;
    P4->OUT |= 0xED; // pullup resistor
    P4->IES |= 0xED; //falling edge event
    P4->IFG &= ~0xED; //clear flags
    P4->IE |= 0xED; // arm interrupts
    NVIC->IP[9] = (NVIC->IP[9]&0xFF00FFFF)|0x00000000; // priority 2
    NVIC->ISER[1] = 0x00000040;        // enable interrupt 35 in NVIC
}

void PORT4_IRQHandler(void){
    P4->IFG &= ~0xED; //clear flags
    Mode = Stop;
}

int main(void){
  RaceMode_Init();
  Mode = Center; // initial state: dead center

  BumpInt_Init(&HandleCollision);
  TExaS_Init(LOGICANALYZER_P4_765320);
  EnableInterrupts();

  while(1){
    WaitForInterrupt();

    MotorsRun(Mode->LeftM,Mode->RightM,Mode->Dir,Mode->Time);

    Mode = Mode->Next[InterpretVal()];      // transition to next state

    if(Mode->Next[0] == Err){
        errCount++;
        if(errCount >= 500){
            Mode = Lost;
        }
    }else{
        errCount = 0;
    }

    if(Mode->Next[0] == Lost){
            lostCount++;
            if(lostCount >= 1500){
                Mode = Stop;
            }
        }else{
            lostCount = 0;
        }
  }
}
