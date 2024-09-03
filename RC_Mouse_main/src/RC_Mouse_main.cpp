#include <M5Core2.h>
#define MOTOR_PIN_F        33 // to DC Motor Driver FIN
#define MOTOR_PIN_R        32  // to DC Motor Driver RIN
#define MOTOR_PWM_F        0  // PWM CHANNEL
#define MOTOR_PWM_R        1  // PWM CHANNEL

void setup() {
  // set Grove into GPIO32/33
  M5.begin(true, true, true, false);

  //PWM Init
  pinMode(MOTOR_PIN_F, OUTPUT);
  pinMode(MOTOR_PIN_R, OUTPUT);
  ledcSetup(MOTOR_PWM_F, 490, 8); //CHANNEL, FREQ, BIT
  ledcSetup(MOTOR_PWM_R, 490, 8);
  ledcAttachPin(MOTOR_PIN_F, MOTOR_PWM_F);
  ledcAttachPin(MOTOR_PIN_R, MOTOR_PWM_R);
}

void loop(){
  ledcWrite(MOTOR_PWM_F,255); // Forward
  ledcWrite(MOTOR_PWM_R,0);
  delay(3000);

  ledcWrite(MOTOR_PWM_R,0); // Stop
  ledcWrite(MOTOR_PWM_F,0);
  delay(3000);

  ledcWrite(MOTOR_PWM_R,255); // Reverse
  ledcWrite(MOTOR_PWM_F,0);
  delay(3000);
}
