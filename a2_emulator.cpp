//
//  apple2.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 18/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "a2_emulator.hpp"

#include "binary_data.hpp"

a2_emulator::a2_emulator()
    : speaker_(cpu_emulator_.get_clock())
    , disk2_(cpu_emulator_.get_clock())
    , video_(cpu_emulator_.get_bus())
    , io_mem_(video_, keyboard_, speaker_, disk2_)
    , apple2_plus_rom_(::apple2_plus_rom, 0xd000)
    , disk2_rom_(::disk2_rom)
{
	cpu_emulator_.get_bus().plug(0, &ram_, false);
	cpu_emulator_.get_bus().plug(0xc000, &io_mem_, true);

	cpu_emulator_.get_bus().plug(0xd000, &apple2_plus_rom_, false);
	cpu_emulator_.get_bus().plug(0xc600, &disk2_rom_, true);
}
