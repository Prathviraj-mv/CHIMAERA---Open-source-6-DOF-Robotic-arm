#include <AccelStepper.h>
#include <Servo.h>

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

#define A_STEP_PIN    36   // New stepper
#define A_DIR_PIN     34
#define A_ENABLE_PIN  30

#define SERVO1_PIN    11
#define SERVO2_PIN    5    // New servo

#define MAX_SPEED     1600
#define ACCELERATION  800

// Create steppers
AccelStepper stepperX(DRIVER, X_STEP_PIN, X_DIR_PIN); // Elbow
AccelStepper stepperY(DRIVER, Y_STEP_PIN, Y_DIR_PIN); // Shoulder
AccelStepper stepperZ(DRIVER, Z_STEP_PIN, Z_DIR_PIN); // Base
AccelStepper stepperA(DRIVER, A_STEP_PIN, A_DIR_PIN); // New stepper

// Create servos
Servo servo1; // Wrist servo
Servo servo2; // New servo

// Conversion
float degToSteps = 10.0; // Adjust as needed

// Target angles
float targetElbowDeg = 0;
float targetShoulderDeg = 0;
float targetBaseDeg = 0;
float targetAStepperDeg = 0; // New stepper
int targetWristDeg = 90;
int targetServo2Deg = 90;    // New servo

void setup() {
  Serial.begin(9600);

  // Enable drivers
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  pinMode(A_ENABLE_PIN, OUTPUT);

  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);
  digitalWrite(A_ENABLE_PIN, LOW);

  // Stepper setup
  stepperX.setMaxSpeed(MAX_SPEED);
  stepperX.setAcceleration(ACCELERATION);

  stepperY.setMaxSpeed(MAX_SPEED);
  stepperY.setAcceleration(ACCELERATION);

  stepperZ.setMaxSpeed(MAX_SPEED);
  stepperZ.setAcceleration(ACCELERATION);

  stepperA.setMaxSpeed(MAX_SPEED);
  stepperA.setAcceleration(ACCELERATION);

  // Servo setup
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  servo1.write(targetWristDeg);
  servo2.write(targetServo2Deg);

  Serial.println("Ready: Send x(shoulder), y(elbow), v(base), a(stepper4), z(wrist), c(servo2), h(home).");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd.length() == 1 && cmd.charAt(0) == 'h') {
      // Home all
      targetElbowDeg = 0;
      targetShoulderDeg = 0;
      targetBaseDeg = 0;
      targetAStepperDeg = 0;
      targetWristDeg = 90;
      targetServo2Deg = 90;

      stepperX.moveTo(targetElbowDeg * degToSteps);
      stepperY.moveTo((-targetShoulderDeg) * degToSteps);
      stepperZ.moveTo(targetBaseDeg * degToSteps);
      stepperA.moveTo(targetAStepperDeg * degToSteps);

      servo1.write(targetWristDeg);
      servo2.write(targetServo2Deg);

      Serial.println("Homing all axes and servos.");
    }

    else if (cmd.length() > 1) {
      char c = cmd.charAt(0);
      float angle = cmd.substring(1).toFloat();

      switch (c) {
        case 'y': // Elbow
          targetElbowDeg = angle;
          stepperX.moveTo(-targetElbowDeg * degToSteps);
          Serial.print("Elbow target angle: "); Serial.println(targetElbowDeg);
          break;

        case 'v': // Base
          targetBaseDeg = angle;
          stepperZ.moveTo(targetBaseDeg * degToSteps);
          Serial.print("Base target angle: "); Serial.println(targetBaseDeg);
          break;

        case 'x': // Shoulder
          targetShoulderDeg = angle;
          stepperY.moveTo(targetShoulderDeg * degToSteps);
          Serial.print("Shoulder target angle: "); Serial.println(targetShoulderDeg);
          break;

        case 'a': // New stepper
          targetAStepperDeg = angle;
          stepperA.moveTo(targetAStepperDeg * degToSteps);
          Serial.print("Stepper A target angle: "); Serial.println(targetAStepperDeg);
          break;

        case 'z': // Wrist servo
          targetWristDeg = constrain(angle, 0, 180);
          servo1.write(targetWristDeg);
          Serial.print("Wrist servo angle: "); Serial.println(targetWristDeg);
          break;

        case 'c': // New servo
          targetServo2Deg = constrain(angle, 0, 180);
          servo2.write(targetServo2Deg);
          Serial.print("Servo2 angle: "); Serial.println(targetServo2Deg);
          break;

        default:
          Serial.println("Unknown command. Use x, y, v, a, z, c, or h.");
          break;
      }
    }
  }

  // Non-blocking stepper updates
  stepperX.run();
  stepperY.run();
  stepperZ.run();
  stepperA.run();
}
