
#include <AccelStepper.h>

#define DRIVER 1

// RAMPS 1.4 Pins
#define X_STEP_PIN    54
#define X_DIR_PIN     55
#define X_ENABLE_PIN  38

#define Y_STEP_PIN    60
#define Y_DIR_PIN     61
#define Y_ENABLE_PIN  56

#define Z_STEP_PIN    46
#define Z_DIR_PIN     48
#define Z_ENABLE_PIN  62

AccelStepper stepperX(DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(DRIVER, Z_STEP_PIN, Z_DIR_PIN);

void setup() {
  // Enable all motors
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);

  // Smooth motion settings
  const float maxSpeed = 400;
  const float acceleration = 200;

  stepperX.setMaxSpeed(maxSpeed);
  stepperX.setAcceleration(acceleration);
  stepperX.moveTo(800);   // X: 180° CW

  stepperY.setMaxSpeed(maxSpeed);
  stepperY.setAcceleration(acceleration);
  stepperY.moveTo(3200);  // Y: 2 revs CW

  stepperZ.setMaxSpeed(maxSpeed);
  stepperZ.setAcceleration(acceleration);
  stepperZ.moveTo(800);   // Z: 180° CW
}

void loop() {
  stepperX.run();
  stepperY.run();
  stepperZ.run();

  // X: 180° back and forth
  if (stepperX.distanceToGo() == 0)
    stepperX.moveTo(-stepperX.currentPosition());

  // Y: 2 revs back and forth
  if (stepperY.distanceToGo() == 0)
    stepperY.moveTo(-stepperY.currentPosition());

  // Z: 180° back and forth
  if (stepperZ.distanceToGo() == 0)
    stepperZ.moveTo(-stepperZ.currentPosition());
}
