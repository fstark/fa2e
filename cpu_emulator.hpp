//
//  cpu_emulator.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 12/06/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef cpu_emulator_hpp
#define cpu_emulator_hpp

#include "bus.hpp"
#include "clock.hpp"
#include "cpu_6502.hpp"

/**
 This creates a working CPU, with no memory, nor any io
 Use get_memory() to access the memory and add ram, rom and io_space
 Use get_cpu() to access the cpu and execute instructions
 Use get_clock() to access the shared clock, if external clock sync is needed
 */
class cpu_emulator
{
	class clock clock_;
	bus bus_;
	cpu_6502 cpu_;

public:
	cpu_emulator()
	    : cpu_(bus_, clock_)
	{
	}

	class clock& get_clock() { return clock_; }
	bus& get_bus() { return bus_; }
	cpu_6502& get_cpu() { return cpu_; }
};

#endif /* cpu_emulator_hpp */
