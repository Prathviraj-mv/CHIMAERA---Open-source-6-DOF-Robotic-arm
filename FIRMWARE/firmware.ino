#include <AccelStepper.h>

// === CONFIGURATION ===
#define L1 23.0
#define L2 17.5

#define DRIVER 1

// SHOULDER
#define SHOULDER_STEP_PIN 54
#define SHOULDER_DIR_PIN  55
#define SHOULDER_ENABLE_PIN 38

// ELBOW
#define ELBOW_STEP_PIN 60
#define ELBOW_DIR_PIN  61
#define ELBOW_ENABLE_PIN 56

// BASE
#define BASE_STEP_PIN 46
#define BASE_DIR_PIN  48
#define BASE_ENABLE_PIN 62

#define STEPS_PER_REV 200
#define MICROSTEPPING 8
#define GEAR_RATIO 5.0

#define MAX_SPEED 1600
#define ACCELERATION 800

// === JOINT CONSTRAINTS ===
float SHOULDER_MIN_ANGLE = 75.0;
float SHOULDER_MAX_ANGLE = 135.0;
float ELBOW_MIN_ANGLE = 0.0;
float ELBOW_MAX_ANGLE = 75.0;
float BASE_MIN_ANGLE = 0.0;
float BASE_MAX_ANGLE = 180.0;

// === STATE ===
AccelStepper stepperShoulder(DRIVER, SHOULDER_STEP_PIN, SHOULDER_DIR_PIN);
AccelStepper stepperElbow(DRIVER, ELBOW_STEP_PIN, ELBOW_DIR_PIN);
AccelStepper stepperBase(DRIVER, BASE_STEP_PIN, BASE_DIR_PIN);

float shoulderAngleCurrent = 135.0; // mechanical reference (up)
float elbowAngleCurrent = 0.0;      // straight
float baseAngleCurrent = 90.0;      // center

// === FUNCTIONS ===
long angleToStepsAbs(float angle_deg) {
    float stepsPerMotorRev = STEPS_PER_REV * MICROSTEPPING;
    float stepsPerDegree = stepsPerMotorRev / 360.0;
    float requiredMotorAngle = angle_deg * GEAR_RATIO;
    return requiredMotorAngle * stepsPerDegree;
}

void moveToAngles(float theta1_deg, float theta2_deg, float base_deg, bool baseUpdate) {
    // Clip angles safely
    theta1_deg = constrain(theta1_deg, SHOULDER_MIN_ANGLE, SHOULDER_MAX_ANGLE);
    theta2_deg = constrain(theta2_deg, ELBOW_MIN_ANGLE, ELBOW_MAX_ANGLE);
    if (baseUpdate) {
        base_deg = constrain(base_deg, BASE_MIN_ANGLE, BASE_MAX_ANGLE);
        stepperBase.moveTo(angleToStepsAbs(base_deg));
        baseAngleCurrent = base_deg;
    }

    stepperShoulder.moveTo(angleToStepsAbs(theta1_deg));
    stepperElbow.moveTo(angleToStepsAbs(theta2_deg));

    shoulderAngleCurrent = theta1_deg;
    elbowAngleCurrent = theta2_deg;

    Serial.print("Moving -> Shoulder: "); Serial.print(theta1_deg);
    Serial.print(" deg, Elbow: "); Serial.print(theta2_deg);
    Serial.print(" deg, Base: "); Serial.println(baseAngleCurrent);
}

void computeIKAndMove(float x, float y, float theta_base) {
    float D = (x * x + y * y - L1 * L1 - L2 * L2) / (2 * L1 * L2);
    D = constrain(D, -1.0, 1.0);

    float theta2_rad = atan2(sqrt(1 - D * D), D);
    float theta1_rad = atan2(y, x) - atan2(L2 * sin(theta2_rad), L1 + L2 * cos(theta2_rad));

    float theta2_deg = theta2_rad * 180.0 / PI;
    float theta1_deg_geom = theta1_rad * 180.0 / PI;

    // Adjust for your mechanical convention:
    float theta1_deg_mech = 135.0 - theta1_deg_geom; // 0 geom = 135 mech (up)
    float theta2_deg_mech = theta2_deg;              // direct

    // Use current base if theta_base == 0
    bool baseUpdate = false;
    float base_deg = baseAngleCurrent;
    if (theta_base != 0.0) {
        base_deg = theta_base;
        baseUpdate = true;
    }

    moveToAngles(theta1_deg_mech, theta2_deg_mech, base_deg, baseUpdate);
}

void homeArm() {
    Serial.println("Homing all joints.");
    moveToAngles(135.0, 0.0, 90.0, true);
}

void setup() {
    Serial.begin(9600);

    pinMode(SHOULDER_ENABLE_PIN, OUTPUT);
    pinMode(ELBOW_ENABLE_PIN, OUTPUT);
    pinMode(BASE_ENABLE_PIN, OUTPUT);

    digitalWrite(SHOULDER_ENABLE_PIN, LOW);
    digitalWrite(ELBOW_ENABLE_PIN, LOW);
    digitalWrite(BASE_ENABLE_PIN, LOW);

    stepperShoulder.setMaxSpeed(MAX_SPEED);
    stepperShoulder.setAcceleration(ACCELERATION);
    stepperElbow.setMaxSpeed(MAX_SPEED);
    stepperElbow.setAcceleration(ACCELERATION);
    stepperBase.setMaxSpeed(MAX_SPEED);
    stepperBase.setAcceleration(ACCELERATION);

    // Set initial positions:
    stepperShoulder.setCurrentPosition(angleToStepsAbs(shoulderAngleCurrent));
    stepperElbow.setCurrentPosition(angleToStepsAbs(elbowAngleCurrent));
    stepperBase.setCurrentPosition(angleToStepsAbs(baseAngleCurrent));

    Serial.println("3-DOF IK Arm Ready. Send: x y theta or h: to home.");
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.startsWith("h:")) {
            homeArm();
        } else {
            int firstSpace = input.indexOf(' ');
            int secondSpace = input.indexOf(' ', firstSpace + 1);

            if (firstSpace > 0 && secondSpace > firstSpace) {
                float x = input.substring(0, firstSpace).toFloat();
                float y = input.substring(firstSpace + 1, secondSpace).toFloat()+ 10;
                float theta = input.substring(secondSpace + 1).toFloat();
                computeIKAndMove(x, y, theta);
            } else {
                Serial.println("❌ Invalid input. Use: x y theta");
            }
        }
    }

    stepperShoulder.run();
    stepperElbow.run();
    stepperBase.run();
}
