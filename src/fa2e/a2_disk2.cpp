//
//  io_disk2.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#include "a2_disk2.hpp"

#include <assert.h>
#include <iostream>

std::string string_from_mode(const a2_disk2::eMode mode)
{
    switch (mode)
    {
        case a2_disk2::kRead:
            return "READ";
        case a2_disk2::kSenseWriteProtect:
            return "SENSE WRITE PROTECT";
        case a2_disk2::kWrite:
            return "WRITE";
        case a2_disk2::kWriteLoad:
            return "WRITE LOAD";
    }
}



byte a2_disk2::read_byte(const int adrs)
{
	assert(adrs >= 0 && adrs <= 0xf);

	//		std::cout << "IODISK READ " << to_hex2(adrs) << std::endl;

	switch (adrs)
	{
		case 0x0:
			phase_ &= 0xe;
			//			std::cout << "  IODISK PHASE 0 OFF " << to_hex2(phase) << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x1:
			phase_ |= 0x1;
			//			std::cout << "  IODISK PHASE 0 ON " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x2:
			phase_ &= 0xd;
			//			std::cout << "  IODISK PHASE 1 OFF " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x3:
			phase_ |= 0x2;
			//			std::cout << "  IODISK PHASE 1 ON " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x4:
			phase_ &= 0xb;
			//			std::cout << "  IODISK PHASE 2 OFF " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x5:
			phase_ |= 0x4;
			//			std::cout << "  IODISK PHASE 2 ON " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x6:
			phase_ &= 0x7;
			//			std::cout << "  IODISK PHASE 3 OFF " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x7:
			phase_ |= 0x8;
			//			std::cout << "  IODISK PHASE 3 ON " << to_hex2(phase)  << std::endl;
			drives_[drive_].set_phase(phase_);
			break;
		case 0x8:
			//			std::cout << "  IODISK MOTOR OFF " << to_hex2(phase)  << std::endl;
			motor_ = false;
			break;
		case 0x9:
			//			std::cout << "  IODISK MOTOR ON" << std::endl;
			motor_ = true;
			break;
		case 0xa:
			//			std::cout << "  IODISK DRIVE 1" << std::endl;
			drive_ = 0;
			break;
		case 0xb:
			//			std::cout << "  IODISK DRIVE 2" << std::endl;
			drive_ = 1;
			break;
		case 0xc:
			mode_ = (eMode)(mode_ & 0x2);
			//				std::cout << "  IODISK Q6L MODE " << string_from_mode(mode) << std::endl;
			if (mode_ == kRead)
				return drives_[drive_].read();
			break;
		case 0xd:
			mode_ = (eMode)(mode_ | 0x2);
			//			std::cout << "  IODISK Q6H MODE " << to_ststring_from_modering(mode) << std::endl;
			break;
		case 0xe:
			mode_ = (eMode)(mode_ & 0x1);
			//			std::cout << "  IODISK Q7L MODE " << string_from_mode(mode) << std::endl;
			if (mode_ == kSenseWriteProtect)
				return 0x80;
			break;
		case 0xf:
			mode_ = (eMode)(mode_ | 0x1);
			//			std::cout << "  IODISK Q7H MODE " << string_from_mode(mode) << std::endl;
			break;
		default:;
	}
	return 0;
}
