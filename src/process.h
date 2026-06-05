#ifndef PROCESS_H
#define PROCESS_H

class process
{
    public:
    process();
    void process_start();
    void boiler_preheat();
    void error_check();
    void process_setup();
    void variant_settings();
    int calculate_calibration();
    int calculate_dryout_time();
    void secondary_fill();
    void dryout_fill();
    void process_stop();
    void heater1_start();
    void heater1_stop();
    void heater2_start();
    void heater2_stop();    
    // void water_flow_detection();
    void water_level_detection();
    void parameter_test();
    void Check_ticker_stop();

    void Solenoid1_stop();
    void Solenoid2_stop();

    void Contactor1_start();
    void Contactor1_stop();
    void Contactor2_start();    
    void Contactor2_stop();
    
    void ticker_update();

};

extern process process_object;

#endif
