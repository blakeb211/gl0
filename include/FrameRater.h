#pragma once
#include "headers.h"
//
#include "circular_buffer.h"

constexpr auto FR_PRINT_FREQ = 1200;

using timepoint = std::chrono::high_resolution_clock::time_point;

class FrameRater {
public:
  // @TODO: replace with uint32
  unsigned long int frame_count;
  FrameRater()
    : last_time{ std::chrono::high_resolution_clock::now() },
    times{ circular_buffer<double>(600) },
    frame_count{ 0 },
    delta_time{ 0.0f } {}
  void UpdateTimes() {
    // add to time point
    this->frame_count++;
    timepoint newTime = std::chrono::high_resolution_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      newTime - this->last_time)
      .count();

    this->times.put((float)diff_ms);
    this->delta_time = (float)diff_ms;
    this->last_time = newTime;
  }
  float lastTimeInMs() { return this->delta_time; }
  void printFrameRateIfFreqHasBeenReached() {
    if (this->frame_count % FR_PRINT_FREQ == 0) {
      LogPrintLn("avg framerate:", this->getAvgFrameRate());
    }
  }

private:
  timepoint last_time;
  circular_buffer<double> times;  // ms between frames
  float delta_time;                // ms between last frame

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
