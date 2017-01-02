//
//  floppy_drive.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 08/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef floppy_drive_hpp
#define floppy_drive_hpp

#include "clock.hpp"
#include "floppy_disk.hpp"

/**
 	Hanlde unprotected dsk format, 6+2 format
 */
class floppy_drive
{
	static const int SECTOR      = 16;
	static const int TRACK       = 35;
	static const int SECTOR_SIZE = 256;

	floppy_disk* current_disk_ = nullptr;

	const class clock& clock_;

	int track_;  //	The track the head is on
	int sector_; //	The sector the head is on
	int motor_;  //	The current active motor, if any

	void load(int t, int s, bool force);

	friend void phase_change_callback(void* data, int phase);
	friend byte read_byte_callback(void* data);

	byte track_data_[floppy_disk::TRACK_SIZE_NIB];

public:
	floppy_drive(class clock& clock, const std::string name)
	    : clock_(clock)
	    , track_(0)
	    , sector_(0)
	    , motor_(0)
	{
		commander::cli.register_variable(name, int_variable("track", track_));
		commander::cli.register_variable(name, int_variable("sector", sector_));
		commander::cli.register_variable(name, int_variable("motor", motor_));

		commander::cli.register_variable(name, { "disk", [&]() { return std::to_string((long)current_disk_); } }); //	####
	}

	void set_phase(const int p);
	byte read();

	void insert(floppy_disk* d);
};

#endif /* floppy_drive_hpp */
