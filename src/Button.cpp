#include <Arduino.h>
#include "Ext_Var.h"

// ---------------- SAFETY LIMIT DEFINITIONS ----------------
#define MAX_SAFETY_TEMP 150          // Maximum allowed heater safety temperature
#define MIN_SAFETY_TEMP 50           // Minimum allowed heater safety temperature
#define SAFETY_TEMP_STEP 1           // Step increment/decrement for safety temperature
#define PROBE_CALIBRATION_LIMIT 20   // Max +/- limit for probe calibration error

// ---------------- LED PIN DEFINITIONS ----------------
#define RED_LED 17       //Heater ON indication LED
#define YELLOW_LED 39    // Standby / Warning indication LED
#define GREEN_LED 38     // Settings indication LED 

// ---------------- PROCESS CONFIGURATION ----------------
#define OPERATING_TIME 4   // Base operating time factor used for max volume calculation

// ---------------- GLOBAL VARIABLES ----------------

// Volume control
float counter = 0.0;            // User-selected output volume (in liters)
float Max_liter = 0.0;          // Maximum allowable volume based on variant

// Calibration & tuning
float calibration_value = 3.0;  // Calibration offset for system
float temp_error = 0.0;         // Probe temperature error adjustment
float speed = 0.5;              // Reserved / adjustable speed parameter
// bool zero_calib = 0;             // Flag to indicate zero calibration condition

// Timing
long start_tt = 0;              // Used for detecting long press (UP + DOWN combo)

// ---------------- BUTTON & UI STATE FLAGS ----------------
bool dualPressActive = 0;                  // Helper flag for dual button press detection

bool mainscreenflag = 0;        // Indicates Main Screen is active
bool usersettings = 0;          // Indicates User Settings menu is active
bool servicemenu = 0;           // Indicates Service Menu is active
bool inmenu = 0;                // Indicates user is inside a submenu

// Pointer control for menu navigation
bool uppointer = 0;             // Pointer on upper option
bool downpointer = 0;           // Pointer on lower option

// ---------------- FEATURE FLAGS ----------------
bool secondaryyes = 1;          // Secondary fill option enable/disable
bool solenoidoverride = 1;      // Solenoid override control
bool flowoverride = 0;          // Flow sensor override
bool leveloverride = 0;         // Level sensor override
bool probeoverride = 0;         // Temperature probe override

bool speedup = 0;               // Reserved for speed control feature
bool secondarytimerflag = 0;    // Indicates secondary fill timer is running
bool time_skip = 0;             // Used to skip time during secondary process

// ---------------- FACTORY RESET ----------------
bool factoryresetflag = 1;      // Confirmation flag for factory reset
bool testsettings=0;
bool intestmenu=0;
bool override_alert_flag=0;        // Flag to indicate alert condition due to override settings
bool overridescreenflag=0;        // Flag to indicate that the override screen is currently active
bool flowcheckscreen=0;        // Flag to indicate that the flow check screen is currently active
bool levelcheckscreen=0;      // Flag to indicate that the level check screen is currently active
bool probecheckscreen=0;      // Flag to indicate that the probe check screen is currently active

// ---------------- COUNTERS ----------------
uint8_t skip_count = 0;         // Counter used for time skip logic
uint8_t longpress_count = 0;    // Counter for smooth long press increment/decrement

// ---------------- SAFETY PARAMETERS ----------------
int Heatersafteytemp = 80;     // Current heater safety temperature setting

// ---------------- BUTTON PIN DEFINITIONS ----------------
#define UP 35
#define DOWN 32
#define BACK 26
#define MODE 36

// ---------------- BUTTON OBJECTS ----------------
// Using OneButton library for handling click & long press events
OneButton up_button(UP, true);
OneButton down_button(DOWN, true);
OneButton back_button(BACK, true);
OneButton mode_button(MODE, true);

// ---------------- CLASS CONSTRUCTOR ----------------
buttonClass::buttonClass()
{
    // Constructor currently does nothing
}

// ---------------- BUTTON INITIALIZATION ----------------
void buttonClass::button_setup()
{
    // Configure button pins with internal pull-up resistors
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(BACK, INPUT_PULLUP);
    pinMode(MODE, INPUT_PULLUP);
    
    // -------- BUTTON EVENT ATTACHMENTS --------

    // UP button
    up_button.attachClick(increment);                 // Short press → Increment value / move up
    up_button.attachDuringLongPress(long_press_up);   // Long press → Continuous increment

    // DOWN button
    down_button.attachClick(decrement);               // Short press → Decrement value / move down
    down_button.attachDuringLongPress(long_press_down);// Long press → Continuous decrement

    // MODE button
    mode_button.attachDuringLongPress(user_settings); // Long press → Enter User Settings
    mode_button.attachClick(enter_function);          // Short press → Enter / Confirm action

    // BACK button
    back_button.attachClick(back_screen);             // Short press → Go back one screen
    back_button.setPressTicks(1000);                     // Long press threshold (1 second)
    back_button.attachDuringLongPress(back_to_home);  // Long press → Return to main screen

    // Note:
    // BACK long press acts as a "hard reset" to main UI state
}

// ---------------- BUTTON EVENT PROCESSING ----------------
// This function must be called repeatedly inside loop()
// It updates the state of all buttons (click / long press detection)
void buttonClass::button_ticks()
{
    up_button.tick();     // Process UP button events
    down_button.tick();   // Process DOWN button events
    mode_button.tick();   // Process MODE button events
    back_button.tick();   // Process BACK button events
}

// ---------------- POINTER DISPLAY FUNCTION ----------------
// Displays '>' symbol at given LCD position
void buttonClass::setPointer(uint8_t col, uint8_t row)
{
    lcd.clear();              // Clear entire LCD 
    lcd.setCursor(col, row);  // Move cursor to specified position
    lcd.print(">");           // Print selection pointer
}

// ---------------- DUAL BUTTON LONG PRESS (UP + DOWN) ----------------
// Detects simultaneous press of UP and DOWN buttons
// If held for defined duration → enters Service Menu
void buttonClass::but_check()
{
    // -------- STEP 1: Detect initial simultaneous press --------
    if (digitalRead(UP) == LOW && digitalRead(DOWN) == LOW && dualPressActive == 0)
    {
        start_tt = millis();  // Record start time of press
        dualPressActive = 1;             // Mark that dual press has started
    }

    // -------- STEP 2: Check if buttons are still held --------
    else if (digitalRead(UP) == LOW && digitalRead(DOWN) == LOW && dualPressActive == 1)
    {
        // Check if hold duration exceeded threshold
        if ((millis() - start_tt) >= 1000)   // 1000 ms = 1 second
        {
            // -------- ENTER SERVICE MENU --------
            lcd.clear();

            screen = ServiceMenuScreen1; // Switch to first service menu screen
            servicemenu = 1;             // Activate service menu mode
            mainscreenflag = 0;          // Exit main screen

            // Set pointer to first option
            uppointer = 1;
            downpointer = 0;
            setPointer(0, 0);

            // Provide user feedback via buzzer
            digitalWrite(BUZZER, HIGH);
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();

            return; // Exit after triggering action
        }
    }

    // -------- STEP 3: Reset if buttons released --------
    else if (dualPressActive == 1)
    {
        // If either button is released → cancel detection
        if (digitalRead(UP) == HIGH || digitalRead(DOWN) == HIGH)
        {
            dualPressActive = 0; // Reset dual press flag
        }
    }
}

