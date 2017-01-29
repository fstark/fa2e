//
//  floppy_drive.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 08/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#include "floppy_drive.hpp"

/*
 The floppy drive motor is composed of 4 electormagnets, numbered from 0 to 3
 The apple2 will power those magnets one after the other, and the drive will spin to follow the magnets
 The set_phase gives the bit pattern of the 'on' magnets at the moment.
 We pretend that the drives only moves if:
	It was pointing to a magnet
	It now points to another adjacent magnet
 The drive moves per half-tracks, but we don't emulate those (ie: track 0 is double, then track 1, etc)
 */
void floppy_drive::set_phase(const int p)
{
	//	Those phases are the one with a single magnet on
	static const int s_motor[16] = {
		0, 1, 2, 0,
		3, 0, 0, 0,
		4, 0, 0, 0,
		0, 0, 0, 0
	};

	const auto m = s_motor[p];
	if (!m)
		return; //	Either no motors, or several motors

	//	When motors change, where does the head moves ?
	//	(it only moves when going from motor -> motor+1 or motor -> motor-1)
	static const int s_directions[5][5] = {
		{ 0, 0, 0, 0, 0 },
		{ 0, 0, 1, 0, -1 },
		{ 0, -1, 0, 1, 0 },
		{ 0, 0, -1, 0, 1 },
		{ 0, 1, 0, -1, 0 },
	};

	const auto d = s_directions[motor_][m];
	track_ += d;

	if (track_ < 0)
		track_ = 0;
	if (track_ >= TRACK * 2)
		track_ = TRACK * 2 - 1;

	motor_ = m;

	if (current_disk_)
		current_disk_->load_track(track_data_, track_ / 2);
}

void floppy_drive::insert(floppy_disk* d)
{
	current_disk_ = d;
	if (current_disk_)
		current_disk_->load_track(track_data_, track_ / 2);
}

//	Cycle count for each of the items in a sector
const int CYCLE_PER_BYTE = 32 /* 25 */;

/*
 Reading means read what is beyond the head track. Each byte stays 32 cycles, and we manage 13312 of such bytes under a single circular track. It is possible to "overclock" the disk drive up to 25 cycles per byte, but more than that and the internal ROM won't keep up with the data flowing
 */
byte floppy_drive::read()
{
	long c = clock_.get_cycles();
	c /= CYCLE_PER_BYTE;

	c = c % floppy_disk::TRACK_SIZE_NIB;

	static long last_read = 0;
	if (c == last_read)
		return 0x00; //	We have no new data yet
	last_read = c;

	return track_data_[c];
}
