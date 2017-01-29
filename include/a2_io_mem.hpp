//
//  io_mem.hpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright (c) 2016 Fred. All rights reserved.
//

#ifndef io_mem_hpp
#define io_mem_hpp

#include "a2_disk2.hpp"
#include "a2_keyboard.hpp"
#include "a2_speaker.hpp"
#include "a2_video.hpp"
#include "chip.hpp"
#include <iostream>

/**
 The Apple2 io memory, mapped between C000 to C0FF
 It will communicate with the various drivers to expose them as I/O registers
 */
class a2_io_mem : public chip
{
	//	Returns name of the adress, for easer of debugging
	static std::string nameof(const byte adrs);

	a2_video& video_;
	a2_keyboard& keyboard_;
	a2_speaker& speaker_;
	a2_disk2& disk2_;

public:
	a2_io_mem(a2_video& video, a2_keyboard& keyboard, a2_speaker& speaker, a2_disk2& disk2)
	    : video_(video)
	    , keyboard_(keyboard)
	    , speaker_(speaker)
	    , disk2_(disk2)
	{
	}

	byte r(const bool f)
	{
		return f ? 0x80 : 0x00;
	}

	///	For I/O port that act on read or write
	byte touch(const byte adrs)
	{
		if (adrs == 0x10)
		{
			keyboard_.clear();
			return 0xff;
		}

		if (adrs == 0x50) {
			video_.set_text(false);
			return r(video_.is_text());
		}
		if (adrs == 0x51) {
			video_.set_text(true);
			return r(video_.is_text());
		}
		if (adrs == 0x52) {
			video_.set_mixed(false);
			return r(video_.is_mixed());
		}
		if (adrs == 0x53) {
			video_.set_mixed(true);
			return r(video_.is_mixed());
		}
		if (adrs == 0x54) {
			video_.set_page2(false);
			return r(video_.is_page2());
		}
		if (adrs == 0x55) {
			video_.set_page2(true);
			return r(video_.is_page2());
		}
		if (adrs == 0x56) {
			video_.set_hires(false);
			return r(video_.is_hires());
		}
		if (adrs == 0x57) {
			video_.set_hires(true);
			return r(video_.is_hires());
		}

		return 0xff;
	}

	byte read_byte(const int adrs)
	{
		//	disk
		if ((adrs & 0xf0) == 0xe0)
		{
			return disk2_.read_byte(adrs & 0xf);
		}

		//	kbd
		if ((adrs & 0xf0) == 0x0) //	#### 0-F
		{
			return keyboard_.read();
		}

		//	spkr
		if ((adrs & 0xff) == 0x30) {
			speaker_.click();
		}

		//	?
		if (adrs == 0x81)
		{
			return 0xb0; //	wtf...
		}

		//	1 bit input
		switch (adrs & 0xff)
		{
			case 0x61:
			case 0x62:
			case 0x63:
				return 0x00;
		}

		//	anything else
		return touch(adrs);
	}

	void write_byte(const int adrs, const byte value)
	{
		(void)touch(adrs);

#ifdef VERBOSE
//		std::cout << "IO WRITE @C0" << to_hex2(adrs)  << "(" << nameof(adrs) << ")" << "=" << to_hex2(value) << std::endl;
#endif
	}

	int size() const { return 256; }
};

#endif /* io_mem_hpp */
