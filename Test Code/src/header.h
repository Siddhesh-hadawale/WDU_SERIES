#ifndef HEADER_H
#define HEADER_H

#include<Arduino.h>
#include<Ticker.h>
#include<OneButton.h>
#include<LiquidCrystal_I2C.h>


#include"button.h"
#include"display.h"

extern Button button; 
extern LCD Lcd;


extern OneButton button1;
extern OneButton button2;
extern OneButton button3;
extern OneButton button4;

extern LiquidCrystal_I2C lcd;


extern bool version_display;
extern int version;


extern float pt100_temp;
extern bool level_flag;
extern bool flow_flag;

void pt100_temperature();

#endif