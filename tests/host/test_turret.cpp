// Host tests for the turret logic: g++ -std=c++17 -I tests/host tests/host/test_turret.cpp && ./a.out
// They exercise the sketch's own functions, compiled unchanged, against tests/host/Arduino.h.
#include <cstdio>
#include <cstdlib>
#include "Servo.h"
#include "../../Source Codes/Sky_Forge_Logic.ino"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

// Round-trip echo time for a target d cm away, as the sketch converts it (29 us/cm each way).
static long echoFor(long cm) { return cm * 58 + 10; }

static void queue(std::initializer_list<long> cms) {
  echoes.clear();
  for (long cm : cms) echoes.push_back(cm < 0 ? 0 : echoFor(cm));
}

int main() {
  const long NONE = MAX_DISTANCE_CM + 1;

  // readDistanceOnce: microseconds -> cm, and "no echo" -> out of range
  queue({20}); CHECK(readDistanceOnce() == 20);
  queue({-1}); CHECK(readDistanceOnce() == NONE);

  // readDistanceAvg: averages the in-range echoes, ignores the rest
  queue({20, 24}); CHECK(readDistanceAvg(2) == 22);
  queue({20, -1}); CHECK(readDistanceAvg(2) == 20);
  queue({-1, 45}); CHECK(readDistanceAvg(2) == NONE);

  // voteDistance: two readings in range that agree make a target
  CHECK(voteDistance(28, 27, NONE) == 27);
  CHECK(voteDistance(NONE, 12, 14) == 13);
  CHECK(voteDistance(10, 12, 11) == 10);                      // the closest-agreeing pair wins
  // ... and two that disagree do not: a stray 5 cm echo plus a 28 cm target is not a 16 cm target
  CHECK(voteDistance(5, 28, NONE) == NONE);
  CHECK(voteDistance(5, 28, 40) == NONE);
  CHECK(voteDistance(NONE, NONE, 20) == NONE);
  CHECK(voteDistance(30, 30 - VOTE_AGREE_CM, NONE) == 30 - (VOTE_AGREE_CM + 1) / 2);
  CHECK(voteDistance(30, 30 - VOTE_AGREE_CM - 1, NONE) == NONE);

  // One sweep step with nothing in front: no engagement, and SAMPLES_PER_READ echoes per reading
  setup();
  Serial.take();
  pulses = 0;
  echoes.clear();
  loop();
  std::string out = Serial.take();
  CHECK(pulses == 3 * SAMPLES_PER_READ);
  CHECK(out.find("scan a=0 d=0") != std::string::npos);
  CHECK(out.find("ENGAGE") == std::string::npos);

  // A target at 20 cm in every reading: the step engages, re-checks and fires
  scanAngle = 45;
  echoes.clear();
  for (int i = 0; i < 4 * SAMPLES_PER_READ; i++) echoes.push_back(echoFor(20));
  loop();
  out = Serial.take();
  CHECK(out.find("scan a=45 d=20") != std::string::npos);
  CHECK(out.find("ENGAGE at angle 45 dist=20") != std::string::npos);
  CHECK(out.find("FIRE!") != std::string::npos);
  CHECK(servoAim.read() == 45 + AIM_ANGLE_OFFSET);

  if (failures) { std::printf("%d check(s) failed\n", failures); return 1; }
  std::printf("all turret checks passed\n");
  return 0;
}
