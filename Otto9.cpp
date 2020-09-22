// OttoDIY version 0.3.9

#include "Arduino.h"
#include "Otto9.h"

void Otto9::init(int YL, int YR, int RL, int RR, bool load_calibration, int NoiseSensor, int Buzzer, int USTrigger, int USEcho) {
  _servo_pins[0] = YL;
  _servo_pins[1] = YR;
  _servo_pins[2] = RL;
  _servo_pins[3] = RR;
  attachServos();
  setRestState(false);
  if(load_calibration) {
    for(int i = 0; i < 4; i++) {
      int servo_trim = EEPROM.read(i);
      if(servo_trim > 128) servo_trim -= 256;
      _servo[i].setTrim(servo_trim);
    }
  }
  for(int i = 0; i < 4; i++) {
    _servo_position[i] = 90;
  }
}

// ATTACH & DETACH FUNCTIONS
void Otto9::attachServos() {
  for(int i = 0; i < 4; i++) {
    _servo[i].attach(_servo_pins[i]);
  }
}

void Otto9::detachServos() {
  for(int i = 0; i < 4; i++) {
    _servo[i].detach();
  }
}

// OSCILLATORS TRIMS
void Otto9::setTrims(int YL, int YR, int RL, int RR) {
  _servo[0].setTrim(YL);
  _servo[1].setTrim(YR);
  _servo[2].setTrim(RL);
  _servo[3].setTrim(RR);
}

void Otto9::saveTrimsOnEEPROM() {
  for(int i = 0; i < 4; i++) { 
    EEPROM.write(i, _servo[i].getTrim());
  } 
}

// BASIC MOTION FUNCTIONS
void Otto9::moveServos(int time, int  servo_target[]) {
  attachServos();
  if(getRestState()==true){
    setRestState(false);
  }
  if(time > 10) {
    for(int i = 0; i < 4; i++) 
      _increment[i] = ((servo_target[i]) - _servo_position[i]) / (time / 10.0);
    _final_time =  millis() + time;

    for(int iteration = 1; millis() < _final_time; iteration++) {
      _partial_time = millis() + 10;
      for(int i = 0; i < 4; i++) {
        _servo[i].setPosition(_servo_position[i] + (iteration * _increment[i]));
      }
      while(millis() < _partial_time) {
        //pause
      }
    }
  } else {
    for(int i = 0; i < 4; i++) {
      _servo[i].setPosition(servo_target[i]);
    }
  }
  for(int i = 0; i < 4; i++) { 
    _servo_position[i] = servo_target[i];
  }
}

void Otto9::moveSingle(int position, int servo_number) {
  if(position > 180) 
    position = 180;
  if(position < 0) 
    position = 0;
  attachServos();
  if(getRestState() == true) {
    setRestState(false);
  }
  _servo[servo_number].setPosition(position);
}

void Otto9::oscillateServos(int A[], int O[], int T, double phase_diff[], float cycle) {
  for(int i = 0; i < 4; i++) {
    _servo[i].setO(O[i]);
    _servo[i].setA(A[i]);
    _servo[i].setT(T);
    _servo[i].setPh(phase_diff[i]);
  }
  double ref = millis();
  for(double x = ref; x <= T * cycle + ref; x = millis()) {
    for(int i = 0; i < 4; i++) {
      _servo[i].refresh();
    }
  }
}

void Otto9::_execute(int A[], int O[], int T, double phase_diff[], float steps) {
  attachServos();
  if(getRestState() == true) {
    setRestState(false);
  }
  int cycles = (int)steps;
  if(cycles >= 1) {   // Execute complete cycles
    for(int i = 0; i < cycles; i++) {
      oscillateServos(A, O, T, phase_diff);
    }
  }     
  oscillateServos(A, O, T, phase_diff, (float)steps - cycles); // Execute the final not complete cycle    
}

// HOME = Otto at rest position
void Otto9::home() {
  if(getRestState() == false) { // Go to rest position only if necessary
    int homes[] = {90, 90, 90, 90}; //All the servos at rest position
    moveServos(500, homes); //Move the servos in half a second
    detachServos();
    setRestState(true);
  }
}

bool Otto9::getRestState() {
  return _isOttoResting;
}

void Otto9::setRestState(bool state) {
    _isOttoResting = state;
}

// PREDETERMINED MOTION SEQUENCES

// Otto movement: Jump
//  Parameters:
//    steps: Number of steps
//    T: Period
void Otto9::jump(float steps, int T) {
  int up[] = {90, 90, 150, 30};
  moveServos(T, up);
  int down[] = {90, 90, 90, 90};
  moveServos(T, down);
}

