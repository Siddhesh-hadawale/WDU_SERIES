#include <Arduino.h>
#include "Ext_Var.h"

// LED and Buzzer Pins
#define RED_LED 17 //HELLO
#define YELLOW_LED 39
#define GREEN_LED 38
#define BUZZER 18



void toggle_screen();
LiquidCrystal_I2C lcd(0X27,16,2);
int screen=0;
int optimecounter=0;
int prodtypecounter=0;
int Sec_time=0;
int remaining_time=0;   

bool toggle=0;
bool dduflag=0;
bool flow_check=0;
bool level_check=0;
bool probe_check=0;
bool check_flag=0; // Flag to indicate if error has been acknowledged by user


unsigned long solenoid_stop=0;

int optime[3]={3,6,8};
int prodtype[3]={150,250,400};
// int Sfill_time[3]={10*240,(11*144)+60,13*90};
int Sfill_default[3]={40 ,28 ,20 };
int sfill_time=0;
float base_calibration[3]={1.4,2.4,3.9};

// Error blinking ticker for main screen
Ticker error_blink(toggle_screen,500,0,MILLIS);

void Override_alert()
{
    if(toggle)
    {
        if(flowoverride  && !flow_check)
        {
            digitalWrite(BUZZER,HIGH);
            lcd.setCursor(0,0);
            lcd.print("  FLOW SENSOR        ");
            lcd.setCursor(0,1);
            lcd.print("    OVERRIDE             ");
            if (check_flag)
            {
                flow_check = 1; // Mark flow error as acknowledged
                check_flag = 0; // Reset check flag for next error
            }
        }
        else if(leveloverride && !level_check )
        {
            digitalWrite(BUZZER,HIGH);
            lcd.setCursor(0,0);
            lcd.print("  LEVEL SENSOR       ");
            lcd.setCursor(0,1);
            lcd.print("    OVERRIDE             ");
            if (check_flag)
            {
                level_check = 1; // Mark level error as acknowledged
                check_flag = 0; // Reset check flag for next error
            }
        }
        else if(probeoverride && !probe_check && dduflag)
        {
            digitalWrite(BUZZER,HIGH);
            lcd.setCursor(0,0);
            lcd.print("  PT100 PROBE     ");
            lcd.setCursor(0,1);
            lcd.print("    OVERRIDE             ");
            if (check_flag)
            {
                probe_check = 1; // Mark probe error as acknowledged
                check_flag = 0; // Reset check flag for next error
            }
        }
        else
        {
            // digitalWrite(BUZZER,LOW);
            process_object.Check_ticker_stop(); // Stop the warning ticker if no overrides are active
            lcd.clear();
            check_flag = 0;
            override_alert_flag = 0; // Reset override alert flag
            probe_check=0;
            level_check=0;
            flow_check=0;
        }
    }
    else{
        // digitalWrite(BUZZER,LOW);
        lcd.setCursor(0,0);
        lcd.print("               ");
        lcd.setCursor(0,1);
        lcd.print("               ");
    }

}
 
//DEFINATIONS
lcdclass::lcdclass()
{}

void toggle_screen()
{
    toggle=!toggle;
}
// -------- LCD INTIALIZATION --------
void lcdclass::lcd_setup()
{
    Wire.begin();
    Wire.setClock(100000);
    lcd.backlight();
    lcd.init();
    
    delay(100);
    // lcd.noBacklight();
    lcd.print(" SELECT VOLUME   ");
    lcd.noAutoscroll();


    error_blink.start();

    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);

    variant = ((prodtype[prodtypecounter]) / 10);
    screen = VersionScreen;
}

void lcdclass:: lcd_blink_update()
{
    error_blink.update();

}

