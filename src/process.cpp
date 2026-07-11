#include <Arduino.h>
#include "Ext_Var.h"

uint16_t mint=0;                                         // Counter for operating time in minutes
bool process_flag=0;                                      // Indicates if the main process is running
bool flow_error_checkflag=0;                             // Flag to indicate flow error condition
bool error_check_flag=0;                                 // Flag to indicate any error condition
bool waterlevel_error_flag=0;                             // Flag to indicate water level error condition
bool temp_drop_flag=0;                                    // Flag to indicate temperature drop condition during process
bool probeerrorflag=0;                                   // Flag to indicate temperature probe error condition
bool closetap=0;                                          // Flag to indicate tap is closed (used in error handling)
bool primary_filling_flag=0;                              // Flag to indicate primary boiler filling is in progress
bool preheat_flag=0;                                           // Flag to indicate boiler preheating is in progress
bool Secodaryfill_error_flag=0;
bool dryout_flag=0;                                           // Flag to indicate dryout condition during process
float sfill_collection,after_dryout_fill=0;


unsigned long pf_start=0;                                   // Timestamp for when the main process starts (used for timing and error handling)
unsigned long Noflow_start_time = 0;                       // Timestamp for when flow error condition starts (used for timing flow error handling)
unsigned long flow_start_time = 0;                       // Timestamp for when flow is detected again after a no-flow condition (used for timing and error handling)
// unsigned long override_msg_start=0;
bool noflow_timing = false;                                  // Flag to indicate flow error timing is active
bool flow_timing = false;
// bool pauseflag=0;
bool level_detect=true;

// int Heatersafteytemp=30;
int resume_value=0;                                         // Used to store the timer value when process is paused during secondary boiler filling (for resuming later)
int refill_tym=0;                                           // Used to track the time spent on refilling the secondary boiler (for timing and error handling)
int pre_end_time=0;                                         // Time before the main process end time when secondary boiler filling should start (calculated based on variant and calibration)
int end_time=0;                                              // Total time for the main process to complete (calculated based on variant and calibration)
int variant=0;                                              // Product variant selected by the user (used for timing and calibration calculations)
int remaining_volume=0;                                     // Used to track the remaining volume to be dispensed during the process (for timing and display purposes)
int u_input=0;                                              // User input for the desired volume to be dispensed (in liters, multiplied by 10 for easier calculations)
float time_per_step=0;                                      // Time in seconds required to dispense 0.1 liters (calculated based on end_time and u_input)
int previous_time=0;                                        // Used to track the last time the remaining volume was updated (for timing the dispensing steps)
int temp_error_count=0;
int screen2=0;
int dryout_fill_time,dryout_calculation=0;
bool heater_start=false;   // Counter for consecutive temperature error readings (used for error handling)              

#define FLOW_SWITCH 24
// Solenoid Pins
#define SOLENOID1 13
#define SOLENOID2 12

//Contactor and Heater Pins
#define HEATER1 15
#define HEATER2 14
#define CONTACTOR1 10
#define CONTACTOR2 11

// Water level detection pins
#define POWER_DETECTION 5
#define WATER_LEVEL_SENSOR 4

#define STARTING_DELAY 2000     // Delay in milliseconds before starting/stopping heaters and contactors to ensure proper operation and avoid electrical issues

#define PREHEAT_TIME 240         // Time in seconds for preheating the boiler before starting the main process
// #define PREHEAT_TIME 10
#define SECONDARY_CHECK_TIME 300   // Secondary boiler filling check time in seconds
#define OVERRIDE_ALERT_INTERVAL 600  // Time in seconds after which override alert should be triggered during the process if override settings are enabled
// #define OVERRIDE_ALERT_INTERVAL 30

// void heat1_start();
void contact2_start();
// void heat2_start();

void contact1_stop();
// void heat2_stop();
void contact2_stop();
void Change_flag();


// Tickers for managing delayed start and stop of heaters and contactors during the process
// Ticker T_heater1_start(heat1_start,STARTING_DELAY,1,MILLIS);
Ticker T_contactor2_start(contact2_start,(STARTING_DELAY),1,MILLIS);
// Ticker T_heater2_start(heat2_start,(STARTING_DELAY),1,MILLIS);

Ticker T_contactor1_stop(contact1_stop,STARTING_DELAY,1,MILLIS);
// Ticker T_heater2_stop(heat2_stop,(STARTING_DELAY),1,MILLIS);
Ticker T_contactor2_stop(contact2_stop,(STARTING_DELAY),1,MILLIS);

