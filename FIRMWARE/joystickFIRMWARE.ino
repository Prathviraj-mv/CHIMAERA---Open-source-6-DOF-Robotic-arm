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

#define SERVO1_PIN    11

#define MAX_SPEED     2000
#define ACCELERATION  1000

// Create steppers
AccelStepper stepperX(DRIVER, X_STEP_PIN, X_DIR_PIN); // Elbow
AccelStepper stepperY(DRIVER, Y_STEP_PIN, Y_DIR_PIN); // Shoulder
AccelStepper stepperZ(DRIVER, Z_STEP_PIN, Z_DIR_PIN); // Base

Servo servo1; // Wrist servo

// Conversion (adjust for your system)
float degToSteps = 10.0; // calibrate based on your gearing

// Target angles
float targetElbowDeg = 0;
float targetShoulderDeg = 0;
float targetBaseDeg = 0;
int targetWristDeg = 90;

void setup() {
  Serial.begin(9600);

  // Enable drivers
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);

  // Stepper setup
  stepperX.setMaxSpeed(MAX_SPEED);
  stepperX.setAcceleration(ACCELERATION);

  stepperY.setMaxSpeed(MAX_SPEED);
  stepperY.setAcceleration(ACCELERATION);

  stepperZ.setMaxSpeed(MAX_SPEED);
  stepperZ.setAcceleration(ACCELERATION);

  // Servo setup
  servo1.attach(SERVO1_PIN);
  servo1.write(targetWristDeg);

  Serial.println("Ready: Send x<angle>(shoulder), y<angle>(elbow), v<angle>(base), z<angle>(wrist), h(home).");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd.length() == 1 && cmd.charAt(0) == 'h') {
      // Home all axes
      targetElbowDeg = 0;
      targetShoulderDeg = 0;
      targetBaseDeg = 0;
      targetWristDeg = 90;

      stepperX.moveTo(targetElbowDeg * degToSteps);
      stepperY.moveTo((-targetShoulderDeg) * degToSteps); // Inverted Y-axis
      stepperZ.moveTo(targetBaseDeg * degToSteps);
      servo1.write(targetWristDeg);

      Serial.println("Homing all axes to 0° and wrist to 90°");
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
          stepperY.moveTo((targetShoulderDeg) * degToSteps); 
          Serial.print("Shoulder target angle (inverted): "); Serial.println(targetShoulderDeg);
          break;

        case 'z': // Wrist
          targetWristDeg = constrain(angle, 0, 180);
          servo1.write(targetWristDeg);
          Serial.print("Wrist servo angle: "); Serial.println(targetWristDeg);
          break;

        default:
          Serial.println("Unknown command. Use x, y, v, z with angle or h to home.");
          break;
      }
    }
  }

  stepperX.run();
  stepperY.run();
  stepperZ.run();
}