// Otto gait: Walking  (forward or backward)    
//  Parameters:
//    * steps:  Number of steps
//    * T : Period
//    * Dir: Direction: FORWARD / BACKWARD
void Otto9::walk(float steps, int T, int dir) {
  // Oscillator parameters for walking
  // Hip sevos are in phase
  // Feet servos are in phase
  // Hip and feet are 90 degrees out of phase
  //      -90 : Walk forward
  //       90 : Walk backward
  // Feet servos also have the same offset (for tiptoe a little bit)
  int A[] = {30, 30, 20, 20};
  int O[] = {0, 0, 4, -4};
  double phase_diff[] = {0, 0, DEG2RAD(dir * -90), DEG2RAD(dir * -90)};

  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps);  
}

// Otto gait: Turning (left or right)
//  Parameters:
//   * Steps: Number of steps
//   * T: Period
//   * Dir: Direction: LEFT / RIGHT
void Otto9::turn(float steps, int T, int dir) {
  // Same coordination than for walking (see Otto::walk)
  // The Amplitudes of the hip's oscillators are not igual
  // When the right hip servo amplitude is higher, the steps taken by
  //   the right leg are bigger than the left. So, the robot describes an left arc
  int A[] = {30, 30, 20, 20};
  int O[4] = {0, 0, 4, -4};
  double phase_diff[4] = {0, 0, DEG2RAD(-90), DEG2RAD(-90)}; 
  if(dir == LEFT) {  
    A[0] = 30; // Left hip servo
    A[1] = 10; // Right hip servo
  } else {
    A[0] = 10;
    A[1] = 30;
  }
  // Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}

// Otto gait: Lateral bend
//  Parameters:
//    steps: Number of bends
//    T: Period of one bend
//    dir: RIGHT=Right bend LEFT=Left bend
void Otto9::bend(int steps, int T, int dir) {
  // Parameters of all the movements. Default: Left bend
  int bend1[] = {90, 90, 62, 35}; 
  int bend2[] = {90, 90, 62, 105};
  int homes[] = {90, 90, 90, 90};
  // Time of one bend, constrained in order to avoid movements too fast.
  //T=max(T, 600);
  if(dir == RIGHT) { // Changes in the parameters if right direction is chosen 
    bend1[2] = 145;
    bend1[3] = 120;
    bend2[2] = 75;
    bend2[3] = 120;
  }
  // Time of the bend movement. Fixed parameter to avoid falls
  int T2 = 800; 
  //Bend movement
  for(int i=0; i < steps; i++) {
    moveServos(T2 / 2, bend1);
    moveServos(T2 / 2, bend2);
    delay(T * 0.8); // FIXME 
    moveServos(500, homes);
  }

}

// Otto gait: Shake a leg
//  Parameters:
//    steps: Number of shakes
//    T: Period of one shake
//    dir: RIGHT=Right leg LEFT=Left leg
void Otto9::shakeLeg(int steps, int T,int dir) {
  int numberLegMoves = 2; // This variable change the amount of shakes
  // Parameters of all the movements. Default: Left leg
  int shake_leg1[] = {90, 90, 58, 35};   
  int shake_leg2[] = {90, 90, 58, 120};
  int shake_leg3[] = {90, 90, 58, 60};
  int homes[] = {90, 90, 90, 90};
  // Changes in the parameters if right leg is chosen
  if(dir == RIGHT) {
    shake_leg1[2] = 145;
    shake_leg1[3] = 122;
    shake_leg2[2] = 60;
    shake_leg2[3] = 122;
    shake_leg3[2] = 120;
    shake_leg3[3] = 122;
  }
  // Time of the bend movement. Fixed parameter to avoid falls
  int T2 = 1000;    
  // Time of one shake, constrained in order to avoid movements too fast.            
  T = T - T2;
  T = max(T, 200 * numberLegMoves);  
  for(int j = 0; j < steps; j++) {
    // Bend movement
    moveServos(T2 / 2, shake_leg1);
    moveServos(T2 / 2, shake_leg2);
    // Shake movement
    for(int i = 0; i < numberLegMoves; i++) {
      moveServos(T / (2 * numberLegMoves), shake_leg3);
      moveServos(T / (2 * numberLegMoves), shake_leg2);
    }
    moveServos(500, homes); //Return to home position
  }
  delay(T);
}

// Otto movement: up & down
//  Parameters:
//    * steps: Number of jumps
//    * T: Period
//    * h: Jump height: SMALL / MEDIUM / BIG 
//              (or a number in degrees 0 - 90)
void Otto9::updown(float steps, int T, int h) {
  // Both feet are 180 degrees out of phase
  // Feet amplitude and offset are the same
  // Initial phase for the right foot is -90, so that it starts
  //   in one extreme position (not in the middle)
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h, -h};
  double phase_diff[4] = {0, 0, DEG2RAD(-90), DEG2RAD(90)};
  
  //-- Let's oscillate the servos!
  //_execute(A, O, T, phase_diff, steps); 
  _execute(A, O, T, phase_diff, steps); 
}