Ticker flag_warning(Change_flag,5000,0,MILLIS);



void Change_flag()
{
    check_flag=1;
}

void process:: Check_ticker_stop()
{
    flag_warning.stop();
   


} 


void contact2_start()
{
    if((process_flag || secondarytimerflag) && !error_check_flag )
        process_object.Contactor2_start();
}
//--------------------------------------------------------------------------

void contact1_stop()
{
    process_object.Contactor1_stop();
}

// void heat2_stop()
// {
//     process_object.heater2_stop();
// }

void contact2_stop()
{
    process_object.Contactor2_stop();
}

// #define POWER_DETECTION2 7

// Definations


process::process()
{}
//
void process:: process_setup()           // Initialising the Heaters,Solenoid,Contactors and Water level sensor
{
    pinMode(FLOW_SWITCH,INPUT_PULLUP);
    pinMode(SOLENOID1,OUTPUT);
    pinMode(SOLENOID2,OUTPUT); 
    pinMode(HEATER1,OUTPUT);
    pinMode(HEATER2,OUTPUT);
    pinMode(CONTACTOR1,OUTPUT);
    pinMode(CONTACTOR2,OUTPUT);
    pinMode(POWER_DETECTION,OUTPUT);
    pinMode(WATER_LEVEL_SENSOR,INPUT_PULLUP);
    // pinMode(POWER_DETECTION2,OUTPUT);
    digitalWrite(POWER_DETECTION,HIGH);
    // digitalWrite(POWER_DETECTION2,HIGH);

}

void process:: ticker_update(){
    // T_heater1_start.update();
    // T_heater2_start.update();
    T_contactor2_start.update();

    T_contactor1_stop.update();
    // T_heater2_stop.update();
    T_contactor2_stop.update();
    flag_warning.update();

}  

void process:: variant_settings()      // Calculations for Process time and Secondary Fill time for different variant
{
    variant=((prodtype[prodtypecounter])/10);
    int final_calb=process_object.calculate_calibration();

    switch(prodtype[prodtypecounter])
    {
        case 150:                        // For 1.5 Liters
            // pre_end_time = 15;
            // pre_end_time = 10*240;  //40 minutes (to pre-heat/fill the secondary boiler before starting normal operation.)
            sfill_collection = 1.2;     // in liters
            after_dryout_fill= 0.8;            // in liters


            dryout_fill_time= process_object.calculate_dryout_time()*60; 
            pre_end_time = sfill_time*60;
            
            
            end_time = (final_calb*4)*60;//---------------end_time in seconds
            // end_time =15;
        break;

        case 250:                        // For 2.5 Liters
            // pre_end_time = 25;
            // pre_end_time = (11*144)+60;  //26.4 minutes

            sfill_collection = 1.3;     // in liters
            after_dryout_fill= 0.8;            // in liters


            dryout_fill_time= process_object.calculate_dryout_time()*60;
            pre_end_time = sfill_time*60;
            end_time = ((final_calb*2.4)*60) + 60;//----end_time in seconds
            // end_time = 25;
        break;

        case 400:                          // FOr 4.0 Liters
        //  pre_end_time = 40;
        // pre_end_time = 13*90;   //19.5 minutes
        sfill_collection = 1.4;     // in liters
        after_dryout_fill= 1.1;            // in liters


        dryout_fill_time= process_object.calculate_dryout_time()*60;

            pre_end_time = sfill_time*60;
            end_time = (final_calb*1.5)*60 ;//-----------end_time in seconds
            // end_time = 40;
        break;

    }

    remaining_volume = u_input;

    // Calculate seconds required for 0.1L
    time_per_step = end_time / u_input;
    // Protection against division result 0
    if (time_per_step == 0)
        time_per_step = 1;

    previous_time = one_second_counter;

}

int process:: calculate_dryout_time()                      // Calculation of dryout time based on the amount of water collected in secondary boiler and the time taken to collect that water (used for timing the dryout fill step during the process)
{
    dryout_calculation = ((sfill_time*after_dryout_fill)/sfill_collection);
    Serial3.print("Dryout Time Calculation: ");
    Serial3.println(dryout_calculation);
    return dryout_calculation;
}

int process:: calculate_calibration()                    // Calibration for the product 
{
    u_input=counter*10;
   int actual_time = (variant * 60) / (calibration_value*10);
   int Calibrated_volume = (variant * actual_time) / 60;
   int Calb_vol_act = round((u_input * Calibrated_volume) / variant);
   return(Calb_vol_act); 
}

