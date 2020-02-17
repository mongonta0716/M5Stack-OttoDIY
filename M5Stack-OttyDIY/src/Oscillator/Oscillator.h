//--------------------------------------------------------------
//-- Oscillator.pde
//-- Generate sinusoidal oscillations in the servos
//--------------------------------------------------------------
//-- (c) Juan Gonzalez-Gomez (Obijuan), Dec 2011
//-- GPL license
//--------------------------------------------------------------
#ifndef Oscillator_h
#define Oscillator_h

#include <Adafruit_PWMServoDriver.h>

//-- Macro for converting from degrees to radians
#ifndef DEG2RAD
  #define DEG2RAD(g) ((g)*M_PI)/180
#endif


class Oscillator
{
  public:
    Oscillator();
    Oscillator(Adafruit_PWMServoDriver *pwm, int ch, int servomin, int servomax, int trim=0);
    void attach(bool rev =false);
    void detach();
    
    void setA(int A) {_A=A;};
    void setO(int O) {_O=O;};
    void setPh(double Ph) {_phase0=Ph;};
    void setT(unsigned int T);
    void setTrim(int trim){_trim=trim;};
    int  getTrim() {return _trim;};
    void setPosition(int position); 
    void stop() {_stop=true;};
    void play() {_stop=false;};
    void reset() {_phase=0;};
    void refresh();
    void servo_write(int ang);
    
  private:
    bool next_sample();  
    
  private:
    //-- Servo that is attached to the oscillator
    
    //-- Oscillators parameters
    unsigned int _A;  //-- Amplitude (degrees)
    unsigned int _O;  //-- Offset (degrees)
    unsigned int _T;  //-- Period (miliseconds)
    double _phase0;   //-- Phase (radians)
    
    //-- Internal variables
    int _ch;
    int _pos;         //-- Current servo pos
    int _trim;        //-- Calibration offset
    double _phase;    //-- Current phase
    double _inc;      //-- Increment of phase
    double _NN;        //-- Number of samples
    unsigned int _TS; //-- sampling period (ms)
    
    long _previousMillis; 
    long _currentMillis;
    
    //-- Oscillation mode. If true, the servo is stopped
    bool _stop;

    //-- Reverse mode
    bool _rev;

    //-- 
    int _servomin;
    int _servomax;

    Adafruit_PWMServoDriver *_pwm;
};

#endif
