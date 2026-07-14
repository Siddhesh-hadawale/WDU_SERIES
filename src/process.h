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

    void water_level_detection();
    void parameter_test();
    void Check_ticker_stop();

    void Solenoid1_start();
    void Solenoid2_start();
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
