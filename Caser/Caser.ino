/**
 * Move cat's favourite toy around so I don't have to :)
 * 
 * @author costing
 */
 
#include <Servo.h>
#include <LowPower.h>

// Run for 10 min, then power down. Press the reset button to start it again.
#define TIME_LIMIT 600000L

// pan
Servo s1;

// tilt
Servo s2;

// Arduino pins for the two servos and the laser
#define S1_PIN A4
#define S2_PIN A5
#define LASER 3

// Absolute min and max limits for servo travel
// left
#define S1_MAX 2300

// right
#define S1_MIN 850

// top
#define S2_MIN 1700

// bottom
#define S2_MAX 2300

// servo state (speed, position and travel direction)
double s1_inc = 0;
double s2_inc = 0;

double s1_pos = (S1_MIN + S1_MAX) / 2;
double s2_pos = (S2_MIN + S2_MAX) / 2;

int s1_dir = 1;
int s2_dir = 1;

// configure the pins
void setup() {
  s1.attach(S1_PIN);
  s2.attach(S2_PIN);

  pinMode(LASER, OUTPUT);
}

// very simple scanning within the travel limits
void loop() {
  if (millis() > TIME_LIMIT) {
    // after the preset amount of time stop everything and shut down the CPU
    digitalWrite(LASER, LOW);
    pinMode(LASER, INPUT);

    s1.detach();
    s2.detach();

    // If running on a battery then after a short while of very little consumption it will cut the power to it
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

    return;
  }

  s1_inc = (millis() / 30000L) % 4 + 2.1;
  s2_inc = (millis() / 40000L) % 4 + 1.1;

  s1_pos += s1_dir * s1_inc;
  
  if (s1_pos > S1_MAX) {
    s1_pos = S1_MAX - s1_inc;
    s1_dir = -1;
  }

  if (s1_pos < S1_MIN) {
    s1_pos = S1_MIN + s1_inc;
    s1_dir = 1;
  }

  s2_pos += s2_dir * s2_inc;

  if (s2_pos > S2_MAX) {
    s2_pos = S2_MAX - s2_inc;
    s2_dir = -1;
  }

  if (s2_pos < S2_MIN) {
    s2_pos = S2_MIN + s2_inc;
    s2_dir = 1;
  }

  s1.writeMicroseconds((int) s1_pos);
  s2.writeMicroseconds((int) s2_pos);

  // vary laser intensity with the speed
  analogWrite(LASER, (s1_inc + s2_inc) * 5);

  delay(20);
}
