#ifndef EXT_VAR_H
#define EXT_VAR_H


#include <LiquidCrystal_I2C.h>

#include "Lcd_Display.h"
#include <OneButton.h>
#include <EEPROM.h>
#include "Ticker.h"
#include <Wire.h>

#include "Button.h"
#include "Lcd_Display.h"
#include "Buzzer.h"
#include "process.h"
#include "RTD.h"
#include "savememory.h"

#define BUZZER 18
#define SOLENOID1 13
#define SOLENOID2 12

// EEPROM Addresses

#define PRODUCT_SELECTION 0
#define SUBPRODUCT_SELECTION 1
#define SAFETY_TEMP 3
#define CALIBRATION_VALUE 5
#define PROBE_ERROR 9

#define SECONDARY_FILL 15
#define FLOW_CONTROL 17
#define LEVEL_CONTROL 19
#define SOLENOID_CONTROL 21
#define PROBE_CONTROL 23
#define SECONDARY_FILL_TIME 25
// #define OPERATING_TIME 27
#define EEPROM_OPERATING_TIME_ADDR 27




// #define OPERATING_TIME 4   // Base operating time factor used for max volume calculation
#define OPERATING_TIME_FACTOR 4
#define POWER_DETECTION 5

#define PROBE_ERROR_TEMP      255.0

#define MAX_SAFETY_TEMP 200          // Maximum allowed heater safety temperature
#define MIN_SAFETY_TEMP 50           // Minimum allowed heater safety temperature
#define SAFETY_TEMP_STEP 1           // Step increment/decrement for safety temperature
#define PROBE_CALIBRATION_LIMIT 20   // Max +/- limit for probe calibration error
#define SECONDARY_FILL_TIME_LIMIT  60 // Max limit for secondary fill time
#define SECONDARY_FILL_TIME_LIMIT_LOWER  15

extern int prodtype[3];
extern int  optime[3];
extern float base_calibration[3];
extern int Sfill_default[3];
extern int sfill_time;



extern bool toggle;
extern bool usersettings;
extern bool servicemenu;
extern bool mainscreenflag;
extern bool uppointer;
extern bool downpointer;
extern bool inmenu;
extern bool secondaryyes;
extern bool solenoidoverride;
extern bool flowoverride;
extern int one_second_counter;
extern bool process_flag;
extern bool flow_error_checkflag;
extern bool error_check_flag;
extern bool secondarytimerflag;
extern bool pauseflag;
extern bool waterlevel_error_flag;
extern bool dduflag;
extern bool factoryresetflag;
extern bool leveloverride;
extern bool probeoverride;
extern bool closetap;
extern bool  temp_drop_flag;
extern bool time_skip;
extern bool primary_filling_flag;
// extern bool zero_calib;
extern bool heater_start;
extern bool preheat_flag;
extern bool testsettings;
extern bool Secodaryfill_error_flag;
extern bool override_alert_flag;
extern bool check_flag;
extern bool dryout_flag;      

extern float calib_Heater1;
extern float temp_error;
extern float Max_liter;
extern int optimecounter;
extern int prodtypecounter;
extern float calibration_value;
extern float counter;
extern int screen;
extern int screen2;
extern float Heater_temp;
extern int variant;
extern int pre_end_time;
extern int end_time;
extern uint16_t mint;
extern uint8_t Probe1_Err;
extern int remaining_volume;
extern int u_input;
extern float time_per_step;
extern int previous_time;
extern int Heatersafteytemp;
extern unsigned long solenoid_stop;

extern uint8_t skip_count;
extern uint16_t time_counter2;
extern int one_second_counter2;


enum menus
{
    VersionScreen,
    MainScreen,

    ServiceMenuScreen1,
    ServiceMenuScreen2,
    ServiceMenuScreen3,
    ServiceMenuScreen4,
    ServiceMenuScreen5,
    ServiceMenuScreen6,
    ServiceMenuScreen7,
    SDUServiceMenuScreen4,

    UserSettingsScreen1,
    UserSettingsScreen2,
    UserSettingsScreen3,
    UserSettingsScreen4,
    // UserSettingsScreen5,
    // UserSettingsScreen6,
    // UserSettingsScreen7,
    // UserSettingsScreen8,

    LevelSensorSettings,
    TempSensorSettings,

    
    OperatingTimeSettings,
    ProductTypeSettings,
    SubProductTypeSettings,
    CalibrationSettings,
    SolenoidControlSettings,
    FlowControlSettings,

    SecondaryFillSettings,
    SafteyTemperatureSettings,
    ProbeCalibrationSettings,
    TimeFactorSettings,

    FactoryResetScreen,

    ProcessScreen,
    PrimaryFillScreen,
    SecondaryFillTimer,
    SecondaryFillCheck,

    TestingScreen,
    
    // SolenoidErrorScreen,
    ErrorScreen
};

enum test_menus
{
    TestMenu1,
    TestMenu2,
    FlowSensorTest,
    LevelSensorTest,
    PT100ProbeTest,
    //TestMenu3,
};

#endif