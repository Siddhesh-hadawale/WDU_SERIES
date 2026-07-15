#include <Arduino.h>
#include "Ext_var.h"

#define BUZZER 18
void Buzzer_on_off();
void heater2_start();
void heater_stop();
Ticker buzzer(Buzzer_on_off,1000,0,MILLIS);

// Definations

buzzerclass::buzzerclass()
{}

void buzzerclass:: buzzer_setup()
{
    pinMode(BUZZER,OUTPUT);
}

void buzzerclass:: buzzer_update()
{
    buzzer.update();
}

void buzzerclass ::Buzzer_start()
{
   buzzer.start();
}

void buzzerclass ::Buzzer_beep(uint32_t interval1)
{
    buzzer.interval(interval1);
}

void buzzerclass:: buzzer_on()
{
    digitalWrite(BUZZER,HIGH);

}

void buzzerclass:: buzzer_off()
{
    digitalWrite(BUZZER,LOW);

}

void buzzerclass:: buzzer_beep()
{
    buzzerclass_object.buzzer_on();
    delay(1000);
    buzzerclass_object.buzzer_off();
}

void Buzzer_on_off()
{
    digitalWrite(BUZZER,LOW);
    buzzer.stop(); 
}

buzzerclass buzzerclass_object = buzzerclass();