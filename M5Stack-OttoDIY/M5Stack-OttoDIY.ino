#include <M5Stack.h>
#include <M5StackUpdater.h>
#include <Wire.h>
#include "src/Oscillator/Oscillator.h"
#include <Avatar.h>

using namespace m5avatar;


//---------------------------------
// Servo Settings
//---------------------------------

#define N_SERVOS 5

#define INTERVALTIME 10.0

int t = 620;  // 1280;
double pausemillis = 0;
// ch0:RightFoot,ch1:LeftFoot,ch2:RightLeg,ch3:LeftLeg
// ch4:NeckV
int servoMin[N_SERVOS]  = {150, 150, 150, 150, 150};
int servoMax[N_SERVOS]  = {500, 500, 500, 500, 500};
int servoTrim[N_SERVOS] = {10, 0, 0, -10, -10};
int servoInit[N_SERVOS] = {90, 90, 90, 90, 125};
int servoTest[N_SERVOS] = {80, 80, 80, 80, 115};

#define NECKV_CH 4

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
Oscillator *servo[N_SERVOS];

bool maintenanceMode = true;
//---------------------------------

//---------------------------------
// Avatar Settings
//---------------------------------
Avatar avatar;
Face* face;

const Expression expressions[] = {
  Expression::Angry,
  Expression::Sleepy,
  Expression::Happy,
  Expression::Sad,
  Expression::Doubt,
  Expression::Neutral
};
const int expressionsSize = sizeof(expressions) / sizeof(Expression);
int idx = 0;

ColorPalette* cps[4];
const int cpsSize = sizeof(cps) / sizeof(ColorPalette*);
int cpsIdx = 0;



void setup() {
  M5.begin();
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Start");
  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  Serial.println("Lcdinit");
  M5.Lcd.init();
  M5.Lcd.fillScreen(TFT_WHITE);
  pwm.begin();
  pwm.setPWMFreq(50);
  // put your setup code here, to run once:
  for (int i = 0; i < N_SERVOS; i ++) {
    servo[i] = new Oscillator(&pwm, i, servoMin[i], servoMax[i], servoTrim[i]);
  }
  
  Serial.println("Servo init");
  for (int i = 0; i < N_SERVOS; i++) {
    servo[i]->setPosition(servoTest[i]);
  }
  delay(1000);
  for (int i = 0; i < N_SERVOS; i++) {
    servo[i]->setPosition(servoInit[i]);
  }

  Serial.println("getFace");
  face = avatar.getFace();
  cps[0] = new ColorPalette();
  cps[1] = new ColorPalette();
  cps[2] = new ColorPalette();
  cps[3] = new ColorPalette();
  cps[1]->set(COLOR_PRIMARY, TFT_YELLOW);
  cps[1]->set(COLOR_BACKGROUND, TFT_DARKCYAN);
  cps[2]->set(COLOR_PRIMARY, TFT_DARKGREY);
  cps[2]->set(COLOR_BACKGROUND, TFT_WHITE);
  cps[3]->set(COLOR_PRIMARY, TFT_RED);
  cps[3]->set(COLOR_BACKGROUND, TFT_PINK);

  Serial.println("Avatar init");
  avatar.init();
  avatar.setColorPalette(*cps[2]);
  avatar.setExpression(expressions[1]);
  pwm.sleep();
}


void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    pwm.wakeup();
    action1();
    servo[NECKV_CH]->setPosition(servoInit[NECKV_CH]);
    avatar.setExpression(expressions[1]);
    delay(1000);
    pwm.sleep();
  }
  if (M5.BtnB.wasPressed()) {
    pwm.wakeup();
    avatar.setColorPalette(*cps[cpsIdx]);
    cpsIdx = (cpsIdx + 1) % cpsSize;
    action2();
    delay(1000);
    pwm.sleep();
  }
  if (M5.BtnC.wasPressed()) {
    // Run Mode <==> Maintenance MOde
    pwm.wakeup();
    if (maintenanceMode) {
      servoInit[NECKV_CH] = 45;      
      maintenanceMode = false;
    } else {
      servoInit[NECKV_CH] = 135;      
      maintenanceMode = true;
    }
    servo[NECKV_CH]->setPosition(servoInit[NECKV_CH]);
    avatar.setExpression(expressions[1]);
    delay(1000);
    pwm.sleep();
  }
  delay(20);
}
//-----------------------------
// Move Actions
void action1() {
  avatar.setExpression(expressions[5]);
  delay(3000);
  displayStatus("Walk");
  walk(2, t * 3);
  displayStatus("Back");
  backyard(2, t * 3);
  avatar.setExpression(expressions[2]);
  displayStatus("upDown");
  upDown(3, t * 2);
  displayStatus("MWalkR");
  moonWalkRight(1, t*2);
  moonWalkRight(2, t);
  displayStatus("MWalkL");
  moonWalkLeft(1, t*2);
  moonWalkLeft(2, t);
  displayStatus("swing");
  swing(2, t);
}

void action2() {
  avatar.setColorPalette(*cps[cpsIdx]);
  cpsIdx = (cpsIdx + 1) % cpsSize;
  
  headSwing(2, t * 4);
}

void action3() {
  headSwing2(2, t * 4);
}
void displayStatus(String param) {
/*
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 160);
  M5.Lcd.print(param + "    ");
*/
}

