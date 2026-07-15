#ifndef DISPLAY_H
#define DISPLAY_H

class LCD
{
    public:
    void lcd_setup();
    void version_dis();
    void disp_pt100();
    void disp_level_sensor();
    void disp_flow_sensor();
};

#define Pt100_1_pin 27


#define water_detection_pin 4
#define power_detection_pin 5
#define flow_pin 24

#define contactor1 10
#define contactor2 11
#define solenoid1 13
#define solenoid2 12
#define heater1 15
#define heater2 14

void level_s();
void flow_s();

#endif