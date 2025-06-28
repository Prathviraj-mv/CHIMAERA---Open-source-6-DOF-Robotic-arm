#include <AccelStepper.h>

// === CONFIGURATION ===
#define L1 23.0
#define L2 17.5

#define DRIVER 1

#define SHOULDER_STEP_PIN 54
#define SHOULDER_DIR_PIN  55
#define SHOULDER_ENABLE_PIN 38

#define ELBOW_STEP_PIN 60
#define ELBOW_DIR_PIN  61
#define ELBOW_ENABLE_PIN 56

#define STEPS_PER_REV 200
#define MICROSTEPPING 8
#define GEAR_RATIO 5.0

#define MAX_SPEED 2000
#define ACCELERATION 1000

// === JOINT CONSTRAINTS ===
float SHOULDER_MIN_ANGLE = 75.0;
float SHOULDER_MAX_ANGLE = 135.0;
float ELBOW_MIN_ANGLE = 0.0;
float ELBOW_MAX_ANGLE = 75.0;

// === STATE ===
AccelStepper stepperShoulder(DRIVER, SHOULDER_STEP_PIN, SHOULDER_DIR_PIN);
AccelStepper stepperElbow(DRIVER, ELBOW_STEP_PIN, ELBOW_DIR_PIN);

float shoulderAngleCurrent = 135.0; // up
float elbowAngleCurrent = 0.0;      // straight

// === FUNCTIONS ===
long angleToStepsAbs(float angle_deg) {
    float stepsPerMotorRev = STEPS_PER_REV * MICROSTEPPING;
    float stepsPerDegree = stepsPerMotorRev / 360.0;
    float requiredMotorAngle = angle_deg * GEAR_RATIO;
    return requiredMotorAngle * stepsPerDegree;
}

void moveToAngles(float theta1_deg, float theta2_deg) {
    // Clip angles directly to constraints
    if (theta1_deg > SHOULDER_MAX_ANGLE) theta1_deg = SHOULDER_MAX_ANGLE;
    if (theta1_deg < SHOULDER_MIN_ANGLE) theta1_deg = SHOULDER_MIN_ANGLE;

    if (theta2_deg > ELBOW_MAX_ANGLE) theta2_deg = ELBOW_MAX_ANGLE;
    if (theta2_deg < ELBOW_MIN_ANGLE) theta2_deg = ELBOW_MIN_ANGLE;

    stepperShoulder.moveTo(angleToStepsAbs(theta1_deg));
    stepperElbow.moveTo(angleToStepsAbs(theta2_deg));

    shoulderAngleCurrent = theta1_deg;
    elbowAngleCurrent = theta2_deg;

    Serial.print("Moving to -> Shoulder: ");
    Serial.print(theta1_deg);
    Serial.print(" deg, Elbow: ");
    Serial.print(theta2_deg);
    Serial.println(" deg.");
}

void computeIKAndMove(float x, float y) {
    float D = (x * x + y * y - L1 * L1 - L2 * L2) / (2 * L1 * L2);

    if (D > 1.0) D = 1.0;
    if (D < -1.0) D = -1.0;

    float theta2_rad = atan2(sqrt(1 - D * D), D);
    float theta1_rad = atan2(y, x) - atan2(L2 * sin(theta2_rad), L1 + L2 * cos(theta2_rad));

    float theta1_deg = theta1_rad * 180.0 / PI;
    float theta2_deg = theta2_rad * 180.0 / PI;

    moveToAngles(theta1_deg, theta2_deg);
}

void homeArm() {
    Serial.println("Homing to initial position.");
    moveToAngles(135.0, 0.0);
}

void setup() {
    Serial.begin(9600);
    pinMode(SHOULDER_ENABLE_PIN, OUTPUT);
    pinMode(ELBOW_ENABLE_PIN, OUTPUT);
    digitalWrite(SHOULDER_ENABLE_PIN, LOW);
    digitalWrite(ELBOW_ENABLE_PIN, LOW);

    stepperShoulder.setMaxSpeed(MAX_SPEED);
    stepperShoulder.setAcceleration(ACCELERATION);

    stepperElbow.setMaxSpeed(MAX_SPEED);
    stepperElbow.setAcceleration(ACCELERATION);

    stepperShoulder.setCurrentPosition(angleToStepsAbs(shoulderAngleCurrent));
    stepperElbow.setCurrentPosition(angleToStepsAbs(elbowAngleCurrent));

    Serial.println("2-Link IK Arm Ready with Clipping Constraints.");
    Serial.println("Send 'x y' in cm to move, or 'h:' to home.");
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.startsWith("h:")) {
            homeArm();
        } else {
            int spaceIndex = input.indexOf(' ');
            if (spaceIndex > 0) {
                float x = input.substring(0, spaceIndex).toFloat();
                float y = input.substring(spaceIndex + 1).toFloat();
                computeIKAndMove(x, y);
            } else {
                Serial.println("Invalid input. Send 'x y' or 'h:'.");
            }
        }
    }

    stepperShoulder.run();
    stepperElbow.run();
}