void process:: error_check()                  // Checks the Error,1)Flow Switch 2)Water Detection 3)Probe Error
{
    static int lastState = -1;
    static unsigned long stateStartTime = 0;

    if (!flowoverride)
    {
        int currentState = digitalRead(FLOW_SWITCH);

            // Detect state change
        if (currentState != lastState)
        {
            lastState = currentState;
            stateStartTime = millis();  // reset timer on every change
        }
        if (currentState == HIGH)   // NO FLOW
        {
            if (millis() - stateStartTime >= 15000)
            {
                if (!flow_error_checkflag)
                {
                    solenoid_stop = 0;
                    digitalWrite(SOLENOID1, LOW);
                    digitalWrite(SOLENOID2, LOW);
                }
                flow_error_checkflag = 1;
            }
        }
        else   // FLOW present (LOW)
        {
            if (millis() - stateStartTime >= 10000)
            {
                flow_error_checkflag = 0;
            }
        }
    }
    else
    {
        flow_error_checkflag = 0;
    }

    if(flow_error_checkflag)
    {
        // error_check_flag=1;
        pauseflag=1;

        screen=ErrorScreen;
        if(!error_check_flag)
        {
            primary_filling_flag=0;                       //**************************************************** */
            error_check_flag=1;
        // process_object.process_stop();
        heater_start=0;
        // process_object.heater1_stop();
        process_object.Contactor1_stop();
        lcd_object.lcd_buzzer_toggle_start();
        
        // Error_buzzer_toggle.start();
        // buzzerclass_object.heater_stop();
        }
        return;
    }

    else
    {
        // digitalWrite(BUZZER,LOW); 
        // pauseflag=0;
        error_check_flag=0;
    }
    
    if(!leveloverride)
    {
        process_object.water_level_detection();
    }
    else
    {
        waterlevel_error_flag=0;
    }

    if(waterlevel_error_flag)
    {
        // error_check_flag=1;
        pauseflag=1;

        screen=ErrorScreen;
        if(!error_check_flag)
        {
            error_check_flag=1;
            // process_object.process_stop();
            heater_start=0;
            // process_object.heater1_stop();
            process_object.Contactor1_stop();
            // lcd_object.lcd_buzzer_toggle_start();
            // Serial3.println("BUZZER TOGGLE START");
            // buzzerclass_object.heater_stop();
        }
        return;
    }
    else
    {
        // digitalWrite(BUZZER,LOW); 
        // pauseflag=0;
        error_check_flag=0;
    }

    if(!probeoverride)
    {
        if(dduflag)                                 //  Probe Error should be active only if DDU flag is active
        {
            if(!probeoverride)
            {
                PT100_object.PT100_error_check();
                if(Probe1_Err)
                {
                    heater_start=0;
                    error_check_flag=1;
                    pauseflag=1;
                    temp_error_count=0;

                    screen=ErrorScreen;
                    if(!probeerrorflag)
                    {
                        // Serial3.println("hi");
                        error_check_flag=1;
                        probeerrorflag=1;
                        // heater_start=0;
                        // process_object.heater1_stop();
                        // process_object.Contactor1_stop();
                        // buzzerclass_object.heater_stop();
                       lcd_object.lcd_buzzer_toggle_start();
                       process_object.Contactor1_stop();
                    }
                    return;
                }
                else
                {
                    // digitalWrite(BUZZER,LOW);
                    error_check_flag=0;
                    // pauseflag=0;
                    probeerrorflag=0;   
                }
            }
            // PT100_object.read_temperature();
            // PT100_object.PT100_error_check();
            // Serial3.println("HI");
            if(process_flag)                            
            {
                if(calib_Heater1 > Heatersafteytemp)       // Heatersafteytemp If Secondary Boiler gets empty
                {
                    temp_error_count++;
                    // Serial3.print("Temperature Error Count: ");
                    // Serial3.println(temp_error_count);
                    if(temp_error_count>=100)
                    {
                        process_flag=0;
                        heater_start=0;
                        // process_object.heater1_stop();
                        // process_object.heater1_start();
                        // buzzerclass_object.heater_stop();
                        resume_value=one_second_counter-5; 
                        // error_check_flag=1;
                        // pauseflag=1;
                        lcd.clear();
                        screen=SecondaryFillTimer;
                        time_skip=0;
                        dryout_flag=1;
                        temp_drop_flag=1;               //////////////////
                        one_second_counter=0;
                        secondarytimerflag=1;
                        temp_error_count=0; 
                        // process_object.Contactor1_start();
                        // heater_start=0;
                        process_object.Contactor2_stop();
                        // process_object.heater2_stop();
                        // process_object.heater1_start();
                        // buzzerclass_object.heater_stop();
                    }
                    
                }
            }
            
        }
    }

}

