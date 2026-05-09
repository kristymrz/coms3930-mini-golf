/**
 * Motor ESP32 - Mini Golf Door Motor
 * Continuously oscillates left/right to act as a blocking door
 * on the lighthouse entrance. No network logic needed.
 */

#include <Arduino.h>

#define MOTOR_IN1 32
#define MOTOR_IN2 27
#define MOTOR_IN3 26
#define MOTOR_IN4 13

// Duration each direction runs (ms). Tune this to control door speed.
#define SWING_DURATION_MS 1200

static int stepIndex = 0;

const int stepSequence[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1}
};

void setMotorPins(int step) {
    digitalWrite(MOTOR_IN1, stepSequence[step][0]);
    digitalWrite(MOTOR_IN2, stepSequence[step][1]);
    digitalWrite(MOTOR_IN3, stepSequence[step][2]);
    digitalWrite(MOTOR_IN4, stepSequence[step][3]);
}

void powerOffMotor() {
    digitalWrite(MOTOR_IN1, 0);
    digitalWrite(MOTOR_IN2, 0);
    digitalWrite(MOTOR_IN3, 0);
    digitalWrite(MOTOR_IN4, 0);
}

void moveForDuration(int direction, unsigned long durationMs) {
    unsigned long start = millis();
    while (millis() - start < durationMs) {
        stepIndex = (stepIndex + direction + 4) % 4;
        setMotorPins(stepIndex);
        delayMicroseconds(5000);
    }
    powerOffMotor();
}

void setup() {
    Serial.begin(115200);

    pinMode(MOTOR_IN1, OUTPUT);
    pinMode(MOTOR_IN2, OUTPUT);
    pinMode(MOTOR_IN3, OUTPUT);
    pinMode(MOTOR_IN4, OUTPUT);
    powerOffMotor();

    Serial.println("[MOTOR] Mini golf door motor starting...");
}

void loop() {
    Serial.println("[MOTOR] Swinging left...");
    moveForDuration(-1, SWING_DURATION_MS);

    Serial.println("[MOTOR] Swinging right...");
    moveForDuration(+1, SWING_DURATION_MS);
}
