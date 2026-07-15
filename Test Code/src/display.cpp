#include"header.h"

LiquidCrystal_I2C lcd(0x27 , 16 , 2);

Ticker level(level_s , 1000);
Ticker flow(flow_s , 1000);

float R0 = 100.0;
float R1 = 100.0;
float R2 = 4700.0;
float R3 = 4700.0;
const float Alpha = 0.00385;
const float Vs = 5.0;

float pt100_val;
float pt100_Vout;
float pt100_Vin;
float pt100_Rt;
float pt100_temp;

static float total_tem=0;
static int flag=0;

bool level_flag=0 ,flow_flag=0;
static int num=0 ;

void LCD :: lcd_setup()
{
    lcd.begin(16,2);
    lcd.backlight();
    lcd.setCursor(0,0);
    level.start();
    flow.start();
    pinMode(Buzzer , OUTPUT);
    digitalWrite(Buzzer , LOW);

    pinMode(power_detection_pin , OUTPUT);
    pinMode(water_detection_pin , INPUT_PULLUP);

    pinMode(contactor1 , OUTPUT);
    pinMode(contactor2 , OUTPUT);
    pinMode(solenoid1 ,OUTPUT);
    pinMode(solenoid2 , OUTPUT);
    pinMode(heater1 , OUTPUT);
    pinMode(heater2 , OUTPUT);
}



void LCD :: version_dis()
{
    if(millis() < 6000)
    {
        // lcd.clear();
        if(millis() < 3000)
        {
            lcd.setCursor( 0 , 0);
            lcd.print("BOROSIL LABQUEST");
            lcd.setCursor(0,1);
            lcd.print(" WDU_DISP_V1.5");
            digitalWrite(Buzzer , HIGH);
        }
        
        else
        {
            digitalWrite(Buzzer , LOW);
            for (int row = 0; row < 2; row++)
            {
                lcd.setCursor(0, row);
                for (int col = 0; col < 16; col++)
                {
                    lcd.write(255);  
                }
            }
        }
    }
    else
    {
        lcd.clear();
        version_display=0;
        digitalWrite(LED1 , HIGH);
        digitalWrite(LED2 , HIGH);
        digitalWrite(LED3 , HIGH);
    }
}

void LCD :: disp_pt100()
{
    if(flag < 20)
    {
        pt100_val = analogRead(Pt100_1_pin);
        pt100_Vout = ( pt100_val * Vs ) / 1024.0;
        total_tem = total_tem + pt100_Vout;
        flag++;
    }
    else
    {
        // lcd.clear();
        pt100_Vout=total_tem/20;
        pt100_Vin = pt100_Vout / 10.0;
        pt100_Rt =(R3 * (((R1 + R2) * pt100_Vin) / Vs + R1)) / (R2 - ((R1 + R2) * pt100_Vin / Vs));
        pt100_temp = ((pt100_Rt /R1 )-1) / Alpha;
        Serial2.print("RTD1 -> ");
        Serial2.println(pt100_temp);
        
        total_tem=0;
        flag=0;
    }

   
    
}





void LCD :: disp_level_sensor()
{
    level.update();
    
}


void level_s()
{
    digitalWrite(power_detection_pin , HIGH);

    if(digitalRead(water_detection_pin) == LOW)
    {
        level_flag=1;
    }
    else
    {
        level_flag=0;
    }
    digitalWrite(power_detection_pin , LOW);
}

void LCD :: disp_flow_sensor()
{
    flow.update();
}

void flow_s()
{
    if(digitalRead(flow_pin) == HIGH)
    {
        flow_flag=0;
    }
    else
    {
        flow_flag=1;
    }
}