void process::water_level_detection()       // Water level Detection Fuction, It is closed for 6 sec and open for 1 sec
{
    static unsigned long lvl_start=0;

    uint8_t cycle_position = one_second_counter2 % 15;

    if (cycle_position >= 10)   // From second 10 to 14 (5 seconds ON)
    {
        digitalWrite(POWER_DETECTION, LOW);  // Sensor ON
        delay(100);

        if (digitalRead(WATER_LEVEL_SENSOR) == HIGH && !primary_filling_flag)  // Active low sensor
        {
            // error_check_flag=1;
            pf_start=millis();
            primary_filling_flag=1;
            lcd.clear();
            screen=PrimaryFillScreen;
            pauseflag=1;
            // process_flag=0;
            heater_start=0;
            process_object.Contactor1_stop();
            
        }

        else if(digitalRead(WATER_LEVEL_SENSOR) == LOW && primary_filling_flag)
        {
            // bool level_detect;
            if(level_detect)
            {
                lvl_start=millis();
                level_detect=false;
            }

            if(millis()-lvl_start>=3000)                  // Added this logic to avoid false triggering of water level error due to water surface turbulence or bubbles during filling which can cause the sensor to read high momentarily even when the water level is sufficient.
            { 
                digitalWrite(BUZZER,LOW);   // To stop the buzzer if it was triggered due to water level error during filling
                primary_filling_flag=0;
                waterlevel_error_flag = 0;
                pauseflag=0;
                level_detect=true;
                // error_check_flag=0;

                lcd.clear();
                heater_start=1;
                process_object.Contactor1_start();
                if(secondarytimerflag)
                {
                    screen=SecondaryFillTimer;
                    // process_object.heater1_start();
                }
                else
                {
                    screen=ProcessScreen;
                    // process_flag=1;
                }
            }
        }
        else if(digitalRead(WATER_LEVEL_SENSOR) == HIGH && primary_filling_flag)
        {
            if(millis()-pf_start>=180000)   // If water level is low for more than 3 minutes
            {
                level_detect=true;
                if(!waterlevel_error_flag)
                {
                    lcd_object.lcd_buzzer_toggle_start();
                    Serial3.println("BUZZER TOGGLE START"); 
                }
                waterlevel_error_flag=1;
                // error_check_flag=0;
                primary_filling_flag=0;
                // Serial3.println("1");
            }
        }
        else
        {
            waterlevel_error_flag = 0;
            // Serial3.println("0");
        }
    }
    else   // First 10 seconds OFF
    {
        digitalWrite(POWER_DETECTION, HIGH);  // Sensor OFF
    }
}

void process:: dryout_fill()                                                      // Secondary boiler dryout fill function, It is called when the temperature drop condition is detected during the process which indicates that the secondary boiler might have run dry and needs to be refilled before resuming the process. This function manages the timing and error handling for refilling the secondary boiler in this scenario.
{
    if(dryout_flag && !error_check_flag && !pauseflag)
    {
        if(one_second_counter >= dryout_fill_time)
        {
            if(one_second_counter<=(dryout_fill_time + SECONDARY_CHECK_TIME) && !probeoverride)
            {
                heater_start=1;
                process_object.Contactor2_start();
                screen=SecondaryFillCheck;

                if(calib_Heater1 > Heatersafteytemp && calib_Heater1 < MAX_SAFETY_TEMP )
                {
                    Secodaryfill_error_flag=1;
                    error_check_flag=1;
                    secondarytimerflag=0;
                    one_second_counter=0;
                    time_skip=0;
                    dryout_flag=0;
                    heater_start=0;
                    lcd_object.lcd_buzzer_toggle_start();
                    screen=ErrorScreen;
                    process_object.Contactor1_stop();
                    digitalWrite(SOLENOID1, LOW);  // Close solenoid 1
                    digitalWrite(SOLENOID2, LOW);  // Close solenoid 2
                }
            }
            else
            {
                lcd.clear();
                dryout_flag=0;
                secondarytimerflag=0;
                one_second_counter=0;
                time_skip=0;
                lcd.setCursor(0,0);
                lcd.print("SECONDARY BOILER");
                lcd.setCursor(0,1);
                lcd.print("FILLING COMPLETED");
                digitalWrite(BUZZER,HIGH);
                delay(1000);
                buzzerclass_object.Buzzer_beep(1000);
                buzzerclass_object.Buzzer_start();
                // if(temp_drop_flag)                          //  If  fuction is called between the process because of secondary boiler empty
                // {
                process_flag=1;
                temp_drop_flag=0;
                screen=ProcessScreen;
                heater_start=1;
                process_object.Contactor1_start();
                one_second_counter=resume_value;
                previous_time = one_second_counter;
                lcd.clear();
                
            // }

            }
            // Dryout fill logic here
        }
    }
}


