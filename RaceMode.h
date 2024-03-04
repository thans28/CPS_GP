#ifndef RACEMODE_H_
#define RACEMODE_H_

//Initialize line sensors
//Deploy battery saving techniques
void RaceMode_Init(void);

//read in 8-bit input from line sensors
uint8_t Reflectance_Read(void);

//interpret line sensor reading into usable data
uint8_t InterpretVal(void);

void PWM_Duty3(uint16_t duty3);

void PWM_Duty4(uint16_t duty4);

void Motor_Stop(void);

void Motor_Forward(uint16_t leftDuty, uint16_t rightDuty);

void Motor_Right(uint16_t leftDuty, uint16_t rightDuty);

void Motor_Left(uint16_t leftDuty, uint16_t rightDuty);

void Motor_Backward(uint16_t leftDuty, uint16_t rightDuty);

//motor control
void MotorsRun(uint32_t left, uint32_t right, uint8_t dir, uint32_t time);

#endif
