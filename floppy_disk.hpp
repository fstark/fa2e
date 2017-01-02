//
//  floppy_disk.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 10/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef floppy_disk_hpp
#define floppy_disk_hpp

#include "core_types.hpp"

/**
	This abstract class represent a floppy disk (not the drive, the actual disk itself)
	A floppy disk can return a whole nibbled track to a floppy drive that would present it to the disk controller
*/
class floppy_disk
{
public:
	//	The number of nibbles in a sector
	static const int SECTOR_SIZE_NIB = 0x1a0;

	//	The number of sector per track
	static const int SECTOR_PER_TRACK = 16;

	//	The number of nibbles in a track
	static const int TRACK_SIZE_NIB = SECTOR_SIZE_NIB * SECTOR_PER_TRACK;

	//	The number of full tracks in a disk
	static const int TRACK_COUNT = 35;

	//	Loads TRACK_SIZE nibbles for the specific track
	virtual void load_track(void* const p, const int t) = 0;
};

/**
	A disk that is backed by a pre-nibbled file
 */
class nibbled_floppy_disk : public floppy_disk
{
	static const int SIZE = TRACK_COUNT * TRACK_SIZE_NIB;
	byte data_[SIZE];

public:
	nibbled_floppy_disk(const char* const afile);
	void load_track(void* const p, const int t);
};

/**
	A disk that is backed by a non nibbled file
 */
class raw_floppy_disk : public floppy_disk
{
public:
	static const int SECTOR_SIZE_RAW = 256;

private:
	static const int SIZE = TRACK_COUNT * SECTOR_PER_TRACK * SECTOR_SIZE_RAW;
	byte data_[SIZE];

public:
	raw_floppy_disk(const char* disk_data, size_t len);
	raw_floppy_disk(const char* afile);
	void load_track(void* const p, const int t);
};

#endif /* floppy_disk_hpp */
