#pragma once
#include "headers.h"
//
#include "circular_buffer.h"

constexpr auto FR_PRINT_FREQ = 400;

typedef std::chrono::high_resolution_clock::time_point timepoint;

class FrameRater {
 public:
  unsigned long int frame_count;
  FrameRater()
      : lastTime{std::chrono::high_resolution_clock::now()},
        times{circular_buffer<double>(400)},
        frame_count{0},
        deltaTime{0.0f} {}
  void UpdateTimes() {
    // add to time point
    this->frame_count++;
    timepoint newTime = std::chrono::high_resolution_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                       newTime - this->lastTime)
                       .count();

    this->times.put((float)diff_ms);
    this->deltaTime = (float)diff_ms;
    this->lastTime = newTime;
  }
  float lastTimeInMs() { return this->deltaTime; }
  void printFrameRateIfFreqHasBeenReached() {
    if (this->frame_count % FR_PRINT_FREQ == 0) {
      logPrintLn("avg framerate:", this->getAvgFrameRate());
    }
  }

 private:
  timepoint lastTime;
  circular_buffer<double> times;  // ms between frames
  float deltaTime;                // ms between last frame

  double getAvgFrameRate(void) {
    // sum and empty the circular_buffer
    double total = 0.0;
    while (!times.empty()) {
      total += times.get();
    }
    auto avg_frametime_ms = total / times.capacity();
    auto avg_frametime_s = avg_frametime_ms / 1000.0;
    return 1.0 / avg_frametime_s;
  }
};
