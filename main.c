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
#include "Clock.h"
#include "TExaS.h"

struct State {
  uint8_t Dir //indicate direction of each motor as 2 bit number XY where X is left & Y is right, 1 is forward & 0 is backward
  uint32_t LeftM; //output for left motor, initially set to 32 bit for PWM
  uint32_t RightM; //output for right motor, initially set to 32 bit for PWM
  uint32_t Time;  // time to run
  const struct State *Next[8];};// depends on 4-bit input

typedef const struct State State_t;

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

State_t FSM[10]={
 {0x03,9000,9000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x01,9000,9000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x02,9000,9000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x03,3000,9000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x03,9000,3000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x03,9000,7000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x03,7000,9000,1000,{Err,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x03,3000,3000,1000,{Lost,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x00,3000,3000,1000,{Stop,Center,SharpL,SharpR,SlightL,SlightR,CenterL,CenterR}},
 {0x00,0,0,10000,{Stop,Stop,Stop,Stop,Stop,Stop,Stop,Stop}}
};


int main(void){
  State_t *Mode;  // state pointer

  Clock_Init48MHz();              // initialize clock to 48MHz
  TExaS_Init(LOGICANALYZER_P4);

  Mode = Center;                    // initial state: dead center
  while(1){
                               // set lights to current state's Out value
    MotorsRun(Mode->LeftM,Mode->RightM,Mode->Dir);

    Clock_Delay1ms(Mode->Time);  // wait 1 ms * current state's Time value

    Mode = Mode->Next[(InterpretVal())];      // transition to next state
  }
}