// Otto movement: swinging side to side
//  Parameters:
//     steps: Number of steps
//     T : Period
//     h : Amount of swing (from 0 to 50 aprox)
void Otto9::swing(float steps, int T, int h) {

  // Both feets are in phase. The offset is half the amplitude
  // It causes the robot to swing from side to side
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h/2, -h/2};
  double phase_diff[4] = {0, 0, DEG2RAD(0), DEG2RAD(0)};
  
  // Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}

// Otto movement: swinging side to side without touching the floor with the heel
//  Parameters:
//     steps: Number of steps
//     T : Period
//     h : Amount of swing (from 0 to 50 aprox)
void Otto9::tiptoeSwing(float steps, int T, int h) {
  // Both feets are in phase. The offset is not half the amplitude in order to tiptoe
  // It causes the robot to swing from side to side
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h, -h};
  double phase_diff[4] = {0, 0, 0, 0};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}

// Otto gait: Jitter 
//  Parameters:
//    steps: Number of jitters
//    T: Period of one jitter 
//    h: height (Values between 5 - 25)   
void Otto9::jitter(float steps, int T, int h) {
  // Both feet are 180 degrees out of phase
  // Feet amplitude and offset are the same
  // Initial phase for the right foot is -90, so that it starts
  //   in one extreme position (not in the middle)
  // h is constrained to avoid hit the feets
  h = min(25, h);
  int A[4]= {h, h, 0, 0};
  int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(-90), DEG2RAD(90), 0, 0};
  
  // Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}

// Otto gait: Ascending & turn (Jitter while up&down)
//  Parameters:
//    steps: Number of bends
//    T: Period of one bend
//    h: height (Values between 5 - 15) 
void Otto9::ascendingTurn(float steps, int T, int h) {
  // Both feet and legs are 180 degrees out of phase
  // Initial phase for the right foot is -90, so that it starts
  //   in one extreme position (not in the middle)
  // h is constrained to avoid hit the feets
  h = min(13, h);
  int A[4]= {h, h, h, h};
  int O[4] = {0, 0, h+4, -h+4};
  double phase_diff[4] = {DEG2RAD(-90), DEG2RAD(90), DEG2RAD(-90), DEG2RAD(90)};
  
  // Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}

// Otto gait: Moonwalker. Otto moves like Michael Jackson
//  Parameters:
//    Steps: Number of steps
//    T: Period
//    h: Height. Typical valures between 15 and 40
//    dir: Direction: LEFT / RIGHT
void Otto9::moonwalker(float steps, int T, int h, int dir) {
  // This motion is similar to that of the caterpillar robots: A travelling
  // wave moving from one side to another
  // The two Otto's feet are equivalent to a minimal configuration. It is known
  // that 2 servos can move like a worm if they are 120 degrees out of phase
  // In the example of Otto, the two feet are mirrored so that we have:
  //    180 - 120 = 60 degrees. The actual phase difference given to the oscillators
  //  is 60 degrees.
  //  Both amplitudes are equal. The offset is half the amplitud plus a little bit of
  //  offset so that the robot tiptoe lightly
 
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h/2+2, -h/2 -2};
  int phi = (-dir * 90);
  double phase_diff[4] = {0, 0, DEG2RAD(phi), DEG2RAD(-60 * dir + phi)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}

// Otto gait: Crusaito. A mixture between moonwalker and walk
//   Parameters:
//     steps: Number of steps
//     T: Period
//     h: height (Values between 20 - 50)
//     dir:  Direction: LEFT / RIGHT
void Otto9::crusaito(float steps, int T, int h, int dir) {
  int A[4]= {25, 25, h, h};
  int O[4] = {0, 0, h/2+ 4, -h/2 - 4};
  double phase_diff[4] = {90, 90, DEG2RAD(0), DEG2RAD(-60 * dir)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps);
}

// Otto gait: Flapping
//  Parameters:
//    steps: Number of steps
//    T: Period
//    h: height (Values between 10 - 30)
//    dir: direction: FOREWARD, BACKWARD
void Otto9::flapping(float steps, int T, int h, int dir) {
  int A[4]= {12, 12, h, h};
  int O[4] = {0, 0, h - 10, -h + 10};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(180), DEG2RAD(-90 * dir), DEG2RAD(90 * dir)};
  
  // Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps);
}

