#pragma once
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
       lastTime{std::chrono::high_resolution_clock::now()} {}


  void sleep() {
    // add to time point
    tp += time_between_frames;
    timepoint newTime = std::chrono::high_resolution_clock::now();
    auto diff_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(newTime - lastTime).count();
    
    // check for framerates not being met
    if (diff_ms > 10) {
      logPrintLn({
          "Min Framerate broken by a long frame.",
          "Diff in MS:",
          (int)diff_ms,
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
};