void process:: secondary_fill()           // This functions runs the Secondary boiler fill logic
{
    if(secondarytimerflag && !error_check_flag && !pauseflag)
    {
        if(one_second_counter% OVERRIDE_ALERT_INTERVAL==0 && one_second_counter!=0 && !override_alert_flag)
        {
            override_alert_flag=1;
            flag_warning.start();
            // unsigned long override_msg_start=millis();
        }
        // if(one_second_counter>=10)
        if(one_second_counter>=pre_end_time)
        {
            if(probeoverride)
            {
                screen=OverrideScreen;              // Go to override screen if any override is active
                overridescreenflag=1;
                secondarytimerflag=0;
                probecheckscreen=1;   // Flag to indicate that the probe check screen is currently active
            return;
            }
            else
            {
                screen=ProcessScreen;
                lcd.clear();
                process_flag=1;
                secondarytimerflag=0;
                mainscreenflag=0;
                one_second_counter=0;
                pauseflag=0;
                error_check_flag=0;
                flow_error_checkflag=0;
                heater_start=1;
                process_object.Contactor2_start();
            }
        }
    }

}

void process:: boiler_preheat()                          // Preheating logic for the boiler before starting the main process.
{
    if(preheat_flag && !error_check_flag)
    {
        if(one_second_counter >= PREHEAT_TIME)
        {
            preheat_flag=0;
            one_second_counter=0;
            process_flag=1;
            screen=ProcessScreen;
            heater_start=1;

            process_object.Contactor1_start();
            
        }
    }
}
void process:: process_start()                    // Process timing funcution
{
    if(process_flag && !error_check_flag)
    {
    // end_time=20; //for testing purpose only. Comment this line during actual operation and uncomment the line below.
    // if(one_second_counter % 60 == 0 )
    // {
    //     Serial3.print("Heater Temp: ");
    //     Serial3.println(calib_Heater1);

    // }
    if(one_second_counter% OVERRIDE_ALERT_INTERVAL==0 && one_second_counter!=0 && !override_alert_flag)
    {
        override_alert_flag=1;
        flag_warning.start();
        // unsigned long override_msg_start=millis();
    }

    // if(override_alert_flag)
    // {
    //     if(millis()-override_msg_start>=5000)
    //     {
    //         override_alert_flag=0;
    //     }

    // }
    if(one_second_counter>= end_time)
    {
        digitalWrite(BUZZER,HIGH);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("PROCESS COMPLETED");
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
        
        delay(3000);
        buzzerclass_object.Buzzer_beep(1000);
        buzzerclass_object.Buzzer_start();
        one_second_counter=0;
        process_flag=0;
        heater_start=0;
        secondaryyes=0;
        EEPROM.update(SECONDARY_FILL, secondaryyes);
        process_object.Contactor1_stop();
        // process_object.heater1_stop();
        // buzzerclass_object.heater_stop();
        process_object.Solenoid1_stop();
        process_object.Solenoid2_stop();

        if(solenoidoverride)                              
        {
            screen=MainScreen;
            mainscreenflag=1;
        }
        else
        {
            lcd_object.lcd_buzzer_toggle_start();
            error_check_flag=1;
            Probe1_Err = 0;
            waterlevel_error_flag = 0;
            flow_error_checkflag = 0;
            closetap=1;
            delay(100);

            screen=ErrorScreen;

            return;
        }      
    }
    } 


}