void lcdclass::lcd_display()
{

    switch(screen)
    {
        case VersionScreen:
            lcd.setCursor(0,0);
            lcd.print("LABQUEST BOROSIL");
            lcd.setCursor(1,1);
            if(dduflag)
            {
                lcd.print("DDU ");// V3.00");
                lcd.print(prodtype[prodtypecounter]);
                lcd.print(" V3.00");
            }
            else
            {
                lcd.print("SDU ");// V3.00");
                lcd.print(prodtype[prodtypecounter]);
                lcd.print(" V3.00");

            }
            // lcd.print("WDU V3.00");
            digitalWrite(BUZZER,HIGH);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();
            delay(2000);
            screen++;
            lcd.clear();
            mainscreenflag=1;
        
        break;

        case MainScreen:
            downpointer=0;
            digitalWrite(YELLOW_LED,HIGH);
            digitalWrite(GREEN_LED,LOW);
            digitalWrite(RED_LED,LOW);
            lcd.setCursor(0,0);
            lcd.print(" SELECT VOLUME   ");     
            lcd.setCursor(3,1);
            lcd.print(counter,1);
            if(counter<10.0){
                lcd.setCursor(6,1);
                lcd.print(" ");
                lcd.print("LITERS ");
            }
            else
            {
                lcd.setCursor(7,1);
                lcd.print("");
                lcd.print(" LITERS");
            }
            buttonClass_object.but_check();
        break;

        case UserSettingsScreen1:
            mainscreenflag=0;
            digitalWrite(GREEN_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            lcd.setCursor(1,0);
            lcd.print("SECONDARY FILL   ");    
            lcd.setCursor(1,1);
            lcd.print("FLOW CONTROL    "); 
        break;

        case UserSettingsScreen2:
            lcd.setCursor(1,0);
            lcd.print("FLOW CONTROL   ");     
            lcd.setCursor(1,1);
            lcd.print("LEVEL CONTROL  ");
        break;
        
        case UserSettingsScreen3:
            lcd.setCursor(1,0);
            lcd.print("LEVEL CONTROL  ");
            lcd.setCursor(1,1);
            lcd.print("SOLENOID CONTRL   ");
        break;

        case UserSettingsScreen4:
            lcd.setCursor(1,0);
            lcd.print("SOLENOID CONTRL   ");   
            lcd.setCursor(1,1);
            lcd.print("PROBE CONTROL   ");
        break;

         case ServiceMenuScreen1:
            digitalWrite(GREEN_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);
            lcd.setCursor(1,0);
            lcd.print("PRODUCT TYPE  ");
            lcd.setCursor(1,1);
            lcd.print("SUBPRODUCT TYPE   "); 
        break;

        case ServiceMenuScreen2:
            lcd.setCursor(1,0);
            lcd.print("SUBPRODUCT TYPE  ");  
            lcd.setCursor(1,1);
            lcd.print("CALIBRATION   "); 
        break;

        case ServiceMenuScreen3:
            lcd.setCursor(1,0);
            lcd.print("CALIBRATION   ");    
            lcd.setCursor(1,1);
            lcd.print("OPERATING TIME   "); 
        break;

        case ServiceMenuScreen4:
            lcd.setCursor(1,0);
            lcd.print("OPERATING TIME   ");    
            lcd.setCursor(1,1);
            lcd.print("SEC.SAFETY TEMP   "); 
        break;

        // case ServiceMenuScreen3:
        //     lcd.setCursor(1,0);
        //     lcd.print("CALIBRATION   ");    
        //     lcd.setCursor(1,1);
        //     lcd.print("SEC.SAFETY TEMP   "); 
        // break;

        case ServiceMenuScreen5:
            lcd.setCursor(1,0);
            lcd.print("SEC.SAFETY TEMP   ");  
            lcd.setCursor(1,1);
            lcd.print("SEC.PROBE CALB "); 
        break;

        case ServiceMenuScreen6:
            lcd.setCursor(1,0);
            lcd.print("SEC.PROBE CALB   "); 
            lcd.setCursor(1,1);
            lcd.print("SEC.TIME FACTOR     "); 
        break;

        case ServiceMenuScreen7:
            lcd.setCursor(1,0);
            lcd.print("SEC.TIME FACTOR  "); 
            lcd.setCursor(1,1);
            lcd.print("FACTORY SETTINGS"); 
        break;

        case SDUServiceMenuScreen4:
            lcd.setCursor(1,0);
            lcd.print("OPERATING TIME  ");    
            lcd.setCursor(1,1);
            lcd.print("FACTORY SETTINGS"); 
        break;

        case CalibrationSettings:
            lcd.setCursor(0,0);
            lcd.print("SET CALIBRATION");
            lcd.setCursor(0,1);
            lcd.print(calibration_value,1);
            if(calibration_value<10.0){
                lcd.setCursor(3,1);
                lcd.print(" ");
                lcd.print("LITERS   ");
            }
            else
            {
                lcd.setCursor(4,1);
                lcd.print(" ");
                lcd.print("LITERS    ");

            }
        break;

        case SecondaryFillSettings:
            lcd.setCursor(0,0);
            lcd.print("SECONDARY FILL");
            lcd.setCursor(1,1);
            lcd.print("YES");
            lcd.setCursor(12,1);
            lcd.print("NO");
            break;

        case OperatingTimeSettings:
            lcd.setCursor(0,0);
            lcd.print("SET OPERATE TIME");
            lcd.setCursor(0,1);
            lcd.print(optime[optimecounter]);
            if(optime[optimecounter]<10.0){
                lcd.setCursor(1,1);
                lcd.print(" ");
                lcd.print("HOURS ");

            }
            else
            {
                lcd.setCursor(2,1);
                lcd.print(" ");
                lcd.print("HOURS");

            }
        break;

        case ProductTypeSettings:
            lcd.setCursor(0,0);
            lcd.print("PRODUCT TYPE");
            lcd.setCursor(1,1);
            lcd.print("SDU");
            lcd.setCursor(12,1);
            lcd.print("DDU");
        break;

        case SubProductTypeSettings:
            lcd.setCursor(0,0);
            lcd.print("SUBPRODUCT TYPE   ");
            lcd.setCursor(0,1);
            if(dduflag){
            lcd.print("DDU");
            lcd.print(prodtype[prodtypecounter]);
            lcd.setCursor(6,1);
            lcd.print("     ");
            variant=((prodtype[prodtypecounter])/10);

            }
            else
            {
                lcd.print("SDU");
                lcd.print(prodtype[prodtypecounter]);
                lcd.setCursor(6,1);
                lcd.print("     ");
                variant=((prodtype[prodtypecounter])/10);

            }
        break;

        case TimeFactorSettings:
            lcd.setCursor(0,0);
            lcd.print("SET TIME FACTOR");
            lcd.setCursor(0,1);
            lcd.print("DDU");
            lcd.print(prodtype[prodtypecounter]);
            lcd.setCursor(6,1);
            lcd.print("  ");
            lcd.print(sfill_time);
            lcd.print(" mins  ");
            // variant=((prodtype[prodtypecounter])/10);
        break;

        case SolenoidControlSettings:
            lcd.setCursor(0,0);
            lcd.print("SOLENOID CONTROL");
        break;

        case FlowControlSettings:
            lcd.setCursor(0,0);
            lcd.print("FLOW CONTROL");
        break;

        

        case ProcessScreen:
            digitalWrite(YELLOW_LED,LOW);
            digitalWrite(RED_LED, HIGH);
            process_object.error_check();
            if(process_flag)
            {
                // lcd.setCursor(0,0);
                // digitalWrite(RED_LED,HIGH);
                // digitalWrite(YELLOW_LED,LOW);
                if(override_alert_flag)
                {
                    Override_alert();
                }
                else
                {
                    // if(one_second_counter % 60 == 0)
                    // {
                    // Serial3.print("Heater Temp: ");
                    // Serial3.println(calib_Heater1);
                    // }

                    // Serial3.println("In_process");
                    lcd.setCursor(0,0);
                    lcd.print("PROCESS STARTED");
                
                    if ((one_second_counter - previous_time) >= time_per_step)
                    {
                        previous_time = one_second_counter;
                        if (remaining_volume > 0)
                        {
                            remaining_volume--;
                            // Decrement 0.1L
                        }
                    }
                    lcd.setCursor(0,1);
                    lcd.print("   ");
                    lcd.setCursor(3,1);
                    lcd.print(remaining_volume/10);
                    lcd.print(".");
                    lcd.print(remaining_volume%10);
                    lcd.print(" LITERS         ");
                }
                process_object.process_start();
            }
            else if(preheat_flag)
            {
                lcd.setCursor(0,0);
                // digitalWrite(YELLOW_LED,HIGH);
                // digitalWrite(RED_LED,LOW);
                lcd.print("PROCESS START  ");
                lcd.setCursor(0,1);
                lcd.print("PRE-HEATING...   ");
                process_object.boiler_preheat();
            }

            // process_object.error_check();
            // if(!error_check_flag && !temp_drop_flag)
            // {
            //     process_flag=1;
            //     process_object.process_start();
            // }
        break;

        case PrimaryFillScreen:
            lcd.setCursor(0,0);
            lcd.print("PRIMARY BOILER   ");
            lcd.setCursor(0,1);
            lcd.print("FILLING...      ");
            process_object.error_check();
        break;

        case SecondaryFillTimer:
            process_object.error_check();
            if(override_alert_flag)
            {
                Override_alert();
            }
            else
            {
                if(dryout_flag)
                {
                    lcd.setCursor(0,0);
                    digitalWrite(RED_LED,HIGH);
                    digitalWrite(YELLOW_LED,LOW);  
                    
                    lcd.print("SECONDARY BOILER");
                    lcd.setCursor(0,1);
                    lcd.print("FILLING...      ");
                    if(!pauseflag)
                    {
                        Sec_time = pre_end_time - one_second_counter;
                        // lcd.print(Sec_time);
                    }
                    if(!error_check_flag)
                    {
                    secondarytimerflag=1;
                    process_object.dryout_fill();
                    }
                }
                else
                {

                    lcd.setCursor(0,0);
                    digitalWrite(RED_LED,HIGH);
                    digitalWrite(YELLOW_LED,LOW);  
                    
                    lcd.print("SECONDARY BOILER");
                    lcd.setCursor(0,1);
                    lcd.print("FILLING...      ");
                    if(!pauseflag)
                    {
                        Sec_time = pre_end_time - one_second_counter;
                        // lcd.print(Sec_time);
                    }
                    if(!error_check_flag)
                    {
                    secondarytimerflag=1;
                    process_object.secondary_fill();
                    }

                }
            }
        break;

        case SecondaryFillCheck:
            process_object.error_check();
            lcd.setCursor(0,0);
            digitalWrite(RED_LED,HIGH);
            digitalWrite(YELLOW_LED,LOW);  
            
            lcd.print("SECONDARY BOILER");
            lcd.setCursor(0,1);
            lcd.print("FILLING CHECK      ");
            if(!error_check_flag)
            {
            secondarytimerflag=1;
            process_object.dryout_fill();
            }
        break;

        case SafteyTemperatureSettings:
            lcd.setCursor(0,0);
            lcd.print("SET SAFTEY TEMP");
            lcd.setCursor(0,1);
            lcd.print("TEMP: ");
            lcd.print(Heatersafteytemp);
            lcd.print(" ");
            lcd.print((char)223);
            lcd.print("C  ");
        break;

        case ProbeCalibrationSettings:
            
            lcd.setCursor(0,0);
            lcd.print("PROBE CALB ");
            // lcd.print(Heater_temp,1 );
            // lcd.print(" ");
            lcd.print(calib_Heater1,1);
            lcd.print(" ");
            lcd.setCursor(0,1);
            lcd.print("Error  ");
            lcd.print(temp_error,1);
            lcd.print("  ");
        break;

        case TempSensorSettings:
            lcd.setCursor(0,0);
            lcd.print("PROBE OVERRIDE");
        break;

        case LevelSensorSettings:
            lcd.setCursor(0,0);
            lcd.print("LEVEL OVERRIDE");
        break;

        case FactoryResetScreen:
            lcd.setCursor(0,0);
            lcd.print("FACTORY SETTINGS ");
            // lcd.setCursor(1,1);
            // lcd.print("YES");
            // lcd.setCursor(12,1);
            // lcd.print("NO");
        break;

        case TestingScreen:
            // lcd.setCursor(0,0);
            // lcd.print("TESTING SCREEN   ");
            // lcd.setCursor(0,1);
            // lcd.print("NO FUNCTION    ");
            process_object.parameter_test();
        break;

        case ErrorScreen:
            // Serial3.println(error_check_flag);
            if(error_check_flag)
            {
            
                digitalWrite(RED_LED,LOW);
                if(toggle)
                {
                    // if(zero_calib)                               // If error due to zero calibration
                    // {
                    //     digitalWrite(BUZZER,HIGH);
                    //     digitalWrite(YELLOW_LED,HIGH);
                    //     lcd.setCursor(0,0);
                    //     lcd.print(" CALIB MISSING!     ");
                    //     lcd.setCursor(0,1);
                    //     lcd.print("SET CALIBRATION   ");
                        
                    // }

                    if(closetap)                   // Solenoid Error
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        // lcd.print("PROCESS COMPLETE    ");
                        // lcd.setCursor(0,1);
                        lcd.print("     CLOSE      ");
                        lcd.setCursor(0,1);
                        lcd.print("   WATER TAP    ");
                        
                    }

                    if(Secodaryfill_error_flag)
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("SECONDARY BOILER ");
                        lcd.setCursor(0,1);
                        lcd.print("NOT FILLED       ");
                    }

                    if(flow_error_checkflag)      // If error due to flow issue
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("NO WATER SUPPLY  ");
                        lcd.setCursor(0,1);
                        lcd.print("CHECK WATER TAP");
                        
                        if(solenoid_stop == 0)
                        {
                            solenoid_stop=millis();
                        }
                        if(millis()- solenoid_stop >= 5000)
                        {
                            digitalWrite(SOLENOID1,HIGH);
                            if(dduflag && process_flag)
                            {
                            digitalWrite(SOLENOID2,HIGH);
                            }

                        }
                    }

                    if(waterlevel_error_flag  && !flow_error_checkflag)            // If error due to water level issue
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("  CHECK WATER ");
                        lcd.setCursor(0,1);
                        lcd.print("  LEVEL SENSOR");
                    }
                    if(Probe1_Err && !flow_error_checkflag && !waterlevel_error_flag)             // If error due to temperature probe issue
                    {
                        digitalWrite(BUZZER,HIGH);
                        digitalWrite(YELLOW_LED,HIGH);
                        lcd.setCursor(0,0);
                        lcd.print("  PROBE  ERROR    ");
                        lcd.setCursor(0,1);
                        lcd.print("CHECK TEMP PROBE    ");
                    }

                    

                }
                else
                {
                    digitalWrite(BUZZER,LOW);
                    digitalWrite(YELLOW_LED,LOW);
                    lcd.setCursor(0,0);
                    lcd.print("                 ");
                    lcd.setCursor(0,1);
                    lcd.print("                 ");
                }
            }
            else
            {
    
                lcd.clear();
                pauseflag=0;
                // primary_filling_flag=0;
                digitalWrite(BUZZER,LOW);
                digitalWrite(YELLOW_LED,LOW);
                if(secondarytimerflag)
                {
                    digitalWrite(BUZZER,LOW);
                    screen=SecondaryFillTimer;
                    process_object.Contactor1_start();
                    heater_start=1;
                }
                else if(process_flag || preheat_flag)
                {
                    digitalWrite(BUZZER,LOW);
                    screen=ProcessScreen;
                    process_object.Contactor1_start();
                    heater_start=1;
                    // if(dduflag)
                    // {
                    //     buzzerclass_object.heater_start();
                    // }
                }
                else
                {
                    digitalWrite(BUZZER,LOW);
                    screen=MainScreen;
                    mainscreenflag=1;
                    heater_start=0;
                    // Serial3.println("1");
                }
                digitalWrite(BUZZER,LOW);
            }

            if( !Secodaryfill_error_flag && !closetap  && (process_flag || secondarytimerflag || preheat_flag))           // If error is not due to zero calibration or solenoid issue and process is running or secondary timer is running → check for errors continuously
            {
            process_object.error_check();
            }
        break;

        default:

        break;
    }
    

}

lcdclass lcd_object = lcdclass();         