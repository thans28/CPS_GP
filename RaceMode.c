#include <stdint.h>
#include "msp432.h"
#include "Clock.h"

//Initialize line sensors
//Deploy battery saving techniques
void RaceMode_Init(void){

}

//read in 8-bit input from line sensors
uint8_t Reflectance_Read(void){
    return 0;
}

//interpret line sensor reading into usable data
uint8_t Interpreter(void){
    return 0;
}

//motor control
void MotorsRun(uint32_t left, uint32_t right, uint8_t dir){

}
