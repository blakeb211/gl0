#pragma once
#include "circular_buffer.h"
#include <chrono>
#include <ratio>
#include <thread>

typedef std::chrono::high_resolution_clock::time_point timepoint;

template <std::intmax_t FPS> class FrameRater {
public:
  FrameRater()
      :                         // initialize the object keeping the pace
        time_between_frames{1}, // std::ratio<1, FPS> seconds
        tp{std::chrono::steady_clock::now()},
        lastTime{std::chrono::high_resolution_clock::now()},
        times{circular_buffer<double>(200)}, frame_count{0} {}

  void sleep() {
    // add to time point
    tp += time_between_frames;
    frame_count++;
    timepoint newTime = std::chrono::high_resolution_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                       newTime - lastTime)
                       .count();
    times.put((double)diff_ms);
    // check for framerates not being met
    if (frame_count % 500 == 0) {
      logPrintLn({
        "avg framerate:", this->getFrameRate()
      });
    }
    lastTime = newTime;

    std::this_thread::sleep_until(tp);
  }

private:
  // a duration with a length of 1/FPS seconds
  std::chrono::duration<double, std::ratio<1, FPS>> time_between_frames;

  // the time point we'll add to in every loop
  std::chrono::time_point<std::chrono::steady_clock,
                          decltype(time_between_frames)>
      tp;
  timepoint lastTime;
  circular_buffer<double> times;
  unsigned long int frame_count;

  double getFrameRate(void) {
    double total = 0.0;
    while (!times.empty()) {
      total += times.get();
    }
    auto avg_frametime_ms = total / times.capacity();
    auto avg_frametime_s = avg_frametime_ms / 1000.0;
    return 1.0 / avg_frametime_s;
  }
};
