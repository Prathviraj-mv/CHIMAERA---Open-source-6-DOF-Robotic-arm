#include <AccelStepper.h>
#include <Servo.h>

#define DRIVER 1

// Stepper Pins (RAMPS 1.4)
#define X_STEP_PIN    54
#define X_DIR_PIN     55
#define X_ENABLE_PIN  38

#define Y_STEP_PIN    60
#define Y_DIR_PIN     61
#define Y_ENABLE_PIN  56

#define Z_STEP_PIN    46
#define Z_DIR_PIN     48
#define Z_ENABLE_PIN  62

// Servo Pins (RAMPS)
#define SERVO1_PIN    11
#define SERVO2_PIN    6

// Speed Macros
#define sp 2000
#define sp1 1000

AccelStepper stepperX(DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(DRIVER, Z_STEP_PIN, Z_DIR_PIN);

Servo servo1;
Servo servo2;

int servo1Angle = 90;
int servo2Angle = 90;

String commandQueue = "";
String blocks[20];   // up to 20 blocks
int blockCount = 0;
int currentBlock = 0;

bool executing = false;
bool blockExecuting = false;
bool zToggleDirection = true;

void setup() {
  Serial.begin(9600);

  // Enable steppers
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(Z_ENABLE_PIN, LOW);

  // Attach servos
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo1.write(servo1Angle);
  servo2.write(servo2Angle);

  // Stepper configuration
  stepperX.setMaxSpeed(sp);
  stepperX.setAcceleration(sp1);

  stepperY.setMaxSpeed(sp);
  stepperY.setAcceleration(sp1);

  stepperZ.setMaxSpeed(sp);
  stepperZ.setAcceleration(sp1);

  Serial.println("Ready: Use '0' in strings for block pauses.");
}

void processCommand(char cmd) {
  switch (cmd) {
    // Servo 1 control
    case '7':
      servo1Angle = constrain(servo1Angle + 10, 0, 180);
      servo1.write(servo1Angle);
      Serial.print("Servo1 angle: "); Serial.println(servo1Angle);
      break;
    case '4':
      servo1Angle = constrain(servo1Angle - 10, 0, 180);
      servo1.write(servo1Angle);
      Serial.print("Servo1 angle: "); Serial.println(servo1Angle);
      break;

    // Elbow X-axis
    case '8':
      stepperX.move(600);
      Serial.println("Stepper X: +600 steps");
      break;
    case '5':
      stepperX.move(-600);
      Serial.println("Stepper X: -600 steps");
      break;

    // Shoulder Y-axis
    case '9':
      stepperY.move(600);
      Serial.println("Stepper Y: +600 steps");
      break;
    case '6':
      stepperY.move(-600);
      Serial.println("Stepper Y: -600 steps");
      break;

    // Base Z-axis toggle
    case 'c':
      if (zToggleDirection) {
        stepperZ.move(-1200);
        Serial.println("Stepper Z: -1200 steps");
      } else {
        stepperZ.move(1200);
        Serial.println("Stepper Z: +1200 steps");
      }
      zToggleDirection = !zToggleDirection;
      break;
  }
}



void splitIntoBlocks(String input) {
  blockCount = 0;
  String current = "";
  for (int i = 0; i < input.length(); i++) {
    if (input[i] == '0') {
      if (current.length() > 0) {
        blocks[blockCount++] = current;
        current = "";
      }
    } else {
      current += input[i];
    }
  }
  if (current.length() > 0) {
    blocks[blockCount++] = current;
  }
}

void executeBlock(String block) {
  Serial.print("Executing block: "); Serial.println(block);
  for (int i = 0; i < block.length(); i++) {
    char cmd = block[i];
    processCommand(cmd);
  }
}

void loop() {
  if (Serial.available() && !executing) {
    commandQueue = Serial.readStringUntil('\n');
    splitIntoBlocks(commandQueue);
    executing = true;
    currentBlock = 0;
    blockExecuting = false;
    Serial.print("Blocks queued: ");
    for (int i = 0; i < blockCount; i++) {
      Serial.print(blocks[i]); Serial.print(" | ");
    }
    Serial.println();
  }

  if (executing && !blockExecuting && currentBlock < blockCount) {
    executeBlock(blocks[currentBlock]);
    blockExecuting = true;
  }

  if (blockExecuting) {
    stepperX.run();
    stepperY.run();
    stepperZ.run();

    if (!stepperX.isRunning() && !stepperY.isRunning() && !stepperZ.isRunning()) {
      currentBlock++;
      blockExecuting = false;

      if (currentBlock >= blockCount) {
        executing = false;
        Serial.println("Done executing all blocks.");
      }
    }
  }
}