// ---------------- BACK TO HOME (LONG PRESS BACK BUTTON) ----------------
// This function performs a "hard reset" of the UI and process state.
// It stops all operations, resets flags, and returns to Main Screen.
// Also handles error condition routing based on override settings.

void buttonClass::back_to_home()
{
    // Execute only if NOT already on main screen AND tap is not closed
    if (!mainscreenflag && !closetap)
    {
        digitalWrite(BUZZER, HIGH);
        if(!usersettings && !servicemenu && !inmenu && !intestmenu && !testsettings)
        {
            lcd.clear();  // Clear display
            lcd.setCursor(0,0);
            lcd.print("    PROCESS ");
            lcd.setCursor(0,1);
            lcd.print("   TERMINATED");
            delay(2000); // Short delay to allow message to be read
            buzzerclass_object.Buzzer_beep(100);
            buzzerclass_object.Buzzer_start();
        }
        else
        {
            buzzerclass_object.Buzzer_beep(1000);
            buzzerclass_object.Buzzer_start();
        }
        // -------- RESET SYSTEM STATES --------
        mainscreenflag = 1;      // Return to main screen
        flow_error_checkflag = 0;
        temp_drop_flag = 0;      // Reset temperature drop logic
        inmenu = 0;              // Exit any submenu
        heater_start = 0;        // Reset heater start flag
        intestmenu=0;          // Exit test menu if active
        testsettings=0;          // Reset test settings flag
        waterlevel_error_flag=0;   // Reset water level error flag
        flow_error_checkflag=0;    // Reset flow error flag
        Probe1_Err=0;               // Reset probe error flag
        primary_filling_flag=0;   // Reset primary filling flag
        pauseflag=1;              // Stop process Timer
        process_flag = 0;        // Stop process execution
        preheat_flag = 0;        // Reset preheat flag
        secondarytimerflag = 0;  // Stop secondary timer
        error_check_flag=0;      // Reset error check flag
        time_skip=0;            // Reset time skip logic

        

        // -------- STOP HARDWARE OPERATIONS --------
        process_object.Contactor1_stop();
        digitalWrite(SOLENOID1, LOW);  // Close solenoid 1
        digitalWrite(SOLENOID2, LOW);  // Close solenoid 2
        digitalWrite(POWER_DETECTION, LOW);  // Sensor OFF

        // -------- RESTORE SETTINGS FROM EEPROM --------
        eeprom_object.eeprom_dataread();

        // Default screen → Main Screen
        screen = MainScreen;

        

        // -------- ERROR HANDLING LOGIC --------
        // If solenoid override is OFF and not in settings/menu,
        // force system into error screen for safety validation
        if (!solenoidoverride && !usersettings && !servicemenu)
        {
            Probe1_Err = 0;
            waterlevel_error_flag = 0;
            flow_error_checkflag = 0;

            closetap = 1;          // Mark tap as closed
            error_check_flag = 1;  // Trigger error check

            screen = ErrorScreen;  // Redirect to error screen
            lcd_object.lcd_buzzer_toggle_start();
        }
        else
        {
            // Normal return to main screen
            usersettings = 0;
            servicemenu = 0;
            screen = MainScreen;
        }
        lcd.clear();
    }
}

// ---------------- LONG PRESS UP BUTTON ----------------
// Handles continuous increment when UP button is held
// Uses longpress_count to control increment speed (non-blocking)

void buttonClass::long_press_up()
{
    // Prevent conflict: only act if DOWN button is NOT pressed
    if (digitalRead(DOWN) == HIGH)
    {
        // -------- MAIN SCREEN: INCREASE VOLUME --------
        if (mainscreenflag)
        {
            // Calculate maximum allowed volume
            Max_liter=(optime[optimecounter])*(variant/10.0);

            if (counter < Max_liter)
            {
                longpress_count++;  // Increment counter for smooth stepping

                // Increase value every 4 cycles
                if (longpress_count == 4)
                {
                    counter += 0.5;
                    longpress_count = 0;  // Reset counter
                }
            }
        }

        // -------- CALIBRATION SETTINGS --------
        if (screen == CalibrationSettings)
        {
            if (calibration_value < Max_liter +5)
            {
                longpress_count++;

                // Faster increment (every 3 cycles)
                if (longpress_count == 3)
                {
                    calibration_value += 0.1;
                    longpress_count = 0;
                }
            }
        }

        // -------- SAFETY TEMPERATURE SETTINGS --------
        if (screen == SafteyTemperatureSettings)
        {
            if (Heatersafteytemp < MAX_SAFETY_TEMP)
            {
                longpress_count++;

                // Slower increment (every 5 cycles for safety)
                if (longpress_count == 5)
                {
                    Heatersafteytemp += SAFETY_TEMP_STEP;
                    longpress_count = 0;
                }
            }
        }
        // -------- TIME FACTOR SETTINGS --------
        if (screen == TimeFactorSettings)
        {
            if (sfill_time < SECONDARY_FILL_TIME_LIMIT)
            {
                longpress_count++;

                // Controlled increment (every 5 cycles)
                if (longpress_count == 5)
                {
                    sfill_time += 1;
                    longpress_count = 0;
                }
            }
        }

        // -------- PROBE CALIBRATION SETTINGS --------
        if (screen == ProbeCalibrationSettings)
        {
            if (temp_error <= PROBE_CALIBRATION_LIMIT)
            {
                longpress_count++;

                // Controlled increment (every 5 cycles)
                if (longpress_count == 5)
                {
                    temp_error += 0.1;
                    longpress_count = 0;
                }
            }
        }
    }
}

// ---------------- LONG PRESS DOWN BUTTON ----------------
// Handles continuous decrement when DOWN button is held
// Mirror logic of long_press_up()

void buttonClass::long_press_down()
{
    // Prevent conflict: only act if UP button is NOT pressed
    if (digitalRead(UP) == HIGH)
    {
        // -------- MAIN SCREEN: DECREASE VOLUME --------
        if (counter >= 0.5)
        {
            if (mainscreenflag)
            {                              
                longpress_count++;

                // Decrease every 4 cycles
                if (longpress_count == 4)
                {
                    counter -= 0.5;
                    longpress_count = 0;
                }
            }
        }

        // -------- CALIBRATION SETTINGS --------
        if (screen == CalibrationSettings)
        {
            if (calibration_value > 0.0)
            {
                longpress_count++;

                if (longpress_count == 3)
                {
                    calibration_value -= 0.1;
                    longpress_count = 0;
                }
            }
            if(calibration_value < 0.0)
                calibration_value = 0.0;
        }

        // -------- SAFETY TEMPERATURE SETTINGS --------
        if (screen == SafteyTemperatureSettings)
        {
            if (Heatersafteytemp > MIN_SAFETY_TEMP)
            {
                longpress_count++;

                if (longpress_count == 5)
                {
                    Heatersafteytemp -= SAFETY_TEMP_STEP;
                    longpress_count = 0;
                }
            }
        }
        // -------- TIME FACTOR SETTINGS --------
        if (screen == TimeFactorSettings)
        {
            if (sfill_time >  SECONDARY_FILL_TIME_LIMIT_LOWER)
            {
                longpress_count++;

                if (longpress_count == 5)
                {
                    sfill_time -= 1;
                    longpress_count = 0;
                }
            }
        }

        // -------- PROBE CALIBRATION SETTINGS --------
        if (screen == ProbeCalibrationSettings)
        {
            if (temp_error >= -(PROBE_CALIBRATION_LIMIT))
            {
                longpress_count++;

                if (longpress_count == 5)
                {
                    temp_error -= 0.1;
                    longpress_count = 0;
                }
            }
        }
    }
}

