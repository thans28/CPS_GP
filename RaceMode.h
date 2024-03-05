#ifndef RACEMODE_H_
#define RACEMODE_H_

//Initialize line sensors
//Deploy battery saving techniques
void RaceMode_Init(void);

//extracted clock delay function - delay 1 us
void Clock_Delay1us(uint32_t n);

//read in 8-bit input from line sensors
uint8_t Reflectance_Read(void);

//interpret line sensor reading into usable data
uint8_t InterpretVal(void);

void PWM_Duty3(uint16_t duty3);

void PWM_Duty4(uint16_t duty4);

//motor control
uint8_t MotorsRun(uint16_t left, uint16_t right, uint8_t dir, uint32_t time);

#endif
