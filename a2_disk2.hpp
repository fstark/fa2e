//
//  io_disk2.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef io_disk2_hpp
#define io_disk2_hpp

#include "clock.hpp"
#include "floppy_drive.hpp"

/**
 This is a read-only apple2 diskII drive enclosure
 It emulate I/O ports into state changes and calls to actual floppy_drives
 */
class a2_disk2
{
	static void phase_change_null_callback(void*, int);
	static byte read_byte_null_callback(void*);

	//	Bitpattern of values are actually important
	typedef enum {
		kRead              = 0x0,
		kSenseWriteProtect = 0x1,
		kWrite             = 0x2,
		kWriteLoad         = 0x3
	} eMode;

	eMode mode_;
	int phase_;
	int drive_;
	bool motor_;
	floppy_drive drives_[2];

public:
	a2_disk2(class clock& clock, const std::string name = "disk2")
	    : mode_(kRead)
	    , phase_(0)
	    , drive_(0)
	    , motor_(false)
	    , drives_{ floppy_drive(clock, "drive1"), floppy_drive(clock, "drive2") }
	{
		commander::cli.register_variable(name, { "mode", [&]() { return std::to_string(mode_); } }); //	####
		commander::cli.register_variable(name, { "drive_", [&]() { return std::to_string(drive_); } });
		commander::cli.register_variable(name, bool_variable("motor", motor_));
		commander::cli.register_variable(name, int_variable("phase", phase_));
	}

	std::string to_string(const eMode mode) const
	{
		switch (mode)
		{
			case kRead:
				return "READ";
			case kSenseWriteProtect:
				return "SENSE WRITE PROTECT";
			case kWrite:
				return "WRITE";
			case kWriteLoad:
				return "WRITE LOAD";
		}
	}

	///	Read a byte from the I/O range low byte
	byte read_byte(const int adrs);

	floppy_drive& get_drive(const int d) { return drives_[d]; }
};

#endif /* io_disk2_hpp */