//-----------------------------
void oscillate(int A[N_SERVOS], int O[N_SERVOS], int T, double phase_diff[N_SERVOS]) {
  for (int i = 0; i < N_SERVOS; i++) {
    servo[i]->setO(O[i]);
    servo[i]->setA(A[i]);
    servo[i]->setT(T);
    servo[i]->setPh(phase_diff[i]);
  }


  double ref = millis();
  for (double x = ref; x < T + ref; x = millis()) {
    for (int i = 0; i < N_SERVOS; i++) {
      servo[i]->refresh();
    }
  }

//  for (int i=0; i< N_SERVOS; i++) {
//    servo[i]->reset();
//    servo[i]->setPosition(90);
//  }

}

unsigned long final_time;
unsigned long interval_time;
int oneTime;
int iteration;
float increment[N_SERVOS];
int oldPosition[N_SERVOS] = {90, 90, 90, 90, 90};

void moveNServos(int time, int  newPosition[]) {
  for (int i = 0; i < N_SERVOS; i++)  increment[i] = ((newPosition[i]) - oldPosition[i]) / (time / INTERVALTIME);

  final_time =  millis() + time;

  iteration = 1;
  while (millis() < final_time) { //Javi del futuro cambia esto
    interval_time = millis() + INTERVALTIME;

    oneTime = 0;
    while (millis() < interval_time) {
      if (oneTime < 1) {
        for (int i = 0; i < N_SERVOS; i++) {
          servo[i]->setPosition(oldPosition[i] + (iteration * increment[i]));
        }
        iteration++;
        oneTime++;
      }
    }
  }

  for (int i = 0; i < N_SERVOS; i++) {
    oldPosition[i] = newPosition[i];
  }
}
//-------------------------------------------------
// Motions
//-------------------------------------------------
void walk(int steps, int T) {
  int A[N_SERVOS] = { 15, 15, 30, 30, 25};
  int O[N_SERVOS] = { 0, 0, 0, 0, 0};
  double phase_diff[N_SERVOS] = { DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90), DEG2RAD(135) };

  for (int i = 0; i < steps; i++) {
    oscillate(A, O, T, phase_diff);
  }
}

void backyard(int steps, int T) {
  int A[N_SERVOS] = {15, 15, 30, 30, 25};
  int O[N_SERVOS] = {0, 0, 0, 0, 0};
  double phase_diff[N_SERVOS] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(-90), DEG2RAD(-90), DEG2RAD(135)};

  for (int i = 0; i < steps; i++)oscillate(A, O, T, phase_diff);
}


void upDown(int steps, int tempo) {
  int move1[N_SERVOS] = {50, 130, 90, 90, 115};
  int move2[N_SERVOS] = {90, 90, 90, 90, 90};

  for (int x = 0; x < steps; x++) {
    pausemillis = millis();
    moveNServos(tempo * 0.2, move1);
    delay(tempo * 0.4);
    moveNServos(tempo * 0.2, move2);
    while (millis() < (pausemillis + tempo));
  }
}

void moonWalkRight(int steps, int T) {
  Serial.println("moonWalkRight");
  int A[N_SERVOS] = {15, 15, 0, 0, 0};
  int O[N_SERVOS] = {5 , 5, 0, 0, 0};
  double phase_diff[N_SERVOS] = {DEG2RAD(0), DEG2RAD(180 + 120), DEG2RAD(90), DEG2RAD(90), DEG2RAD(90)};

  for (int i = 0; i < steps; i++)oscillate(A, O, T, phase_diff);
}

void moonWalkLeft(int steps, int T) {
  Serial.println("moonWalkLeft");
  int A[N_SERVOS] = {15, 15, 0, 0, 0};
  int O[N_SERVOS] = {5, 5, 0, 0, 0};
  double phase_diff[6] = {DEG2RAD(0), DEG2RAD(180 - 120), DEG2RAD(90), DEG2RAD(90), DEG2RAD(90)};

  for (int i = 0; i < steps; i++)oscillate(A, O, T, phase_diff);
}

void swing(int steps, int T) {
  int A[N_SERVOS] = {25, 25, 0, 0, 0};
  int O[N_SERVOS] = {15, 15, 0, 0, 0};
  double phase_diff[N_SERVOS] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90), DEG2RAD(90)};

  for (int i = 0; i < steps; i++)oscillate(A, O, T, phase_diff);
}

void headSwing(int steps, int tempo) {
  int move1[N_SERVOS] = {90, 90, 90, 90, 90};
  int move2[N_SERVOS] = {90, 90, 90, 90, 45};

  for (int x = 0; x < steps; x++) {
    pausemillis = millis();
    moveNServos(tempo * 0.2, move1);
    delay(tempo * 0.4);
    moveNServos(tempo * 0.2, move2);
    while (millis() < (pausemillis + tempo));
  }
}

void headSwing2(int steps, int T) {
  int A[N_SERVOS] = {0, 0, 0, 0, 45};
  int O[N_SERVOS] = {0, 0, 0, 0, 0};
  double phase_diff[N_SERVOS] = {DEG2RAD(90), DEG2RAD(90), DEG2RAD(90), DEG2RAD(90), DEG2RAD(135) };

  for (int i = 0; i < steps; i++)oscillate(A, O, T, phase_diff);
}
