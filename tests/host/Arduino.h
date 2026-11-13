// Just enough of the Arduino API to compile Sky_Forge_Logic.ino on a PC and test its logic.
// pulseIn() plays back the echo durations a test queues in echoes; everything else is a no-op
// or records what the sketch did.
#pragma once
#include <cstdint>
#include <deque>
#include <sstream>
#include <string>

#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1

inline std::deque<long> echoes;           // next pulseIn() results, in microseconds (0 = no echo)
inline int pulses = 0;                    // how many times the sketch pinged
inline unsigned long fakeMillis = 0;      // advanced by delay()

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline void delay(unsigned long ms) { fakeMillis += ms; }
inline void delayMicroseconds(unsigned int) {}
inline unsigned long millis() { return fakeMillis; }
inline long pulseIn(int, int, unsigned long) {
  pulses++;
  if (echoes.empty()) return 0;
  long d = echoes.front();
  echoes.pop_front();
  return d;
}

template <class T> T constrain(T x, T lo, T hi) { return x < lo ? lo : (x > hi ? hi : x); }

struct FakeSerial {
  std::ostringstream out;
  void begin(long) {}
  template <class T> void print(const T& x) { out << x; }
  template <class T> void println(const T& x) { out << x << "\n"; }
  std::string take() { std::string s = out.str(); out.str(""); return s; }
};
inline FakeSerial Serial;
