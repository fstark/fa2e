//
//  io_mem.cpp
//  FAE_SDL
//
//  Created by Frederic Stark on 17/05/16.
//  Copyright © 2016 Fred. All rights reserved.
//

#include "a2_io_mem.hpp"

#include <assert.h>

//	Understanding the apple 2, p 158
std::string a2_io_mem::nameof(const byte adrs)
{
	switch (adrs)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			return "KBD";
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			return "KBDSTRB";
		case 0x30:
			return "SPKR";
		case 0x50:
			return "GRAPHICS";
		case 0x51:
			return "TXTSET";
		case 0x52:
			return "CLRMIXED";
		case 0x53:
			return "SETMIXED";
		case 0x54:
			return "TXTPAGE1";
		case 0x55:
			return "TXTPAGE2";
		case 0x56:
			return "LOWRES";
		case 0x57:
			return "HIRES";
		case 0x58:
			return "SETAN0";
		case 0x5A:
			return "SETAN1";
		case 0x5D:
			return "CLRAN2";
		case 0x5F:
			return "CLRAN3";

		case 0x61: return "1BIT-1";
		case 0x62: return "1BIT-2";
		case 0x63: return "1BIT-3";

		case 0x81:
			return "ROMIN"; //	GOD, HELP ME

		case 0xe0: return "PHASE0OFF";
		case 0xe1: return "PHASE0ON";
		case 0xe2: return "PHASE1OFF";
		case 0xe3: return "PHASE1ON";
		case 0xe4: return "PHASE2OFF";
		case 0xe5: return "PHASE2ON";
		case 0xe6: return "PHASE3OFF";
		case 0xe7: return "PHASE3ON";
		case 0xe8: return "MOTOROFF";
		case 0xe9: return "MOTORON";
		case 0xea: return "DRV0EN";
		case 0xeb: return "DRV1EN";
		case 0xec: return "Q6L";
		case 0xed: return "Q6H";
		case 0xee: return "Q7L";
		case 0xef: return "Q7H";

		case 0x64: return "PADDLE0";
		case 0x65: return "PADDLE1";
		case 0x66: return "PADDLE2";
		case 0x67: return "PADDLE3";

		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			return "GAMEPADDLESTUFF";
		case 0x8b:
			return "??? MAKES ZERO SENSE ???";
		default:
			std::cout << "??? Unknown adrs $C0" << to_hex2(adrs) << std::endl;
			//assert( 0 );
			return "?";
	}
}
