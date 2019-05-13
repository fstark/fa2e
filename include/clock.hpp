//
//  clock.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 20/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef clock_hpp
#define clock_hpp

#include "commander.hpp"
#include <iostream>

/**
 The clock is the heartbeat of the emulator.
 It is shared accross the emulator because some peripherals emulation needs to synchonize on the curent clock to simulate real-world
 */
class clock
{
	long cycles_;

public:
	explicit clock(const std::string name = "clock")
	    : cycles_(0)
	{
		commander::cli.register_variable(name, long_variable("cycles", cycles_));
	}

	/// 65 cycles per scan line
	static const int CYCLES_PER_SCAN_LINE = 65;

	///	262 lines per screen (192 + VBL)
	static const int SCAN_LINES_PER_SCREEN = 262;

	///	17030 cycles per refresh
	static const int CYCLES_PER_FRAME = CYCLES_PER_SCAN_LINE * SCAN_LINES_PER_SCREEN;

	///	Target frame rate
	static const int FRAME_RATE = 60;

	///	Convert from cycles to frames
	static long frames_from_cycles(const long cycles) { return cycles / CYCLES_PER_FRAME; }

	///	Convert from frames to cycles
	static long cycles_from_frames(const long frames) { return frames * CYCLES_PER_FRAME; }

	///	Increment frame rate
	clock& operator+=(const int v)
	{
		cycles_ += v;
		return *this;
	}

	///	Cycles since start
	long get_cycles() const { return cycles_; }

	friend std::ostream& operator<<(std::ostream& os, const class clock& obj);
};

std::ostream& operator<<(std::ostream& os, const class clock& c);

#endif /* clock_hpp */