void buttonClass::increment()
{
    // -------- MAIN SCREEN: INCREASE OUTPUT VOLUME --------
    if(mainscreenflag)
    {
        Max_liter=(optime[optimecounter])*(variant/10.0);    // Calculate max allowed liters

        if(counter<Max_liter)
        {
        counter+=0.5;   // Increase volume in steps of 0.5
        }
        return;
    }

    if(testsettings)                                                               // Inside Test menu
    {
        switch(screen2)
        {
            case TestMenu1:
                if(uppointer)
                {
                    if(dduflag)
                    {
                        screen2=TestMenu2;
                        buttonClass_object.setPointer(0,1);
                        uppointer=0;
                        downpointer=1;

                    }
                    else
                    {
                        screen2=TestMenu1;
                        buttonClass_object.setPointer(0,1);
                        uppointer=0;
                        downpointer=1;
                    }
                    

                }
                else
                {
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
            break;

            case TestMenu2:
                if(uppointer)
                {
                    screen2=TestMenu1;
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
                else
                {
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
            break;
        }
        return;
    }

    // -------- HANDLE SCREEN NAVIGATION & SETTINGS --------

    if(usersettings)
    {
    switch(screen)
    {
//  **************** User Settings *******************


        case UserSettingsScreen1:     // Secondary Fill & Flow Control
            if(uppointer)
                {
                    screen=UserSettingsScreen4;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;

                }
                else
                {
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
            
        break;

            case UserSettingsScreen2:    // Flow Control & Level Control    
            if(uppointer)
            {
                if(dduflag)
                {
                    screen=UserSettingsScreen1;
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
                else
                {
                    screen=UserSettingsScreen3;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
            }
            else 
            {
            buttonClass_object.setPointer(0,0);
              uppointer=1;
              downpointer=0;
            }
            break;

            case UserSettingsScreen3:   // Level Control & Solenoid Control
          if(uppointer)
            {
                screen=UserSettingsScreen2;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break; 

        case UserSettingsScreen4:   // Solenoid Control & Probe Control
          if(uppointer)
            {
                screen=UserSettingsScreen3;
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
            else
            {
                buttonClass_object.setPointer(0,0);
                uppointer=1;
                downpointer=0;
            }
        break;

         case SecondaryFillSettings:       // Secondary Fill Enable/Disable
            if(!secondaryyes)
            {
                buttonClass_object.setPointer(0,1);
                secondaryyes=1;
            }
            else
            {
                buttonClass_object.setPointer(11,1);
                secondaryyes=0;
            }
        break;

         case FlowControlSettings:                                 // Flow Sensor Override
          if(!flowoverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                flowoverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                flowoverride=0;
            }
        break;

        case LevelSensorSettings:                                  // Level Sensor Override
            if(!leveloverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                leveloverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                leveloverride=0;
            }
        break;

        case SolenoidControlSettings:                            // Solenoid Override
            if(!solenoidoverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                solenoidoverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                solenoidoverride=0;
            }
        break;

         case TempSensorSettings:                                   // Temperature Probe Override
            if(!probeoverride)
            {
                lcd.setCursor(0,1);
                lcd.print("> OVERRIDE");
                probeoverride=1;
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("> ACTIVE     ");
                probeoverride=0;
            }
        break;

        
        }
    return;
    }

//  **************** Service menu Settings *******************        

        if(servicemenu)
        {
            switch(screen)
            {
                case ServiceMenuScreen1:           // Product Type & Subproduct Type
                    if(dduflag)
                    {
                        if(uppointer)
                        {
                            screen=ServiceMenuScreen7;
                            buttonClass_object.setPointer(0,1);
                            uppointer=0;
                            downpointer=1;
                        }
                        else
                        {
                            buttonClass_object.setPointer(0,0);
                            uppointer=1;
                            downpointer=0;
                        }
                    }
                    else
                    {
                        if(uppointer)
                        {
                            screen=SDUServiceMenuScreen4;
                            buttonClass_object.setPointer(0,1);
                            uppointer=0;
                            downpointer=1;
                        }
                        else
                        {
                            buttonClass_object.setPointer(0,0);
                            uppointer=1;
                            downpointer=0;
                        }

                    }
                break;

                case ServiceMenuScreen2:                             // Subproduct Type & Calibration
                if(uppointer)
                    {
                        screen=ServiceMenuScreen1;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case ServiceMenuScreen3:                                // Calibration & Safety Temperature
                if(uppointer)
                    {
                        screen=ServiceMenuScreen2;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case ServiceMenuScreen4:                                  // Safety Temperature & Probe Calibration
                if(uppointer)
                    {
                        screen=ServiceMenuScreen3;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case ServiceMenuScreen5:                                  // Probe Calibration & Product Type
                if(uppointer)
                    {
                        screen=ServiceMenuScreen4;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case ServiceMenuScreen6:                                  // Probe Calibration & Product Type
                if(uppointer)
                    {
                        screen=ServiceMenuScreen5;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case ServiceMenuScreen7:                                  // Probe Calibration & Product Type
                if(uppointer)
                    {
                        screen=ServiceMenuScreen6;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case SDUServiceMenuScreen4:                                // Calibration & Product Type (SDU)
                if(uppointer)
                    {
                        screen=ServiceMenuScreen3;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }
                break;

                case ProductTypeSettings:                                     // Product type selection (DDU/SDU)
                    if(!dduflag)
                    {
                        lcd.clear();
                        lcd.setCursor(11,1);
                        lcd.print(">DDU");
                        dduflag=1;
                    }
                    else
                    {
                        lcd.clear();
                        lcd.setCursor(0,1);
                        lcd.print(">SDU     ");
                        dduflag=0;
                    }
                break;

                case SubProductTypeSettings:                                   // Subproduct type selection (150,250,400)
                    if(prodtypecounter<2){
                        prodtypecounter++;
                    }
                    else
                    {
                        prodtypecounter=0;
                    }
                break;

                case CalibrationSettings:                                     // Calibration value adjustment
                    if(calibration_value<Max_liter+5.0)
                    {
                        calibration_value+=0.1;
                    }
                break;

                case OperatingTimeSettings:                                 // Operating time adjustment (affects max volume calculation)
                    if(optimecounter<2)
                    {
                        optimecounter++;    
                    }
                    else
                    {
                        optimecounter=0;
                    }

                break;

                case SafteyTemperatureSettings:                                // Heater safety temperature adjustment
                    if(Heatersafteytemp<MAX_SAFETY_TEMP)
                    {
                    Heatersafteytemp+=SAFETY_TEMP_STEP;
                    }
                break;

                case ProbeCalibrationSettings:                                  // Temperature probe calibration adjustment
                    if(temp_error<=PROBE_CALIBRATION_LIMIT)
                    {
                    temp_error+=0.1;
                    }

                break;

                case TimeFactorSettings:                                         // Secondary fill time adjustment
                    if(sfill_time< SECONDARY_FILL_TIME_LIMIT)
                    {
                        sfill_time+=1;
                    }
                    
                break;

                case FactoryResetScreen:                                         // Factory reset confirmation toggle
                    if(!factoryresetflag)
                    {
                        lcd.clear();
                        buttonClass_object.setPointer(0,1);
                        lcd.print("RESET PARAMETERS     ");
                        factoryresetflag=1;
                    }
                    else
                    {
                        lcd.clear();
                        buttonClass_object.setPointer(0,1);
                        lcd.print("SENSORS TESTING   ");
                        factoryresetflag=0;
                    }
                break;  
                }
                return;
        } 
}



void buttonClass::decrement()
{

    // -------- MAIN SCREEN FUNCTIONALITY --------
    // When on main screen → decrease selected output volume
    if(mainscreenflag)                                      
    {
        if(counter > 0.0)           // Ensure value does not go below 0
        {
            counter -= 0.5;         // Decrease volume in steps of 0.5 liters
        }
        return;                     // Exit after handling main screen logic
    }

    if(testsettings)
    {
        switch(screen2)                                // Handle navigation inside test menu
        {
            case TestMenu1:
                if(downpointer)
                {
                    if(dduflag)
                    {
                        
                        screen2=TestMenu2;
                        buttonClass_object.setPointer(0,1);
                        uppointer=0;
                        downpointer=1;
                    }
                    else if(dduflag==0)
                    {
                        
                        screen2=TestMenu1;
                        buttonClass_object.setPointer(0,0);
                        uppointer=1;
                        downpointer=0;
                    }

                }
                else
                {
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
            break;

            case TestMenu2:
                if(downpointer)
                {
                    screen2=TestMenu1;
                    buttonClass_object.setPointer(0,0);
                     uppointer=1;
                    downpointer=0;

                }
                else
                {
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
            break; 
        }
        return;
    }
    // **************** USER SETTINGS *******************
    // Handles navigation and parameter changes inside user settings menu
    if(usersettings)
    {
        switch(screen)
        {

            // -------- USER SETTINGS SCREEN 1 --------
            case UserSettingsScreen1:
                if(downpointer)     // If pointer is already on lower option
                {
                    screen = UserSettingsScreen2;           // Move to next screen
                    buttonClass_object.setPointer(0,1);     // Set pointer to lower position
                    downpointer = 1;
                    uppointer = 0;   
                }
                else                // If pointer is on upper option
                {
                    buttonClass_object.setPointer(0,1);     // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- USER SETTINGS SCREEN 2 --------
            case UserSettingsScreen2:
                if(downpointer)
                {
                    screen = UserSettingsScreen3;           // Move to next screen
                    buttonClass_object.setPointer(0,1);
                    downpointer = 1;
                    uppointer = 0;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);     // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- USER SETTINGS SCREEN 3 --------
            case UserSettingsScreen3:
                if(downpointer)
                {
                    if(dduflag)     // If DDU variant supports next screen
                    {
                        screen = UserSettingsScreen4;       // Move forward
                        buttonClass_object.setPointer(0,1);
                        downpointer = 1;
                        uppointer = 0;
                    }
                    else            // If not supported → wrap back
                    {
                        screen = UserSettingsScreen2;
                        buttonClass_object.setPointer(0,0);
                        downpointer = 0;
                        uppointer = 1;
                    }   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);     // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- USER SETTINGS SCREEN 4 --------
            case UserSettingsScreen4:
                if(downpointer)
                {
                    if(dduflag)     // If DDU variant → wrap to first screen
                    {
                        screen = UserSettingsScreen1;
                        buttonClass_object.setPointer(0,0);
                        downpointer = 0;
                        uppointer = 1;
                    }
                    else            // Otherwise → go to screen 2
                    {
                        screen = UserSettingsScreen2;
                        buttonClass_object.setPointer(0,0);
                        downpointer = 0;
                        uppointer = 1;
                    } 
                }
                else
                {
                    buttonClass_object.setPointer(0,1);     // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SECONDARY FILL SETTING --------
            case SecondaryFillSettings:
                if(!secondaryyes)  // If currently disabled
                {
                    buttonClass_object.setPointer(0,1); // Select YES option
                    secondaryyes = 1;                   // Enable secondary fill
                }
                else                // If currently enabled
                {
                    buttonClass_object.setPointer(11,1); // Move pointer to NO option
                    secondaryyes = 0;                    // Disable secondary fill
                }
            break;

            // -------- FLOW SENSOR CONTROL --------
            case FlowControlSettings:
                if(!flowoverride)   // If override is OFF
                {
                    lcd.setCursor(0,1);
                    lcd.print("> OVERRIDE");  // Show override option
                    flowoverride = 1;         // Enable override
                }
                else                // If override is ON
                {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE     "); // Show active state
                    flowoverride = 0;           // Disable override
                }
            break;

            // -------- LEVEL SENSOR CONTROL --------
            case LevelSensorSettings:
                if(!leveloverride)
                {
                    lcd.setCursor(0,1);
                    lcd.print("> OVERRIDE");  // Enable override display
                    leveloverride = 1;
                }
                else
                {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE     "); // Return to normal operation
                    leveloverride = 0;
                }
            break;

            // -------- SOLENOID CONTROL --------
            case SolenoidControlSettings:
                if(!solenoidoverride)
                {
                    lcd.setCursor(0,1);
                    lcd.print("> OVERRIDE");  // Enable manual override
                    solenoidoverride = 1;
                }
                else
                {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE     "); // Return to automatic control
                    solenoidoverride = 0;
                }
            break;

            // -------- TEMPERATURE SENSOR CONTROL --------
            case TempSensorSettings:
                if(!probeoverride)
                {
                    lcd.setCursor(0,1);
                    lcd.print("> OVERRIDE");  // Enable probe override
                    probeoverride = 1;
                }
                else
                {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE     "); // Use actual sensor reading
                    probeoverride = 0;
                }
            break;   
        }

        return; // Exit after handling user settings
    }


    if(servicemenu)
    {
    // **************** SERVICE MENU *******************
    // Handles navigation and parameter control inside service menu

        switch(screen)
        {
            // -------- SERVICE MENU SCREEN 1 --------
            case ServiceMenuScreen1:
                if(downpointer)   // If pointer already on lower option
                {
                    screen = ServiceMenuScreen2;          // Move to next screen
                    buttonClass_object.setPointer(0,1);   // Set pointer to lower position
                    downpointer = 1;
                    uppointer = 0;   
                }
                else              // If pointer on upper option
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SERVICE MENU SCREEN 2 --------

            case ServiceMenuScreen2:
                if(downpointer)   // If pointer already on lower option
                {
                    screen = ServiceMenuScreen3;          // Move to next screen
                    buttonClass_object.setPointer(0,1);   // Set pointer to lower position
                    downpointer = 1;
                    uppointer = 0;   
                }
                else              // If pointer on upper option
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SERVICE MENU SCREEN 3 --------

            case ServiceMenuScreen3:
                if(dduflag)   // If DDU variant is selected
                {
                    if(downpointer)
                    {
                        screen = ServiceMenuScreen4;      // Move to next screen
                        buttonClass_object.setPointer(0,1);
                        downpointer = 1;
                        uppointer = 0;   
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,1); // Move pointer down
                        downpointer = 1;
                        uppointer = 0;
                    }
                }
                else          // If SDU variant is selected
                {
                    if(downpointer)
                    {
                        screen = SDUServiceMenuScreen4;   // Move to SDU-specific screen
                        buttonClass_object.setPointer(0,1);
                        downpointer = 1;
                        uppointer = 0;   
                    }
                    else
                    {
                        buttonClass_object.setPointer(0,1); // Move pointer down
                        downpointer = 1;
                        uppointer = 0;
                    }
                }
            break;

            // -------- SERVICE MENU SCREEN 4 --------

            case ServiceMenuScreen4:
                if(downpointer)
                {
                    screen = ServiceMenuScreen5;          // Move to next screen
                    buttonClass_object.setPointer(0,1);
                    downpointer = 1;
                    uppointer = 0;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SERVICE MENU SCREEN 5 --------
            case ServiceMenuScreen5:
                if(downpointer)
                {
                    screen = ServiceMenuScreen6;          // Wrap back to first screen
                    buttonClass_object.setPointer(0,1);   // Move pointer to top
                    downpointer = 1;
                    uppointer = 0;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SERVICE MENU SCREEN 6 --------

            case ServiceMenuScreen6:
                if(downpointer)
                {
                    screen = ServiceMenuScreen7;          // Wrap back to first screen
                    buttonClass_object.setPointer(0,0);   // Move pointer to top
                    downpointer = 0;
                    uppointer = 1;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SERVICE MENU SCREEN 7 --------

            case ServiceMenuScreen7:
                if(downpointer)
                {
                    screen = ServiceMenuScreen1;          // Wrap back to first screen
                    buttonClass_object.setPointer(0,0);   // Move pointer to top
                    downpointer = 0;
                    uppointer = 1;   
                }
                else
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- SDU SERVICE MENU SCREEN 3 --------

            case SDUServiceMenuScreen4:
                if(downpointer)
                {
                    screen = ServiceMenuScreen1;          // Return to main service screen
                    buttonClass_object.setPointer(0,0);   // Reset pointer to top
                    downpointer = 0;
                    uppointer = 1;    
                }
                else
                {
                    buttonClass_object.setPointer(0,1);   // Move pointer down
                    downpointer = 1;
                    uppointer = 0;
                }
            break;

            // -------- PRODUCT TYPE SELECTION --------
            case ProductTypeSettings:
                if(!dduflag)   // If currently SDU
                {
                    lcd.clear();
                    lcd.setCursor(11,1);
                    lcd.print(">DDU");   // Show DDU option
                    dduflag = 1;         // Switch to DDU
                }
                else           // If currently DDU
                {
                    lcd.clear();
                    lcd.setCursor(0,1);
                    lcd.print(">SDU     "); // Show SDU option
                    dduflag = 0;            // Switch to SDU
                }
            break;

            // -------- SUB PRODUCT TYPE SELECTION --------
            case SubProductTypeSettings:
                if(prodtypecounter > 0)
                {
                    prodtypecounter--;   // Decrement product type index
                }
                else
                {
                    prodtypecounter = 2; // Wrap around to last option
                }
            break;

            // -------- CALIBRATION SETTINGS --------
            case CalibrationSettings:
                if(calibration_value > 0.0)
                {
                    calibration_value -= 0.1;  // Decrease calibration offset
                }
                if(calibration_value < 0.0)
                    calibration_value = 0.0;   // Clamp to zero (no negative values)
            break;

            // -------- OPERATING TIME SETTINGS --------
            case OperatingTimeSettings:
                    if(optimecounter>0)
                    {
                        optimecounter--;    
                    }
                    else
                    {
                        optimecounter=2;
                    }

                break;


            // -------- SAFETY TEMPERATURE SETTINGS --------
            case SafteyTemperatureSettings:
                if(Heatersafteytemp > MIN_SAFETY_TEMP)
                {
                    Heatersafteytemp -= SAFETY_TEMP_STEP; // Decrease safety temperature
                }    
            break;

            // -------- PROBE CALIBRATION SETTINGS --------
            case ProbeCalibrationSettings:
                if(temp_error >= -(PROBE_CALIBRATION_LIMIT))
                {
                    temp_error -= 0.1;  // Adjust probe error negatively
                }
            break;

            // -------- TIME FACTOR SETTINGS --------
            case TimeFactorSettings:
                    if(sfill_time > SECONDARY_FILL_TIME_LIMIT_LOWER)
                    {
                        sfill_time-=1;
                    }
            break;

            // -------- FACTORY RESET CONFIRMATION --------
            case FactoryResetScreen:                                         // Factory reset confirmation toggle
                    if(!factoryresetflag)
                    {
                        lcd.clear();
                        buttonClass_object.setPointer(0,1);
                        lcd.print("RESET PARAMETERS     ");
                        factoryresetflag=1;
                    }
                    else
                    {
                        lcd.clear();
                        buttonClass_object.setPointer(0,1);
                        lcd.print("SENSORS TESTING   ");  
                        factoryresetflag=0;
                    }
            break;  

        }
        return; // Exit after handling service menu
    }
}

void buttonClass:: user_settings()
{
    // -------- ENTER USER SETTINGS --------
    // Conditions:
    // - Not already in user settings
    // - No active process running
    // - Not inside service menu
    // - No secondary timer running
    // - Not in test menu or test settings
    // - No error check or dryout process active
    // - Not in override screen
    // If all conditions are met, enter user settings mode
    if(!usersettings && !process_flag && !preheat_flag && !servicemenu && !secondarytimerflag && !intestmenu && !testsettings && !error_check_flag && !dryout_flag && !overridescreenflag)
    {
      lcd.clear();

       // Select initial screen based on product type
      if(dduflag)
      {
        screen=UserSettingsScreen1;                         // Start from screen 1 for DDU
      }
      else
      {
        screen=UserSettingsScreen2;                         // Start from screen 2 for SDU (skipping secondary fill setting)
      }

       // -------- UPDATE UI STATES --------
      usersettings=1;                                      // Enter user settings mode
      mainscreenflag=0;                                     // Exit main screen

       // -------- LED INDICATION --------
      digitalWrite(YELLOW_LED,LOW);                        // Turn OFF standby LED
      digitalWrite(GREEN_LED,HIGH);                        // Turn ON settings LED

       // -------- BUZZER FEEDBACK --------
      digitalWrite(BUZZER,HIGH);
      buzzerclass_object.Buzzer_beep(1000);
      buzzerclass_object.Buzzer_start();

       // -------- INITIAL POINTER SETUP --------
      buttonClass_object.setPointer(0,0);
      uppointer=1;
      return;
    }

    // -------- SECONDARY TIMER SKIP FUNCTION --------
    // Allows user to skip remaining time during secondary process
     if((secondarytimerflag || dryout_flag ) && !time_skip)
    {
        skip_count++;   // Increment skip counter on button press

        if (skip_count >= 50)   // Threshold reached for skip action
        {
            digitalWrite(BUZZER, HIGH);
            delay(100);
            if(dryout_flag)
            {
                one_second_counter = dryout_fill_time - 5;  // Jump near end of dryout timer

            }
            else
            {
            one_second_counter = pre_end_time - 5; // Jump near end of secondary fill timer
            }
            skip_count = 0;                        // Reset counter
            time_skip = 1;                         // Mark skip completed

            // -------- USER FEEDBACK --------
            
            buzzerclass_object.Buzzer_beep(2000);
            buzzerclass_object.Buzzer_start();
        }
        return;
    }
    else
    {
        skip_count = 0;   // Reset counter if condition not met
        return;
    }
}

void buttonClass:: back_screen()
{
    if(overridescreenflag)                  // If currently in override acknowledgment screen, exit to main screen
    {
        lcd.clear();
        overridescreenflag=0;
        flowcheckscreen=0;                  // Reset all the override acknowledgment flags
        levelcheckscreen=0;
        probecheckscreen=0;
        screen=MainScreen;
        mainscreenflag=1;
        usersettings=0;
        servicemenu=0;

        // Reset pointer states
        uppointer=0;
        downpointer=0;
        heater_start=0;

        process_object.Contactor1_stop();
        process_object.Solenoid1_stop();
        process_object.Solenoid2_stop();
        return;
    }

     if(error_check_flag && closetap)          // If Solenoid control is ACTIVE and back button is pressed → exit to main screen
    {
        lcd.clear();
        error_check_flag=0;
        closetap=0;
        lcd_object.lcd_buzzer_toggle_stop();
        return;
    }

    if(error_check_flag && Secodaryfill_error_flag)   // If secondary fill error is active and back button is pressed → exit to main screen
    {
        lcd.clear();
        error_check_flag=0;
        Secodaryfill_error_flag=0;
        lcd_object.lcd_buzzer_toggle_stop();
        return;
    }

    // -------- BACK FROM MAIN MENUS --------
    // If in user settings or service menu and not inside submenu

    if((usersettings || servicemenu) && !inmenu){
        lcd.clear();
        // eeprom_object.eeprom_datawrite();

        // -------- RESET TO MAIN SCREEN --------
        screen=MainScreen;
        mainscreenflag=1;
        usersettings=0;
        servicemenu=0;

        // Reset pointer states
        uppointer=0;
        downpointer=0;
        return;
    }

    if(intestmenu)                                      // If inside test menu and back button is pressed → exit to main screen
    {
        intestmenu=0;
        buttonClass_object.setPointer(0,0);
        switch(screen2)
        {
            case FlowSensorTest:
                digitalWrite(SOLENOID1, LOW); 
                screen2=TestMenu1;
                uppointer=1;
                downpointer=0;
            break;

            case LevelSensorTest:
                digitalWrite(SOLENOID1, LOW); 
                digitalWrite(POWER_DETECTION, HIGH);  // Sensor OFF
                delay(100);
                if(dduflag)
                {
                    screen2=TestMenu2;
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
                else
                {
                    screen2=TestMenu1;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
            break;

            case PT100ProbeTest:
                screen2=TestMenu2;
                buttonClass_object.setPointer(0,1);
                uppointer=0;
                downpointer=1;
            break;
        }

        return;

    }


    if(inmenu && testsettings && !intestmenu)              // If inside test settings submenu and back button is pressed → exit to main test menu
    {
      
        if(dduflag)
            {
                screen = ServiceMenuScreen7;
            }
            else
            {
                screen = SDUServiceMenuScreen4;
            }
        // Reset pointer states
        uppointer = 0;
        downpointer = 1;
        buttonClass_object.setPointer(0,1);
        inmenu = 0;   // Exit submenu mode
        testsettings=0;
        return;
    }

    // -------- BACK FROM SUBMENU --------
    // Handles exiting from parameter editing screens
    if(inmenu && !testsettings)
    {
        // -------- RESET POINTER POSITION --------
        uppointer=1;
        downpointer=0;
        buttonClass_object.setPointer(0,0);
        inmenu=0;                                       // Exit submenu mode

        switch(screen)
        {
            // -------- FLOW CONTROL SETTINGS --------
            case FlowControlSettings:
                EEPROM.get(FLOW_CONTROL, flowoverride); // Restore saved value
                screen = UserSettingsScreen2;
                usersettings = 1;
            break;

            // -------- LEVEL SENSOR SETTINGS --------
            case LevelSensorSettings:
                EEPROM.get(LEVEL_CONTROL, leveloverride); 
                screen = UserSettingsScreen3;
                usersettings = 1;
            break;

            // -------- SOLENOID CONTROL SETTINGS --------
            case SolenoidControlSettings:
                EEPROM.get(SOLENOID_CONTROL, solenoidoverride);

                if(dduflag)
                {
                    screen = UserSettingsScreen4;
                    usersettings = 1;
                }
                else
                {
                    uppointer = 0;
                    downpointer = 1;
                    buttonClass_object.setPointer(0,1);
                    screen = UserSettingsScreen3;
                    usersettings = 1;
                }
            break;

            // -------- TEMPERATURE SENSOR SETTINGS --------
            case TempSensorSettings:
                EEPROM.get(PROBE_CONTROL, probeoverride);

                uppointer = 0;
                downpointer = 1;
                buttonClass_object.setPointer(0,1); // Set pointer to lower option
                screen = UserSettingsScreen4;
                usersettings = 1;
            break;

             // -------- SECONDARY FILL SETTINGS --------
            case SecondaryFillSettings:
                EEPROM.get(SECONDARY_FILL, secondaryyes);
                screen = UserSettingsScreen1;
                usersettings = 1;
            break;


            // -------- PRODUCT TYPE SETTINGS --------
            case ProductTypeSettings:
                dduflag = EEPROM.read(PRODUCT_SELECTION); // Restore product type
                screen = ServiceMenuScreen1;
                servicemenu = 1;
            break;

            // -------- SUB PRODUCT TYPE SETTINGS --------
            case SubProductTypeSettings:
                EEPROM.get(SUBPRODUCT_SELECTION, prodtypecounter);
                screen = ServiceMenuScreen2;
                servicemenu = 1;

                // Recalculate maximum volume based on variant
                Max_liter=(optime[optimecounter])*(variant/10.0);
            break;

            // -------- CALIBRATION SETTINGS --------
            case CalibrationSettings:
                EEPROM.get(CALIBRATION_VALUE, calibration_value);
                servicemenu = 1;
                screen = ServiceMenuScreen3;
                
            break;
            
            // -------- OPERATING TIME SETTINGS --------
            case OperatingTimeSettings:

                EEPROM.get(EEPROM_OPERATING_TIME_ADDR, optimecounter);

                servicemenu = 1;
                if(dduflag)
                {
                    screen = ServiceMenuScreen4;
                }
                else
                {
                    screen = SDUServiceMenuScreen4;
                }
            break;

           

            // -------- SAFETY TEMPERATURE SETTINGS --------
            case SafteyTemperatureSettings:
                EEPROM.get(SAFETY_TEMP, Heatersafteytemp);
                screen = ServiceMenuScreen5;
                servicemenu = 1;
            break;

            // -------- PROBE CALIBRATION SETTINGS --------
            case ProbeCalibrationSettings:
                EEPROM.get(PROBE_ERROR, temp_error);
                screen = ServiceMenuScreen6;
                servicemenu = 1;
            break;

            
            // -------- TIME FACTOR SETTINGS --------
            case TimeFactorSettings:
                EEPROM.get(SECONDARY_FILL_TIME, sfill_time);
                screen = ServiceMenuScreen7;
                servicemenu = 1;
            break;

            // -------- FACTORY RESET SCREEN --------
            case FactoryResetScreen:
                uppointer = 0;
                downpointer = 1;
                servicemenu = 1;
                buttonClass_object.setPointer(0,1);

                if(dduflag)
                {
                    screen = ServiceMenuScreen7;
                }
                else
                {
                    screen = SDUServiceMenuScreen4;
                }
            break;
        }
        return;
    }
}

void buttonClass::enter_function()
{
    // -------- ERROR HANDLING : CLOSE TAP --------
    // If Solenoid control is ACTIVE and back button is pressed → exit to main screen
    if(error_check_flag && closetap)
    {
        lcd.clear();
        error_check_flag=0;
        closetap=0;
        lcd_object.lcd_buzzer_toggle_stop();
        return;
    }

    if(error_check_flag && Secodaryfill_error_flag)      // If error due to secondary fill → clear error and return to main screen
    {
        lcd.clear();
        error_check_flag=0;
        Secodaryfill_error_flag=0;
        lcd_object.lcd_buzzer_toggle_stop();
        return;
    }

    if(overridescreenflag)                               
    {
        overridescreenflag=0;
        return;
    }

    // -------- MAIN SCREEN : START PROCESS --------
    if(mainscreenflag && counter>0.0)
    {
        // Apply variant-specific settings
        one_second_counter=0;                          // Reset main timer counter
        one_second_counter2=0;                          // Reset secondary timer counter

        probeerrorflag=0;

        process_object.variant_settings();
        digitalWrite(BUZZER,HIGH);
        buzzerclass_object.Buzzer_beep(1000);
        buzzerclass_object.Buzzer_start();
        process_object.Solenoid1_start();   // Start solenoid 1 for process initiation
        
        if(flowoverride || leveloverride)
        {
            screen=OverrideScreen;              // Go to override screen if any override is active
            mainscreenflag=0;
            overridescreenflag=1;
            flowcheckscreen=1;
            levelcheckscreen=1;
            return;
        }
        // -------- SECONDARY FILL FLOW --------
        if(secondaryyes && dduflag)
        {
            screen=SecondaryFillTimer;              // Go to secondary timer screen
            mainscreenflag=0;
            one_second_counter=0;
            secondarytimerflag=1;
            pauseflag=0;
            error_check_flag=0;
            heater_start=1;
            process_object.Contactor1_start();          // Start first contactor and then heater 1
        }
        else
        {
            // -------- DIRECT PROCESS START WITH PREHEAT --------
            preheat_flag=1;
            screen=ProcessScreen;
            mainscreenflag=0;
            one_second_counter=0;
            pauseflag=0;
            error_check_flag=0;
            // Perform error check before starting
            process_object.error_check();
            if(error_check_flag)
            {
                return;                         // Stop if error detected
            }
            heater_start=1;
            process_object.Contactor1_start();   // Start process(Contactors and heaters starts)
        }
        return;
    }
    if(intestmenu)
    {
        intestmenu=0;
        buttonClass_object.setPointer(0,0);
        switch(screen2)
        {
            case FlowSensorTest:
                digitalWrite(SOLENOID1, LOW);
                screen2=TestMenu1;
                uppointer=1;
                downpointer=0;
            break;

            case LevelSensorTest:
                digitalWrite(POWER_DETECTION, HIGH);  // Sensor OFF
                digitalWrite(SOLENOID1, LOW);
                if(dduflag)
                {
                    screen2=TestMenu2;
                    buttonClass_object.setPointer(0,0);
                    uppointer=1;
                    downpointer=0;
                }
                else
                {
                    screen2=TestMenu1;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
            break;

            case PT100ProbeTest:
                screen2=TestMenu2;
                buttonClass_object.setPointer(0,1);
                uppointer=0;
                downpointer=1;
            break;
        }

        return;

    }

    if((inmenu && testsettings && !intestmenu))               // If inside test settings submenu and enter button is pressed → execute test action
    {
       
        switch(screen2)
        {
            case TestMenu1:
                if(downpointer)  
                {
                    digitalWrite(SOLENOID1, HIGH);
                    screen2 = LevelSensorTest; // Enter level sensor test
                    lcd.clear();
                    intestmenu = 1;
                    digitalWrite(POWER_DETECTION, LOW);  // Sensor ON
                    delay(100);
                    
                }
                else
                {
                    digitalWrite(SOLENOID1, HIGH);             
                    lcd.clear();
                    screen2= FlowSensorTest; 
                    intestmenu = 1;
                }
            break;

            case TestMenu2:
                if(downpointer)
                {
                    screen2 = PT100ProbeTest; // Enter flow sensor test
                    lcd.clear();
                    intestmenu = 1;
                }
                else
                {
                    digitalWrite(SOLENOID1, HIGH);
                    screen2 = LevelSensorTest; 
                    lcd.clear();
                    intestmenu = 1;
                    digitalWrite(POWER_DETECTION, LOW);  // Sensor ON
                    delay(100);
                    
                }
            break;

           
        }
        return;

    }


    // -------- MENU CONFIRM ACTION (ENTER IN SUBMENU) --------
        if(inmenu  && !testsettings)
        {
        uppointer=1;
        downpointer=0;
        buttonClass_object.setPointer(0,0);
        inmenu=0;

        switch(screen)
        {
             // -------- SAVE SECONDARY FILL --------
            case SecondaryFillSettings:
                 EEPROM.put(SECONDARY_FILL, secondaryyes);
                screen=UserSettingsScreen1;
                usersettings=1;

            break;

            // -------- SAVE FLOW CONTROL --------
            case FlowControlSettings:
                EEPROM.put(FLOW_CONTROL, flowoverride);
                screen=UserSettingsScreen2;
                usersettings=1;
            break;
            
            // -------- SAVE LEVEL SENSOR --------
            case LevelSensorSettings:
                EEPROM.put(LEVEL_CONTROL, leveloverride);
                screen=UserSettingsScreen3;
                usersettings=1; 
            break;

             // -------- SAVE SOLENOID CONTROL --------
            case SolenoidControlSettings:
                EEPROM.put(SOLENOID_CONTROL, solenoidoverride);
                usersettings=1;
                if(dduflag)
                {
                    screen=UserSettingsScreen4;
                }
                else
                {
                    screen=UserSettingsScreen3;
                    buttonClass_object.setPointer(0,1);
                    uppointer=0;
                    downpointer=1;
                }
                
            break;

            // -------- SAVE TEMPERATURE SENSOR --------
            case TempSensorSettings:
                EEPROM.put(PROBE_CONTROL, probeoverride);
                screen=UserSettingsScreen4;
                usersettings=1;
                buttonClass_object.setPointer(0,1);
                uppointer=0;
                downpointer=1;
                
            break;
  
            // -------- SAVE PRODUCT TYPE --------
            case ProductTypeSettings:
                screen=ServiceMenuScreen1;
                servicemenu=1;
                EEPROM.write(PRODUCT_SELECTION, dduflag);
                delay(50);
            break;

            // -------- SAVE SUB PRODUCT TYPE --------
            case SubProductTypeSettings:
                EEPROM.put(SUBPRODUCT_SELECTION,prodtypecounter);
                screen=ServiceMenuScreen2;
                servicemenu=1;
                calibration_value=base_calibration[prodtypecounter];
                delay(50);
                EEPROM.put(CALIBRATION_VALUE, calibration_value);
                delay(50);
                Max_liter = (optime[optimecounter]) * (variant / 10.0);
                counter = 0.00;
                sfill_time = Sfill_default[prodtypecounter];
                EEPROM.put(SECONDARY_FILL_TIME, sfill_time);
            break;
            
            // -------- SAVE CALIBRATION --------
            case CalibrationSettings:
                EEPROM.put(CALIBRATION_VALUE, calibration_value);
                servicemenu=1;
                screen=ServiceMenuScreen3;
                
            break;
            
            // -------- SAVE OPERATING TIME --------
            case OperatingTimeSettings:
                EEPROM.put(EEPROM_OPERATING_TIME_ADDR, optimecounter);
                servicemenu=1;
                counter = 0.00;
                if(dduflag)
                {
                    screen=ServiceMenuScreen4;
                }
                else
                {
                    screen=SDUServiceMenuScreen4;
                }
            break;

            // -------- SAVE SAFETY TEMPERATURE --------
            case SafteyTemperatureSettings:
                EEPROM.put(SAFETY_TEMP, Heatersafteytemp);
                screen=ServiceMenuScreen5;
                servicemenu=1;
            break;
            
            // -------- SAVE PROBE CALIBRATION --------
            case ProbeCalibrationSettings:
                EEPROM.put(PROBE_ERROR, temp_error);
                screen=ServiceMenuScreen6;
                servicemenu=1;
            break;
            
            // -------- SAVE TIME FACTOR --------
            case TimeFactorSettings:
                EEPROM.put(SECONDARY_FILL_TIME, sfill_time);
                screen=ServiceMenuScreen7;
                servicemenu=1;
                
             break;
            
            // -------- FACTORY RESET --------
            case FactoryResetScreen:
                if(factoryresetflag)                              // If user confirmed reset
                {
                    lcd.setCursor(0,0);
                    lcd.print(" FACTORY RESET");
                    lcd.setCursor(5,1);
                    lcd.print(" DONE"); 
                    digitalWrite(BUZZER,HIGH);
                    eeprom_object.eeprom_defaultvalue();

                    delay(2000);
                    digitalWrite(BUZZER,LOW);
                    uppointer=1;
                    downpointer=0;
                    mainscreenflag=1;
                    usersettings=0;
                    servicemenu=0;
                    lcd.clear();
                    screen=MainScreen;
                    variant=((prodtype[prodtypecounter])/10);
                    Max_liter=(optime[optimecounter])*(variant/10.0);


                }
                else                                                 // If reset not confirmed
                {
                    uppointer=1;
                    downpointer=0;
                    testsettings=1;
                     inmenu=1;
                    screen=TestingScreen;
                    screen2=TestMenu1;
                }
            break;
        }
        return;
    }


    if(servicemenu && !inmenu)
    {
        // **************** SERVICE MENU ENTRY *******************
        // Handles ENTER button action when navigating service menu (not inside submenu)
        switch(screen)
    {
        // -------- SERVICE MENU SCREEN 1 --------
        case ServiceMenuScreen1:
            if(uppointer)   // If upper option selected
            {
                screen = ProductTypeSettings;  // Enter product type settings
                inmenu = 1;

                // Set pointer based on current product type
                if(dduflag)
                {
                    buttonClass_object.setPointer(11,1); // Point to DDU
                }
                else
                {
                    buttonClass_object.setPointer(0,1);  // Point to SDU
                }
            }
            else              // If lower option selected
            {
                lcd.clear();
                screen = SubProductTypeSettings; // Enter sub product selection
                inmenu = 1;
            }
        break;

        // -------- SERVICE MENU SCREEN 2 --------
        case ServiceMenuScreen2:
            if(downpointer)   // Lower option selected
            {
                screen = CalibrationSettings; // Enter calibration settings
                lcd.clear();
                inmenu = 1;
            }
            else              // Upper option selected
            {
                lcd.clear();
                screen = SubProductTypeSettings; // Enter sub product selection
                inmenu = 1;
            }
        break;

        // -------- SERVICE MENU SCREEN 3 --------
        case ServiceMenuScreen3:
            if(downpointer)
            {
                lcd.clear();
                screen = OperatingTimeSettings; // Enter operating time settings
                inmenu = 1;
            }
            else
            {
                screen = CalibrationSettings; // Enter calibration settings
                lcd.clear();
                inmenu = 1;
            }
        break;

        case ServiceMenuScreen4:
            if(downpointer)
            {
                lcd.clear();
                screen = SafteyTemperatureSettings; // Enter safety temperature settings
                inmenu = 1;
            }
            else
            {
                lcd.clear();
                screen = OperatingTimeSettings; // Enter operating time settings
                inmenu = 1;
            }
        break;

        // -------- SERVICE MENU SCREEN 4 --------
        case ServiceMenuScreen5:
            if(downpointer)
            {
                lcd.clear();
                screen = ProbeCalibrationSettings; // Enter probe calibration
                inmenu = 1;
            }
            else
            {
                lcd.clear();
                screen = SafteyTemperatureSettings; // Enter safety temp settings
                inmenu = 1;
            }
        break;

        // -------- SERVICE MENU SCREEN 5 --------
        case ServiceMenuScreen6:
            if(downpointer)
            {
                factoryresetflag = 1;          // Default selection → YES
                screen = TimeFactorSettings;  // Enter factory reset screen
                inmenu = 1;

                // buttonClass_object.setPointer(0,1); // Set pointer to confirmation
                // lcd.clear();
                // buttonClass_object.setPointer(0,1);
                // lcd.print("RESET PARAMETER     ");
            }
            else
            {
                lcd.clear();
                screen = ProbeCalibrationSettings; // Enter probe calibration
                inmenu = 1;
            }
        break;

        case ServiceMenuScreen7:
            if(downpointer)
            {
                factoryresetflag = 1;          // Default selection → YES
                screen = FactoryResetScreen;  // Enter factory reset screen
                inmenu = 1;

                buttonClass_object.setPointer(0,1); // Set pointer to confirmation
                // lcd.clear();
                // buttonClass_object.setPointer(0,1);
                lcd.print("RESET PARAMETER     ");
            }
            else
            {
                factoryresetflag = 1;          // Default selection → YES
                screen = TimeFactorSettings;  // Enter factory reset screen
                inmenu = 1;
            }
        break;

        // -------- SDU SERVICE MENU SCREEN 3 --------
        case SDUServiceMenuScreen4:
            if(downpointer)
            {
                screen = FactoryResetScreen; // Enter factory reset
                factoryresetflag = 1;        // Default → YES
                inmenu = 1;

                buttonClass_object.setPointer(0,1); // Set pointer
                lcd.print("RESET PARAMETER     ");
            }
            else
            {
                screen = OperatingTimeSettings ; // Enter calibration
                lcd.clear();
                inmenu = 1;
            }
        break;
    }
        return;
    } 

    if(usersettings && !inmenu)
    {
        // **************** USER SETTINGS ENTRY *******************
        // Handles ENTER button action when navigating user settings (not inside submenu)
        switch(screen)
        {
            // -------- USER SETTINGS SCREEN 1 --------
            case UserSettingsScreen1:
                if(uppointer)
                {
                    // lcd.clear();
                    screen=SecondaryFillSettings;
                    inmenu=1;
                    if(secondaryyes)
                    {
                        buttonClass_object.setPointer(0,1);
                    }
                    else
                    { 
                        buttonClass_object.setPointer(11,1);
                    }
                }
                else
                {
                    lcd.clear();
                    screen=FlowControlSettings;
                    if(flowoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;
                }
            break;
            
            // -------- USER SETTINGS SCREEN 2 --------
            case UserSettingsScreen2:
                if(downpointer)
                {
                    screen=LevelSensorSettings;
                    lcd.clear();
                     if(leveloverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;
                }
                else
                {
                    lcd.clear();
                    screen=FlowControlSettings;
                    if(flowoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
            break;
            
             // -------- USER SETTINGS SCREEN 3 --------
             case UserSettingsScreen3:
                if(downpointer)
                {
                    lcd.clear();
                    screen=SolenoidControlSettings;
                    if(solenoidoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
                else
                {
                   screen=LevelSensorSettings;
                    lcd.clear();
                     if(leveloverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
            break;

            // -------- USER SETTINGS SCREEN 4 --------
            case UserSettingsScreen4:
                if(downpointer)
                {
                    screen=TempSensorSettings;
                    lcd.clear();
                     if(probeoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;
                }
                else
                {
                    lcd.clear();
                    screen=SolenoidControlSettings;
                    if(solenoidoverride)
                    {
                        lcd.setCursor(0,1);
                        lcd.print("> OVERRIDE"); 
                    }
                    else
                    {
                    lcd.setCursor(0,1);
                    lcd.print("> ACTIVE   ");
                    }
                    inmenu=1;

                }
            break;

            }
            return;
        }
}

// -------- BUTTON CLASS OBJECT --------
// Global instance of buttonClass used across project
buttonClass buttonClass_object = buttonClass();