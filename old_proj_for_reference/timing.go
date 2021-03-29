package frametimer

import (
	"go-sdl2/sdl"
	"time"
)

const frameCountMax = 100 // max times to use for framerate calculation

// Timer struct holds info on the currently running timer
type Timer struct {
	times       [frameCountMax]int64 // frame times in microseconds
	index       int                  // index of next timeslot to write to
	TotalTime   int64                // running time in microseconds
	TotalFrames int64
	lastTime    time.Time
}

//RecordTime calculates the time since the last frame was added
//and resets the timer. It also increments the TotalFrames
//and the TotalTime the timer has been running.
func (ft *Timer) RecordTime() int64 {
	// delay to cap the framerate
	frametime := ft.GetElapsedSinceLast()
	//if ft.TotalFrames%50 == 0 {
	//	fmt.Println("frametime (ms):", float64(frametime/1_000))
	//}
	pauseTime := float64(14_333 - frametime)
	if pauseTime > 0 {
		delayMilli := uint32(pauseTime / 1_000)
		sdl.Delay(delayMilli)
	}
	// officially record frame time
	ft.times[ft.index] = ft.GetElapsedSinceLast()
	returnVal := ft.times[ft.index]
	ft.lastTime = time.Now()
	ft.TotalTime += ft.times[ft.index]
	ft.TotalFrames++
	ft.index++
	// loop back to front of array and rewrite previous times
	if ft.index == frameCountMax {
		ft.index = 0
	}
	return returnVal
}

// GetElapsedSinceLast returns the microseconds
// since the lastTime time point.
func (ft *Timer) GetElapsedSinceLast() int64 {
	return time.Since(ft.lastTime).Microseconds()
}

// CalcFPS calculate average framerate
func (ft Timer) CalcFPS() float64 {
	var tot float64
	for i := range ft.times {
		tot += float64(ft.times[i])
	}
	return 1 / ((tot / frameCountMax) / 1000000.0)
}
