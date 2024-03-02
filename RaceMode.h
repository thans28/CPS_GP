#ifndef RACEMODE_H_
#define RACEMODE_H_

//Initialize line sensors
//Deploy battery saving techniques
void RaceMode_Init(void);

//read in 8-bit input from line sensors
uint8_t Reflectance_Read(void);

//interpret line sensor reading into usable data
uint8_t Interpreter(void);

//motor control
void MotorsRun(uint32_t left, uint32_t right, uint8_t dir);

#endif
