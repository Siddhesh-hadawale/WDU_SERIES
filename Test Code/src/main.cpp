#include"header.h"
bool version_display = 1 ;
int version=0;

void setup()
{
  button.button_setup();
  Lcd.lcd_setup();
  Serial2.swap(1);
  Serial2.pins(8,9);
  Serial2.begin(9600);
  
}

void loop()
{
  if(version_display == 1)
  {
    Lcd.version_dis();
  }
  else
  {
    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();
    Lcd.disp_pt100();
    Lcd.disp_flow_sensor();
    Lcd.disp_level_sensor();
    button.PRINT();
  }
 
}