void process:: parameter_test()                // Sensor Testing menu
{
    switch(screen2)
    {
        case TestMenu1:
            lcd.setCursor(1,0);
            lcd.print("FLOW SENSOR      ");
            lcd.setCursor(1,1);
            lcd.print("LEVEL SENSOR      ");
            // lcd.print(calibration_value);
        break;

        case TestMenu2:
            lcd.setCursor(1,0);
            lcd.print("LEVEL SENSOR      ");
            lcd.setCursor(1,1);
            lcd.print("PT 100 PROBE         ");
        break;

        case FlowSensorTest:
            lcd.setCursor(0,0);
            lcd.print("FLOW SENSOR:");
            if(digitalRead(FLOW_SWITCH)==HIGH)
            {
                lcd.setCursor(0,1);
                lcd.print("NO FLOW       ");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("FLOW DETECTED ");
            }
        break;

        case LevelSensorTest:
            lcd.setCursor(0,0);
            lcd.print("LEVEL SENSOR:");
            if(digitalRead(WATER_LEVEL_SENSOR)==HIGH)
            {
                lcd.setCursor(0,1);
                lcd.print("BOILER EMPTY     ");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("BOILER FILLED     ");
            }
        break;

        case PT100ProbeTest:
            lcd.setCursor(0,0);
            lcd.print("PT100 PROBE:");
            lcd.setCursor(0,1);
            if (calib_Heater1 > PROBE_ERROR_TEMP)   // Open / Short condition
            {
                lcd.print("PROBE ERROR     ");
            }
            else
            {
                lcd.print(calib_Heater1);
                lcd.print((char)223);
                lcd.print("C       ");
            }
        break;

        // case TestMenu3:
        //     lcd.setCursor(0,0);
        //     lcd.print("HEATER TEMP:");
        //     lcd.setCursor(0,1);
        //     lcd.print(calib_Heater1);
        // break;
    }
}




// void process:: heater1_start()
// {
//     // digitalWrite(SOLENOID1,HIGH);
//     // digitalWrite(CONTACTOR1,HIGH);
//     // if(!primary_filling_flag  && !error_check_flag)
//     if(heater_start || temp_drop_flag)
//     {
//         digitalWrite(HEATER1,HIGH);
//         if(dduflag && process_flag  )
//         {
//             T_contactor2_start.start();
//         }
//     }
// }
    
// void process:: heater1_stop()
// {
//     // digitalWrite(SOLENOID1,LOW);
//     // digitalWrite(CONTACTOR1,LOW);
//     digitalWrite(HEATER1,LOW);
//     T_contactor1_stop.start();
    
// }

// void process:: heater2_start()
// {
//      // solenoid2 on
//     // digitalWrite(SOLENOID2,HIGH);
//     // contactor2 on
//     // digitalWrite(CONTACTOR2,HIGH);
//     // heater2 on
//     digitalWrite(HEATER2,HIGH);
// }

// void process:: heater2_stop()
// {
//     // heater2 off
//     if(!heater_start){
//         digitalWrite(HEATER2,LOW);
//         T_contactor2_stop.start();
//     }
// }

void process:: process_stop()
{
    digitalWrite(SOLENOID1,LOW);
    digitalWrite(CONTACTOR1,LOW);
    digitalWrite(HEATER1,LOW);
    digitalWrite(SOLENOID2,LOW);
    digitalWrite(CONTACTOR2,LOW);
    digitalWrite(HEATER2,LOW);
}

void process:: Contactor1_start()
{
    digitalWrite(SOLENOID1,HIGH);
    delay(50);
    digitalWrite(CONTACTOR1,HIGH);

    // if(heater_start || temp_drop_flag)
    // {
        // digitalWrite(HEATER1,HIGH);
        if(dduflag && process_flag  )
        {
            T_contactor2_start.start();
        }
    // }
    // T_heater1_start.start();
}

void process:: Contactor2_start()
{
    if(heater_start)
    {
        digitalWrite(SOLENOID2,HIGH);
        delay(50);
        digitalWrite(CONTACTOR2,HIGH);

        // T_heater2_start.start();
    }
}

void process:: Contactor1_stop()
{
    
        // if(!process_flag || error_check_flag)
        if(!heater_start)
        {
            digitalWrite(CONTACTOR1,LOW);
            if(dduflag && !heater_start)
            {
                // T_heater2_stop.start();
                T_contactor2_stop.start();
            }
        }
    
}

void process:: Contactor2_stop()
{
    digitalWrite(CONTACTOR2,LOW);
}

void process:: Solenoid1_start()
{
    digitalWrite(SOLENOID1,HIGH);
}

void process:: Solenoid2_start()
{
    digitalWrite(SOLENOID2,HIGH);
}
 
void process:: Solenoid1_stop()
{
    digitalWrite(SOLENOID1,LOW);
}

void process:: Solenoid2_stop()
{
    digitalWrite(SOLENOID2,LOW);
}

process process_object=process();