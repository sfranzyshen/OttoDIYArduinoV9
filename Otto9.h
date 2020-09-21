// OttoDIY version 0.3.9

#include <Servo.h>
#include <Oscillator.h>
#include <EEPROM.h>

//-- Constants
#define FORWARD     1
#define BACKWARD    -1
#define LEFT        1
#define RIGHT       -1
#define SMALL       5
#define MEDIUM      15
#define BIG         30

class Otto9 {
  public:
    // Otto initialization
    void init(int YL, int YR, int RL, int RR, bool load_calibration, int NoiseSensor, int Buzzer, int USTrigger, int USEcho);
    // Attach & detach functions
    void attachServos();
    void detachServos();
    // Oscillator Trims
    void setTrims(int YL, int YR, int RL, int RR);
    void saveTrimsOnEEPROM();
    // Predetermined Motion Functions
    void moveServos(int time, int servo_target[]);
    void moveSingle(int position,int servo_number);
    void oscillateServos(int A[4], int O[4], int T, double phase_diff[4], float cycle = 1);
    // HOME = Otto at rest position
    void home();
    bool getRestState();
    void setRestState(bool state);
    // Predetermined Motion Functions
    void jump(float steps = 1, int T = 2000);
    void walk(float steps = 4, int T = 1000, int dir = FORWARD);
    void turn(float steps = 4, int T = 2000, int dir = LEFT);
    void bend(int steps = 1, int T = 1400, int dir = LEFT);
    void shakeLeg(int steps = 1, int T = 2000, int dir = RIGHT);
    void updown(float steps = 1, int T = 1000, int h = 20);
    void swing(float steps = 1, int T = 1000, int h = 20);
    void tiptoeSwing(float steps = 1, int T = 900, int h = 20);
    void jitter(float steps = 1, int T = 500, int h = 20);
    void ascendingTurn(float steps = 1, int T = 900, int h = 20);
    void moonwalker(float steps = 1, int T = 900, int h = 20, int dir = LEFT);
    void crusaito(float steps = 1, int T = 900, int h = 20, int dir = FORWARD);
    void flapping(float steps = 1, int T = 1000, int h = 20, int dir = FORWARD);
  private:
    Oscillator _servo[4];
    int _servo_pins[4];
    int _servo_trim[4];
    int _servo_position[4];
    unsigned long _final_time;
    unsigned long _partial_time;
    float _increment[4];
    bool _isOttoResting;
    void _execute(int A[4], int O[4], int T, double phase_diff[4], float steps = 1.0);
};

