#ifndef BUTTON_H
#define BUTTON_H

#include"header.h"

class Button
{
    public:
    void button_setup();
    void PRINT();
};

#define LED1 39
#define LED2 17
#define LED3 38

#define key1 36
#define key2 35
#define key3 32
#define key4 26

#define Buzzer 18

void up();
void down();

void solenoid1_on();
void solenoid1_off();
void solenoid2_on();
void solenoid2_off();
void contactor1_on();
void contactor1_off();
void contactor2_on();
void contactor2_off();




#endif