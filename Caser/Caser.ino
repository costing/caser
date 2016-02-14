/**
 * Move cat's favourite toy around so I don't have to :)
 *
 * @author costing
 */

#include <Servo.h>

/**
 * @see https://github.com/rocketscream/Low-Power
 */
#include <LowPower.h>

// Run for 15 min, then power down. Press the reset button to start it again.
#define TIME_LIMIT 900000L

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

// middle points
#define S1_MID ((S1_MIN + S1_MAX) / 2)
#define S2_MID ((S2_MIN + S2_MAX) / 2)

// configure the pins
void setup() {
  s1.attach(S1_PIN);
  s2.attach(S2_PIN);

  pinMode(LASER, OUTPUT);
}

// disconnect the servos and enter low power mode forever
void shutdown() {
  // after the preset amount of time stop everything and shut down the CPU
  digitalWrite(LASER, LOW);
  pinMode(LASER, INPUT);

  s1.detach();
  s2.detach();

  // If running on a battery then after a short while of very little consumption it will cut the power to it
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

// move the two servos
void writeMicroseconds(const int x, const int y) {
  s1.writeMicroseconds(x);
  s2.writeMicroseconds(y);
}

void drawCircles(const int baseRadius, const int steps, const int times) {
  const int rx = 4 * baseRadius;
  const int ry = 2 * baseRadius;

  const int xCentre = S1_MIN + random(rx, S1_MAX - S1_MIN - rx);
  const int yCentre = S2_MIN - 100 + random(ry, S2_MAX - S2_MIN - ry);

  analogWrite(LASER, 100);

  for (int t = 0; t < times; t++)
    for (int i = 0; i < steps; i++)
    {
      const long start = millis();
      const float angle = i * 2 * M_PI / steps;
      const int xPos = xCentre + (cos(angle) * rx);
      const int yPos = yCentre + (sin(angle) * ry);

      writeMicroseconds(xPos, yPos);
      delay(max(millis() - start, 20));
    }
}

void roaming(const int timelimit, const int increment) {
  const long start = millis();

  double s1_pos = S1_MID;
  double s2_pos = S2_MID;

  int s1_dir = 1;
  int s2_dir = 1;

  while ( (millis() - start) < timelimit * 1000L) {
    const double s1_inc = (millis() / 30000L) % 4 + increment / 10.;
    const double s2_inc = (millis() / 40000L) % 4 + increment / 21.;

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
      s2_pos = S2_MAX;
      s2_dir = -1;
    }

    if (s2_pos < S2_MIN) {
      s2_pos = S2_MIN;
      s2_dir = 1;
    }

    writeMicroseconds((int) s1_pos, (int) s2_pos);

    analogWrite(LASER, (s1_inc + s2_inc) * 5);

    delay(20);
  }
}

void goToCartesian(const double x, const double y) {
  const double hypotenuse = sqrt(x * x + y * y);

  const double s1_rad = hypotenuse > 0 ? atan2(y, x) : M_PI / 2;

  const double s2_rad = atan2(hypotenuse, 180);   // fixed height

  const double s1_new_pos = map(s1_rad * 1000, 0, M_PI * 1000, S1_MIN, S1_MAX);
  const double s2_new_pos = S2_MAX - (map(s2_rad * 1000, 0, M_PI * 1000, S1_MIN, S1_MAX) - S1_MIN);

  writeMicroseconds(s1_new_pos, s2_new_pos);
}

double xold = 0;
double yold = 0;

void moveToCartesian(const double x, const double y, const double duration) {
  if (duration <= 20) {
    // go directly to the target coordinates
    goToCartesian(x, y);

    if (duration > 0)
      delay(duration);
  }
  else {
    // linearly go between the old and the new positions
    const double dx = (x - xold) * 20. / duration;
    const double dy = (y - yold) * 20. / duration;

    double t = 0;

    while (t < duration) {
      long dt = millis();

      xold += dx;
      yold += dy;
      t += 20;
      goToCartesian(xold, yold);

      dt = millis() - dt;

      if (dt < 20)
        delay(20 - dt);
    }
  }

  xold = x;
  yold = y;
}

// draw random lines that take travelTime to draw
void drawLines(const int times, const int travelTime, const int pause) {
  for (int i = 0; i < times; i++) {
    const int x = random(-600, 600);
    const int y = random(0, 500);

    analogWrite(LASER, 200);

    moveToCartesian(x, y, travelTime);

    analogWrite(LASER, 500);

    delay(pause);
  }
}

void loop() {
  if (millis() > TIME_LIMIT) {
    shutdown();
  }

  const int r = random(0, 3);

  switch (r) {
    case 0:
      // (count, travel time, pause)
      drawLines(random(20, 50), random(300, 1000), random(1000, 3000));
      break;

    case 1:
      // (radius, steps (1/speed), times)
      drawCircles(random(30, 100), random(30, 200), random(5, 8));
      break;

    default:
      // (seconds, speed)
      roaming(random(30, 60), random(20, 40));
  }

  analogWrite(LASER, 0);
  delay(1000);
}
