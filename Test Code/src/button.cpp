#include"header.h"

OneButton button1(key1 , true);
OneButton button2(key2 , true);
OneButton button3(key3 , true);
OneButton button4(key4 , true);

uint8_t scroll=0;


void Button :: button_setup()
{
    pinMode(LED1 , OUTPUT);
    pinMode(LED2 , OUTPUT);
    pinMode(LED3 , OUTPUT);

    digitalWrite(LED1 , LOW);
    digitalWrite(LED2 , LOW);
    digitalWrite(LED3 , LOW);

    pinMode(key1 , INPUT);
    pinMode(key2 , INPUT);
    pinMode(key3 , INPUT);
    pinMode(key4 , INPUT);

    button2.attachClick(up);
    button3.attachClick(down);
    button1.attachLongPressStart(solenoid1_on);
    button1.attachLongPressStop(solenoid1_off);
    button2.attachLongPressStart(solenoid2_on);
    button2.attachLongPressStop(solenoid2_off);
    button3.attachLongPressStart(contactor1_on);
    button3.attachLongPressStop(contactor1_off);
    button4.attachLongPressStart(contactor2_on);
    button4.attachLongPressStop(contactor2_off);

}


void up()
{
    scroll=0;
    lcd.clear();
}
void down()
{
    scroll=1;
    lcd.clear();
}



void Button :: PRINT()
{
    
    
    if(scroll == 0)
    {
        lcd.setCursor(0,0);
        lcd.print("T:");
        if(pt100_temp < 100)
        {      
            lcd.print(pt100_temp);
            lcd.print("            ");
            
        }
        else
        {
            lcd.print("PROBE ERROR");
            

        }
        lcd.setCursor(0,1);
        lcd.print("L:");
        if(level_flag == 1)
        {
            lcd.print("WATER DETECTED");
        }
        else
        {
            lcd.print("NO WATER      ");
        }
    }
    else
    {
        lcd.setCursor( 0 , 0);
        lcd.print("F:");
        if(flow_flag == 1)
        {
            lcd.print("FLOW DETECTED ");
        }
        else
        {
            lcd.print("NO FLOW       ");
        }
    }   
}







void solenoid1_on()
{
    digitalWrite(solenoid1 , HIGH);
}
void solenoid1_off()
{
    digitalWrite(solenoid1 , LOW);
}

void solenoid2_on()
{
    digitalWrite(solenoid2 , HIGH);
}
void solenoid2_off()
{
    digitalWrite(solenoid2 , LOW);
}

void contactor1_on()
{
    digitalWrite(contactor1 , HIGH);
}
void contactor1_off()
{
    digitalWrite(contactor1 , LOW);
}

void contactor2_on()
{
    digitalWrite(contactor2 , HIGH);
}
void contactor2_off()
{
    digitalWrite(contactor2 , LOW);
}
