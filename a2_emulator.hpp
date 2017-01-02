//
//  apple2.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 18/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef apple2_hpp
#define apple2_hpp

#include "cpu_emulator.hpp"

#include "a2_disk2.hpp"
#include "a2_io_mem.hpp"
#include "a2_video.hpp"

/**
 Creates a a cpu_emulator and configure it into a working apple2+ emultor, with 48K of ram, a keyboard, a speacker, a video device and a read-only dual floppy in slot 6
 */
class a2_emulator
{
	cpu_emulator cpu_emulator_;

	a2_speaker speaker_;
	a2_disk2 disk2_;
	a2_video video_;
	a2_keyboard keyboard_;
	a2_io_mem io_mem_;

	ram<0x10000> ram_;
	rom<0x3000> apple2_plus_rom_;
	rom<0x100> disk2_rom_;

public:
	a2_emulator(); //	#### I THINK IT NEEDS TO BE PASSED THE ROM CHIPS

	void reset() { cpu_emulator_.get_cpu().reset(); }
	void exec() { cpu_emulator_.get_cpu().exec(); }

	cpu_emulator& get_cpu_emulator() { return cpu_emulator_; }

	a2_io_mem& get_io_mem() { return io_mem_; }
	a2_keyboard& get_keyboard() { return keyboard_; }
	a2_speaker& get_speaker() { return speaker_; }
	a2_video& get_video() { return video_; }
	a2_disk2& get_disk() { return disk2_; }
};

#endif /* apple2_hpp */
